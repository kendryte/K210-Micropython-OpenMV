#include <rtthread.h>
#include <rthw.h>
#include <fal.h>
#include <rt_ota.h>
#include <string.h>
#include <rtdevice.h>
#include <stdlib.h>
#include "plic.h"
#define CODE_LOAD_DEFAULT   (0x80000000)

#define UART_RX_EVENT       (0x01 << 1)

#include "sysctl.h"
static void reset_peripheral(void)
{
    sysctl_clock_enable(SYSCTL_CLOCK_UART1);
    sysctl_clock_enable(SYSCTL_CLOCK_UART2);
    sysctl_clock_enable(SYSCTL_CLOCK_UART3);

    sysctl_reset(SYSCTL_RESET_UART1);
    sysctl_reset(SYSCTL_RESET_UART2);
    sysctl_reset(SYSCTL_RESET_UART3);
    

    sysctl_clock_disable(SYSCTL_CLOCK_UART1);
    sysctl_clock_disable(SYSCTL_CLOCK_UART2);
    sysctl_clock_disable(SYSCTL_CLOCK_UART3);

    rt_hw_interrupt_mask(IRQN_UARTHS_INTERRUPT);
    rt_hw_interrupt_mask(IRQN_UART1_INTERRUPT);
    rt_hw_interrupt_mask(IRQN_UART2_INTERRUPT);
    rt_hw_interrupt_mask(IRQN_UART3_INTERRUPT);
}

static void run_application()
{
    rt_size_t fw_size;
    rt_size_t org_fw_size = 0x1000000;

    fw_size = rt_ota_get_raw_fw_size(fal_partition_find("app"));
    if(fw_size == 0)
    {
        rt_kprintf("fw error...\n");
        return;
    }
    rt_kprintf("fw_size:%d\n", fw_size);

    fal_partition_read(fal_partition_find("app"), 0, (void *)CODE_LOAD_DEFAULT,fw_size);

    //runing...
    rt_hw_interrupt_disable();
    rt_kprintf("run addr:0x%08x\n", CODE_LOAD_DEFAULT);
    rt_kprintf("=============================================");
    rt_hw_interrupt_disable();
    reset_peripheral();
    ((void (*)(void))CODE_LOAD_DEFAULT)();
}

static struct rt_event event;
static int bl_delay = 0;
static rt_err_t uart_intput(rt_device_t dev, rt_size_t size)
{
    rt_event_send(&event, UART_RX_EVENT);   
    return RT_EOK;
}

static rt_err_t run_bootloader_delay(void)
{
    rt_err_t res;
    void *rx_indicate_save;
    rt_device_t uart_device;
    rt_uint32_t recved;

    if (bl_delay <= 0)
    {
        return -RT_ERROR;
    }
    uart_device = rt_console_get_device();
    if (uart_device == RT_NULL)
    {
        rt_kprintf("can't find uart device.\n");
        return -RT_ERROR;
    }
    rt_event_init(&event, "event", RT_IPC_FLAG_FIFO);
    //save the old rx_indicate
    rx_indicate_save = uart_device->rx_indicate;
    res = rt_device_set_rx_indicate(uart_device, uart_intput);
    if (res != RT_EOK)
    {
        rt_kprintf("set rx indicate error.%d\n", res);
        return -RT_ERROR;
    }
    res = rt_device_open(uart_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    if (res != RT_EOK)
    {
        rt_kprintf("open device error.%d\n", res);
        return -RT_ERROR;
    }
    while(bl_delay > 0)
    {
        rt_kprintf("Hit any key to stop autoboot :%2d\r", bl_delay);
        res = rt_event_recv(&event, UART_RX_EVENT, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_TICK_PER_SECOND, &recved);
        if (res == RT_EOK)
        {
            break;
        }
        bl_delay--;
    }
    rt_kprintf("Hit any key to stop autoboot :%2d\r\n", bl_delay);
    //restore the old rx_indicate
    rt_device_set_rx_indicate(uart_device, rx_indicate_save);
    rt_device_close(uart_device);

    return res;
}

int stay_in_bootloader = 3;

extern int ota_main(void);


int load_application(void)
{
    
    if(ota_main() == 0)
    {
        // bl_delay = 3;
        bl_delay = 0;
        
        run_bootloader_delay();
        
        if (bl_delay > 0)
        {
            rt_kprintf("run bootloader\r\n");
            return 0;
        }
        rt_kprintf("run application\r\n");
        run_application();
    }

    return 0;
}
INIT_APP_EXPORT(load_application);

int msh_run_application(int argc, char** argv)
{
    rt_kprintf("run application\r\n");
    run_application();
    return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(msh_run_application, __cmd_run_app, run application.);
