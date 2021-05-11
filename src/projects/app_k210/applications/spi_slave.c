#include <rtthread.h>
#include <rtdevice.h>
#ifdef BSP_USING_BRIDGE
#include "spi_slave.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "stdio.h"
#include <drv_io_config.h>
#include "mp.h"
#include "imlib.h"
#include "py/parse.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "usbdbg.h"
#include "dmalock.h"

#define DBG_TAG  "SLAVE"
#define DBG_LVL  DBG_LOG
#include <rtdbg.h>

#define SPI_SLAVE_INT_PIN       BRIDGE_INT_PIN
#define SPI_SLAVE_INT_IO        SPI2_INT_PIN
#define SPI_SLAVE_READY_PIN     BRIDGE_READY_PIN
#define SPI_SLAVE_READY_IO      SPI2_READY_PIN
#define SPI_DATA_READY_PIN      BRIDGE_DATA_READY_PIN
#define SPI_SLAVE_CS_PIN        BRIDGE_CS_PIN
#define SPI_SLAVE_CLK_PIN       BRIDGE_CLK_PIN
#define SPI_SLAVE_MOSI_PIN      BRIDGE_MOSI_PIN
#define SPI_SLAVE_MISO_PIN      BRIDGE_MISO_PIN

#define BUFFER0_LENGTH 1
#define BUFFER1_LENGTH 2
#define BUFFER2_LENGTH 256

#define SPI_SLAVE_BUFFER_WRITE      0
#define SPI_SLAVE_BUFFER_READ       1

#define BUFFER_SIZE (BUFFER2_LENGTH * 4)

extern void serial_tx_buf_flush(void);

static spi_slave_buffer_desc_t slave_buffer_desc[3];

static void spi_slave_set_data_ready(void)
{
    slave_buffer_desc[0].address[0] = 1;
}

static int spi_slave_receive_hook(void *data)
{
    uint8_t request;
    static int xfer_length = 0;
    uint8_t * buf;
    spi_slave_command_t *cmd = (spi_slave_command_t *)data;
    rt_pin_write(SPI_DATA_READY_PIN, 0);
    if(cmd->command.index == 1) // cmd
    {
        
        buf = (uint8_t *)slave_buffer_desc[1].address;
        request = buf[1];
        xfer_length = buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24);
        //ulog_hexdump("cmd", 16, buf, (cmd->command.length + 1) * 4);
        usbdbg_control(buf+6, request, xfer_length);
        if (request & 0x80)
        {
            int bytes = MIN(xfer_length, BUFFER_SIZE);
            buf = (uint8_t *)slave_buffer_desc[2].address;
            xfer_length -= bytes;
            usbdbg_data_in(buf, bytes);
            //ulog_hexdump("in", 16, buf, bytes);
        }
    }
    else if(cmd->command.index == 2)
    {
        if (cmd->command.rw == SPI_SLAVE_BUFFER_WRITE)
        {
            if(xfer_length > 0)
            {
                buf = (uint8_t *)slave_buffer_desc[2].address;
                //ulog_hexdump("out", 16, buf, (cmd->command.length + 1) * 4);
                usbdbg_data_out(buf, (cmd->command.length + 1) * 4);
                xfer_length -= (cmd->command.length + 1) * 4;
            }
        }
        else
        {
            if(xfer_length > 0)
            {
                int bytes = MIN(xfer_length, BUFFER_SIZE);
                buf = (uint8_t *)slave_buffer_desc[2].address;
                //ulog_hexdump("in", 16, buf, (cmd->command.length + 1) * 4);
                xfer_length -= bytes;
                usbdbg_data_in(buf, bytes);
            }
        }
    }
    rt_pin_write(SPI_DATA_READY_PIN, 1);
    return 0;
}

static int spi_slave_init(spi_slave_buffer_desc_t *data, uint8_t len)
{
    fpioa_io_config_t cfg;
    dmac_channel_number_t dma_chn = DMAC_CHANNEL_MAX;

    dmalock_sync_take(&dma_chn, RT_WAITING_FOREVER);

    fpioa_set_function(SPI_SLAVE_CS_PIN, FUNC_SPI_SLAVE_SS);
    fpioa_set_function(SPI_SLAVE_CLK_PIN, FUNC_SPI_SLAVE_SCLK);
    fpioa_set_function_raw(SPI_SLAVE_MOSI_PIN, FUNC_SPI_SLAVE_D0);
    fpioa_get_io(SPI_SLAVE_MOSI_PIN, &cfg);
    cfg.oe_en = 0;
    fpioa_set_io(SPI_SLAVE_MOSI_PIN, &cfg);
    fpioa_set_function_raw(SPI_SLAVE_MISO_PIN, FUNC_SPI_SLAVE_D0);
    fpioa_get_io(SPI_SLAVE_MISO_PIN, &cfg);
    cfg.ie_en = 0;
    fpioa_set_io(SPI_SLAVE_MISO_PIN, &cfg);
    fpioa_set_function(SPI_SLAVE_INT_PIN, FUNC_GPIOHS0 + SPI_SLAVE_INT_IO);
    fpioa_set_function(SPI_SLAVE_READY_PIN, FUNC_GPIOHS0 + SPI_SLAVE_READY_IO);
    rt_pin_mode(SPI_DATA_READY_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SPI_DATA_READY_PIN, 1);
    spi_slave_config(SPI_SLAVE_INT_IO, SPI_SLAVE_READY_IO, dma_chn, data, len, spi_slave_receive_hook);
    rt_kprintf("dual wire spi slave\n");
    return 0;
}

int spi_dbg_init(void)
{
    printf("spi slave init\n");
    slave_buffer_desc[0].address = rt_malloc_align(BUFFER0_LENGTH * 4, 16);
    slave_buffer_desc[0].length = BUFFER0_LENGTH;
    printf("buff0 addr: %p\n", slave_buffer_desc[0].address);
    slave_buffer_desc[1].address = rt_malloc_align(BUFFER1_LENGTH * 4, 16);
    slave_buffer_desc[1].length = BUFFER1_LENGTH;
    printf("buff1 addr: %p\n", slave_buffer_desc[1].address);
    slave_buffer_desc[2].address = rt_malloc_align(BUFFER2_LENGTH * 4, 16);
    slave_buffer_desc[2].length = BUFFER2_LENGTH;
    printf("buff2 addr: %p\n", slave_buffer_desc[2].address);
    serial_tx_buf_flush();
    return spi_slave_init(slave_buffer_desc, 3);
}
INIT_ENV_EXPORT(spi_dbg_init);

#endif /* BSP_USING_BRIDGE */
