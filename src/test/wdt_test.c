
#include <rtthread.h>
#include <rtdevice.h>

#ifdef RT_USING_WDT

static rt_device_t  wdt_dev = RT_NULL;

static int wdt_sample(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t timeout = 5;

    wdt_dev = rt_device_find("wdt0");
    if(!wdt_dev) {
        rt_kprintf("find wdt0 failed\n");
        return RT_ERROR;
    }

    ret = rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if(ret != RT_EOK) {
        rt_kprintf("set timeout fail\n");
        return RT_ERROR;
    }

    ret = rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
    if(ret != RT_EOK) {
        rt_kprintf("start fail\n");
        return RT_ERROR;
    }    
    return ret;
}
MSH_CMD_EXPORT(wdt_sample, wdt feed);

static int wdt_feed(void)
{
    rt_err_t ret = RT_EOK;    
    if(wdt_dev == RT_NULL) {
        rt_kprintf("wdt0 null\n");
    }
    else {
        ret = rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
        if(ret != RT_EOK) {
            rt_kprintf("wdt keepalive fail\n");
        }
        else {
            rt_kprintf("keepalive\n");
        }    
    }
}
MSH_CMD_EXPORT(wdt_feed, wdt test);

#endif