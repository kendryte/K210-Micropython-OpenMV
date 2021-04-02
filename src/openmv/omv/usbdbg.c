/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * USB debugger.
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "mp.h"
#include "imlib.h"
#include "sensor.h"
#include "framebuffer.h"
//#include "ff.h"
#include "usbdbg.h"
#include "py/nlr.h"
#include "py/lexer.h"
#include "py/parse.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/objstr.h"
#include "omv_boardconfig.h"
#include "rt_thread_dbg.h"



static int xfer_bytes;
static int xfer_length;
static enum usbdbg_cmd cmd;

static volatile bool script_ready;
static volatile bool script_running;
static vstr_t script_buf;

static mp_obj_exception_t ide_exception; //IDE interrupt
static mp_obj_str_t   ide_exception_str;
static mp_obj_tuple_t* ide_exception_str_tuple = NULL;
static mp_obj_t mp_const_ide_interrupt = (mp_obj_t)(&ide_exception);

extern const char *ffs_strerror(FRESULT res);
void usbdbg_init0()
{
    ide_exception_str.data = (const byte*)"IDE interrupt";
    ide_exception_str.len  = 13;
    ide_exception_str.base.type = &mp_type_str;
    ide_exception_str.hash = qstr_compute_hash(ide_exception_str.data, ide_exception_str.len);
    ide_exception_str_tuple = (mp_obj_tuple_t*)malloc(sizeof(mp_obj_tuple_t)+sizeof(mp_obj_t)*1);
    if(ide_exception_str_tuple==NULL)
        return;
    ide_exception_str_tuple->base.type = &mp_type_tuple;
    ide_exception_str_tuple->len = 1;
    ide_exception_str_tuple->items[0] = MP_OBJ_FROM_PTR(&ide_exception_str);
    ide_exception.base.type = &mp_type_Exception;
    ide_exception.traceback_alloc = 0;
    ide_exception.traceback_len = 0;
    ide_exception.traceback_data = NULL;
    ide_exception.args = ide_exception_str_tuple;
}
void usbdbg_init()
{
    script_ready=false;
    script_running=false;
    xfer_bytes = 0;
    xfer_length = 0;
    vstr_init(&script_buf, 5*1024);
}

bool usbdbg_script_ready()
{
    return script_ready;
}

void usbdbg_set_script_ready(bool ready)
{
	script_ready = ready;
}

vstr_t *usbdbg_get_script()
{
    return &script_buf;
}

void usbdbg_set_script_running(bool running)
{
    script_running = running;
}

bool usbdbg_script_running()
{
	return script_running;
}

