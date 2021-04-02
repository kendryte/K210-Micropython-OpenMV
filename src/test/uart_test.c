#include <rtthread.h>
#include <rtdevice.h>

int uartrecv(int argc, char *argv[])
{
    rt_device_t dev;
    struct serial_configure cfg = RT_SERIAL_CONFIG_DEFAULT;
    char buf[16];
    int tmout = 30 * 1000;
    rt_tick_t t;

    if (argc != 2)
    {
        rt_kprintf("Usage: uartrecv DEVNAME\n");
        return -1;
    }

    dev = rt_device_find(argv[1]);
    if (!dev)
    {
        rt_kprintf("can not find %s\n", argv[1]);
        return -1;
    }

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != 0)
    {
        rt_kprintf("open fail\n");
        return -1;
    }

    cfg.baud_rate = BAUD_RATE_115200;

    if (rt_device_control(dev, RT_DEVICE_CTRL_CONFIG, &cfg) != 0)
    {
        rt_kprintf("ioctl fail\n");
        goto _out;
    }

    t = rt_tick_get();
    while (1)
    {
        int len;

        len = rt_device_read(dev, 0, buf, sizeof(buf));
        if (len > 0)
        {
            int i;

            for (i = 0; i < len; i++)
            {
                rt_kprintf("%02X ", buf[i]);
            }

            t = rt_tick_get();
            tmout = 2 * 1000;
        }
        else
        {
            if (rt_tick_from_millisecond(rt_tick_get() - t) > tmout)
                break;

            rt_thread_mdelay(50);
        }
    }

_out:
    rt_device_close(dev);

    return 0;
}
MSH_CMD_EXPORT(uartrecv, uartrecv DEVNAME);

int uartsend(int argc, char *argv[])
{
    rt_device_t dev;
    struct serial_configure cfg = RT_SERIAL_CONFIG_DEFAULT;

    if (argc != 3)
    {
        rt_kprintf("Usage: uartsend DEVNAME STRING\n");
        return -1;
    }

    dev = rt_device_find(argv[1]);
    if (!dev)
    {
        rt_kprintf("can not find %s\n", argv[1]);
        return -1;
    }

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != 0)
    {
        rt_kprintf("open fail\n");
        return -1;
    }

    cfg.baud_rate = BAUD_RATE_115200;

    if (rt_device_control(dev, RT_DEVICE_CTRL_CONFIG, &cfg) != 0)
    {
        rt_kprintf("ioctl fail\n");
        goto _out;
    }

    rt_device_write(dev, 0, argv[2], rt_strlen(argv[2]));
    rt_thread_mdelay(2000);

_out:
    rt_device_close(dev);

    return 0;
}
MSH_CMD_EXPORT(uartsend, uartsend DEVNAME STRING);
