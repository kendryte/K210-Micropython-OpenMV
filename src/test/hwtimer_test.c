#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>

#ifdef RT_USING_HWTIMER

static rt_err_t timer_oneshot_cb(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("enter oneshot@Tick(%d)\n", rt_tick_get());

    return 0;
}

static rt_err_t timer_period_cb(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("enter period@Tick(%d)\n", rt_tick_get());

    return 0;
}

static int period_test(rt_device_t dev)
{
    rt_err_t err;
    rt_hwtimer_mode_t mode;
    rt_hwtimerval_t val;

    /* 设置超时回调函数 */
    rt_device_set_rx_indicate(dev, timer_period_cb);

    /* 单次模式 */
    mode = HWTIMER_MODE_PERIOD;
    err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);

    /* 设置定时器超时值并启动定时器 */
    val.sec = 1;
    val.usec = 0;
    rt_kprintf("Start period callback: Sec %d, Usec %d@Tick(%d)\n", val.sec, val.usec, rt_tick_get());
    if (rt_device_write(dev, 0, &val, sizeof(val)) != sizeof(val))
    {
        rt_kprintf("SetTime Fail\n");
        return -1;
    }

    /* 等待回调函数执行 */
    rt_kprintf("sleep 5s\n");
    rt_thread_mdelay(5100);
    /* 停止定时器 */
    rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");

    return 0;
}

static int oneshot_test(rt_device_t dev)
{
    rt_err_t err;
    rt_hwtimer_mode_t mode;
    rt_hwtimerval_t val;

    /* 设置超时回调函数 */
    rt_device_set_rx_indicate(dev, timer_oneshot_cb);

    /* 单次模式 */
    mode = HWTIMER_MODE_ONESHOT;
    err = rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode);

    /* 设置定时器超时值并启动定时器 */
    val.sec = 1;
    val.usec = 0;
    rt_kprintf("Start oneshot callback: Sec %d, Usec %d@Tick(%d)\n", val.sec, val.usec, rt_tick_get());
    if (rt_device_write(dev, 0, &val, sizeof(val)) != sizeof(val))
    {
        rt_kprintf("SetTime Fail\n");
        return -1;
    }

    /* 等待回调函数执行 */
    rt_thread_mdelay(1100);

    return 0;
}

static int timemeasure_test(rt_device_t dev)
{
    rt_err_t err;
    rt_hwtimerval_t val;
    rt_hwtimer_mode_t mode;

    mode = HWTIMER_MODE_PERIOD;
    if (rt_device_control(dev, HWTIMER_CTRL_MODE_SET, &mode) != 0)
        return -1;

    rt_kprintf("Start time measure@Tick(%d)\n", rt_tick_get());
    /* 设置定时器超时值并启动定时器 */
    val.sec = 1;
    val.usec = 0;

    if (rt_device_write(dev, 0, &val, sizeof(val)) != sizeof(val))
    {
        rt_kprintf("SetTime Fail\n");
        return -1;
    }

    /* 读取计数 */
    rt_kprintf("Sleep %d sec\n", val.sec);
    rt_thread_mdelay(val.sec * 1000);
    rt_device_read(dev, 0, &val, sizeof(val));
    rt_kprintf("Read@Tick(%d): Sec = %d, Usec = %d\n", rt_tick_get(), val.sec, val.usec);

    rt_kprintf("Sleep %d sec with timer running\n", val.sec);
    rt_thread_mdelay(val.sec * 1000);
    rt_device_read(dev, 0, &val, sizeof(val));
    rt_kprintf("Read@Tick(%d): Sec = %d, Usec = %d\n", rt_tick_get(), val.sec, val.usec);

    /* 停止定时器 */
    err = rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_kprintf("Timer Stoped\n");

    return 0;
}

static void show_info(rt_device_t dev)
{
    struct rt_hwtimer_info info;

    rt_memset(&info, 0, sizeof(info));
    rt_device_control(dev, HWTIMER_CTRL_INFO_GET, &info);

    rt_kprintf("TimerInfo: maxfreq %d, minfreq %d\n", info.maxfreq, info.minfreq);
}

int hwtimer(int argc, char **argv)
{
    rt_device_t dev = RT_NULL;
    char *devname = "timer10";
    
    if (argc == 2)
    {
        devname = argv[1];
    }

    if ((dev = rt_device_find(devname)) == RT_NULL)
    {
        rt_kprintf("No Device: %s\n", devname);
        return -1;
    }

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open %s Fail\n", devname);
        return -1;
    }

    show_info(dev);

    /**** 时间测量 */
    if (timemeasure_test(dev) != 0)
        goto EXIT;

    /**** 定时执行回调函数 -- 单次模式 */
    if (oneshot_test(dev) != 0)
        goto EXIT;

    /**** 定时执行回调函数 -- 周期模式 */
    period_test(dev);

EXIT:
    rt_device_close(dev);
    rt_kprintf("Close %s\n", devname);

    return 0;
}
MSH_CMD_EXPORT(hwtimer, "hwtimer NAME");
#endif /* RT_USING_HWTIMER */