inline void usbdbg_set_irq_enabled(bool enabled)
{
    serial_set_irq_enabled(enabled);
}
#if 1
#define logout(...) 
#else
#define logout(...) mp_printf(MP_PYTHON_PRINTER, __VA_ARGS__)
#endif
// #define DUMP_RAW  // dump uncompressed framebuffer, slow but clear
#ifdef DUMP_RAW
#define DUMP_FB	MAIN_FB
#else
#define DUMP_FB JPEG_FB
#endif
volatile uint8_t g_omvIdeConnecting;
RT_WEAK int sensor_get_id(void) {return 1;}
RT_WEAK void Hook_OnUsbDbgScriptExec(void) {}
volatile uint8_t s_UsbDbgIsToRunScript;
void usbdbg_data_in(void *buffer, int length)
{
	//logout("usbdbg_data_in: cmd=%x, buffer=0x%08x, bytes=%d\r\n", cmd, buffer, length);
    switch (cmd) {
        case USBDBG_FW_VERSION: {
			g_omvIdeConnecting = 1;
			serial_tx_buf_flush();
            uint32_t *ver_buf = buffer;
            ver_buf[0] = FIRMWARE_VERSION_MAJOR;
            ver_buf[1] = FIRMWARE_VERSION_MINOR;
            ver_buf[2] = 0; //FIRMWARE_VERSION_PATCH;
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_TX_BUF_LEN: {
			uint32_t *p = (uint32_t*)buffer;
            p[0] = serial_tx_buf_len();
            cmd = USBDBG_NONE;
            break;
        }
        case USBDBG_SENSOR_ID: {
            int sensor_id = 0xFF;
            if (sensor_is_detected() == true) {
                sensor_id = sensor_get_id();
            }
            memcpy(buffer, &sensor_id, 4);
            cmd = USBDBG_NONE;
        }

        case USBDBG_TX_BUF: {
			int n = serial_tx_buf_read(buffer, length);
			if (n < 0) {
				cmd = USBDBG_NONE;
			} else if (n == 0) {
				cmd = USBDBG_NONE;
			} else {
				xfer_bytes += n;
				if (xfer_bytes == xfer_length) {
					cmd = USBDBG_NONE;
				}
			}
            break;
        }

        case USBDBG_FRAME_SIZE:
			g_omvIdeConnecting = 0;
		#ifdef OMV_MPY_ONLY
			((uint32_t*)buffer)[0] = 0;
			((uint32_t*)buffer)[1] = 0;
			((uint32_t*)buffer)[2] = 0; // MAIN_FB()->w * MAIN_FB()->h * MAIN_FB()->bpp;			
		#else
			#ifdef DUMP_RAW
				((uint32_t*)buffer)[0] = MAIN_FB()->w;
				((uint32_t*)buffer)[1] = MAIN_FB()->h;
				((uint32_t*)buffer)[2] = MAIN_FB()->bpp; // MAIN_FB()->w * MAIN_FB()->h * MAIN_FB()->bpp;		
			#else
				// Return 0 if FB is locked or not ready.
				((uint32_t*)buffer)[0] = 0;
				// Try to lock FB. If header size == 0 frame is not ready
				if (mutex_try_lock(&JPEG_FB()->lock, MUTEX_TID_IDE)) {
					// If header size == 0 frame is not ready
					if (JPEG_FB()->size == 0) {
						// unlock FB
						mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
					} else {
						// Return header w, h and size/bpp
						((uint32_t*)buffer)[0] = JPEG_FB()->w;
						((uint32_t*)buffer)[1] = JPEG_FB()->h;
						((uint32_t*)buffer)[2] = JPEG_FB()->size;
					}
				}
			#endif
		#endif
            cmd = USBDBG_NONE;
            break;

        case USBDBG_FRAME_DUMP:
			#ifdef OMV_MPY_ONLY
				cmd = USBDBG_NONE;
			#else
				if (xfer_bytes < xfer_length) {
				#ifdef DUMP_RAW
					memcpy(buffer, MAIN_FB()->pixels+xfer_bytes, length);
					xfer_bytes += length;
					if (xfer_bytes == xfer_length) {
						cmd = USBDBG_NONE;
					}			
				#else
					memcpy(buffer, JPEG_FB()->pixels+xfer_bytes, length);
					xfer_bytes += length;
					if (xfer_bytes == xfer_length) {
						cmd = USBDBG_NONE;
						JPEG_FB()->w = 0; JPEG_FB()->h = 0; JPEG_FB()->size = 0;
						mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
					}
				#endif
            }
			#endif
            break;

        case USBDBG_ARCH_STR: {
			// Note: this is not official OpenMV Cam board, thank 
			// openmv llc who supported openMV i.MX RT porting!
			// This board and code is not for commercial purpose!
			// please support openMV by buying genuine OpenMV Cam!
			#if 0
            snprintf((char *) buffer, 64, "%s [%s:%08X%08X%08X]",
                    OMV_ARCH_STR/*OpenMV i.MX RT1050/60 port*/, OMV_BOARD_TYPE /* M7 */,
                    0x4B4E4854,
                    0x564D4F20,
                    0x434C4C20);	
			#else
            snprintf((char *) buffer, 64, "%s [%s:%08X%08X%08X]",
                    OMV_ARCH_STR/*OpenMV i.MX RT1050/60 port*/, OMV_BOARD_TYPE /* M7 */,
                    0x35383236 /*0x4B4E4854*/,
                    0x3436510f /*0x564D4F20*/,
                    0x0041001E /*0x434C4C20*/);	
			#endif
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SCRIPT_RUNNING: {
            uint32_t *buf = buffer;
             // rocky: though may not run yet, set running flag in case openmv ide get not running
             // flag before script get run.
            buf[0] = (uint32_t)script_running; // (s_UsbDbgIsToRunScript || script_running);
            cmd = USBDBG_NONE;
            break;
        }
        default: /* error */
            break;
    }
}

extern int py_image_descriptor_from_roi(image_t *image, const char *path, rectangle_t *roi);

void usbdbg_try_run_script(void)
{
    if (!s_UsbDbgIsToRunScript)
        return;
    // Disable IDE IRQ (re-enabled by pyexec or main).
    //usbdbg_set_irq_enabled(false);
    s_UsbDbgIsToRunScript = 0;
    // Clear interrupt traceback
    mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
    // Interrupt running REPL
    // Note: setting pendsv explicitly here because the VM is probably
    // waiting in REPL and the soft interrupt flag will not be checked.
    // PRINTF("nlr jumping to execute script\r\n");
    
    MP_STATE_VM(mp_pending_exception) = mp_const_ide_interrupt;
    #if MICROPY_ENABLE_SCHEDULER
    if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE) {
        MP_STATE_VM(sched_state) = MP_SCHED_PENDING;
    }
    #endif
}
extern uint8_t g_isMainDotPyRunning;
void usbdbg_data_out(void *buffer, int length)
{
    length = (xfer_length - xfer_bytes) < length ? (xfer_length - xfer_bytes) : length;
    switch (cmd) {
        case USBDBG_FB_ENABLE: {
            uint32_t enable = *((int32_t*)buffer);
            #ifdef DUMP_RAW
            enable = 0;
            #endif
            JPEG_FB()->enabled = enable;
            if (enable == 0) {
                // When disabling framebuffer, the IDE might still be holding FB lock.
                // If the IDE is not the current lock owner, this operation is ignored.
                mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
            }
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SCRIPT_EXEC:
            // check if GC is locked before allocating memory for vstr. If GC was locked
            // at least once before the script is fully uploaded xfer_bytes will be less
            // than the total length (xfer_length) and the script will Not be executed.
			//logout("Execute Script script_running=%d,gc lock=%d\r\n",script_running,gc_is_locked());
            if (!script_running && !gc_is_locked()) {
                vstr_add_strn(&script_buf, buffer, length);
                Hook_OnUsbDbgScriptExec();
                xfer_bytes += length;
                if (xfer_bytes == xfer_length) {
                    s_UsbDbgIsToRunScript = 1;
                    script_ready = true;
                    script_running = true;            
                    // in case main.py is running just after system reset, notify the VM to stop it.
					logout("g_isMainDotPyRunning=%d\r\n",g_isMainDotPyRunning);
                    if (g_isMainDotPyRunning)
                    {
                        mp_obj_exception_clear_traceback(mp_const_ide_interrupt);

                        MP_STATE_VM(mp_pending_exception) = mp_const_ide_interrupt;
                        #if MICROPY_ENABLE_SCHEDULER
                        if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE) {
                            MP_STATE_VM(sched_state) = MP_SCHED_PENDING;
                        }
                        #endif
                    }
                }
            }
			else {
				if (gc_is_locked()) {
					logout("GC locked!\r\n");
				}
			}
            break;

        case USBDBG_TEMPLATE_SAVE: {
			#ifndef OMV_MPY_ONLY
            image_t image ={
                .w = MAIN_FB()->w,
                .h = MAIN_FB()->h,
                .bpp = MAIN_FB()->bpp,
                .pixels = MAIN_FB()->pixels
            };

            // null terminate the path
            length = (length == 64) ? 63:length;
            ((char*)buffer)[length] = 0;

            rectangle_t *roi = (rectangle_t*)buffer;
            char *path = (char*)buffer+sizeof(rectangle_t);

            imlib_save_image(&image, path, roi, 50);
            // raise a flash IRQ to flush image
            //NVIC->STIR = FLASH_IRQn;
			#endif
            break;
        }

        case USBDBG_DESCRIPTOR_SAVE: {
			#ifndef OMV_MPY_ONLY
            image_t image ={
                .w = MAIN_FB()->w,
                .h = MAIN_FB()->h,
                .bpp = MAIN_FB()->bpp,
                .pixels = MAIN_FB()->pixels
            };

            // null terminate the path
            length = (length == 64) ? 63:length;
            ((char*)buffer)[length] = 0;

            rectangle_t *roi = (rectangle_t*)buffer;
            char *path = (char*)buffer+sizeof(rectangle_t);

            py_image_descriptor_from_roi(&image, path, roi);
			#endif
            break;
        }
        case USBDBG_ATTR_WRITE: {
            /* write sensor attribute */
            int32_t attr= *((int32_t*)buffer);
            int32_t val = *((int32_t*)buffer+1);
            switch (attr) {
                case ATTR_CONTRAST:
                    sensor_set_contrast(val);
                    break;
                case ATTR_BRIGHTNESS:
                    sensor_set_brightness(val);
                    break;
                case ATTR_SATURATION:
                    sensor_set_saturation(val);
                    break;
                case ATTR_GAINCEILING:
                    sensor_set_gainceiling(val);
                    break;
                default:
                    break;
            }
            cmd = USBDBG_NONE;
            break;
        }
        default: /* error */
            break;
    }
}

void usbdbg_stop_script(void) {
    // Set script running flag
    mp_printf(&mp_plat_print, "usbdbg_stop_script\r\n");
    logout("stop running script\r\n");
    s_UsbDbgIsToRunScript = 0;
    script_running = false;
    script_ready = false;

    // interrupt running code by raising an exception
    mp_obj_exception_clear_traceback(mp_const_ide_interrupt);
    // pendsv_nlr_jump_hard(mp_const_ide_interrupt);
    MP_STATE_VM(mp_pending_exception) = mp_const_ide_interrupt; //MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_kbd_exception));
    #if MICROPY_ENABLE_SCHEDULER
    if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE) {
        MP_STATE_VM(sched_state) = MP_SCHED_PENDING;
    }
    #endif
     
}


void usbdbg_control(void *buffer, uint8_t request, uint32_t length)
{
    cmd = (enum usbdbg_cmd) request;
    switch (cmd) {
        case USBDBG_FW_VERSION:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_FRAME_SIZE:
			logout("control: USBDBG_FRAME_SIZE, len=%d\r\n", length);
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_FRAME_DUMP:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_ARCH_STR:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_SCRIPT_EXEC:
            xfer_bytes = 0;
            xfer_length = length;
            vstr_reset(&script_buf);
            break;

        case USBDBG_SCRIPT_STOP:
            if (script_running) {
                // Disable IDE IRQ (re-enabled by pyexec or main).
                //usbdbg_set_irq_enabled(false);
                usbdbg_stop_script();
            } else {
                s_UsbDbgIsToRunScript = 0;
                script_running = false;
                script_ready = false;
				logout("no script running!\r\n");
			}
            cmd = USBDBG_NONE;
            break;

        case USBDBG_SCRIPT_SAVE:
            /* save running script */
            // TODO
            break;

        case USBDBG_SCRIPT_RUNNING:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_TEMPLATE_SAVE:
        case USBDBG_DESCRIPTOR_SAVE:
            /* save template */
            xfer_bytes = 0;
            xfer_length =length;
            break;

        case USBDBG_ATTR_WRITE: {
            /* write sensor attribute */
            int16_t attr= *((int16_t*)buffer);
            int16_t val = *((int16_t*)buffer+1);
			#ifndef OMV_MPY_ONLY
            switch (attr) {
                case ATTR_CONTRAST:
                    sensor_set_contrast(val);
                    break;
                case ATTR_BRIGHTNESS:
                    sensor_set_brightness(val);
                    break;
                case ATTR_SATURATION:
                    sensor_set_saturation(val);
                    break;
                case ATTR_GAINCEILING:
                    sensor_set_gainceiling(val);
                    break;
                default:
                    break;
            }
			#endif
            cmd = USBDBG_NONE;
            break;
        }

        case USBDBG_SYS_RESET:
            rt_hw_cpu_reset();
            break;

        case USBDBG_FB_ENABLE: {
            xfer_bytes = 0;
            xfer_length = length;
            break;
        }

        case USBDBG_TX_BUF:
        case USBDBG_TX_BUF_LEN:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        case USBDBG_SENSOR_ID:
            xfer_bytes = 0;
            xfer_length = length;
            break;

        default: /* error */
            cmd = USBDBG_NONE;
            break;
    }
}

void usbdbg_connect(void)
{
	// slow down the sensor to avoid tearing effect, when executing from FlexSPI
	// sensor_set_framerate(2<<9 | 2<<11);
}
void usbdbg_disconnect(void) {
	#ifndef OMV_MPY_ONLY
	JPEG_FB()->enabled = 0;
	mutex_unlock(&JPEG_FB()->lock, MUTEX_TID_IDE);
	#endif
	// sensor_set_framerate(2<<9 | 1<<11);
}

