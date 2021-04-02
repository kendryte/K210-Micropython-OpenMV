/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-08-21     heyuanjie87    first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"
#include "i2c.h"

#ifdef RT_USING_I2C

typedef struct
{
    struct rt_i2c_bus_device busdev;
    volatile i2c_t *i2c;
} ki2c_t;

static int ki2c_send(volatile i2c_t *i2c_adapter, uint8_t *send_buf, uint32_t send_buf_len)
{
    uint32_t fifo_len, index;

    while (send_buf_len)
    {
        fifo_len = 8 - i2c_adapter->txflr;
        fifo_len = send_buf_len < fifo_len ? send_buf_len : fifo_len;
        for (index = 0; index < fifo_len; index++)
            i2c_adapter->data_cmd = I2C_DATA_CMD_DATA(*send_buf++);
        if (i2c_adapter->tx_abrt_source != 0)
        {
            while (i2c_adapter->status & I2C_STATUS_ACTIVITY);	//
            i2c_adapter->clr_intr = i2c_adapter->clr_intr;		//
            return 1;
        }

        send_buf_len -= fifo_len;
    }

    return 0;
}

static int ki2c_recv(volatile i2c_t *i2c_adapter, uint8_t *receive_buf, uint32_t receive_buf_len)
{
    uint32_t fifo_len, index;
    uint32_t rx_len = receive_buf_len;

    while (receive_buf_len || rx_len)
    {
        fifo_len = i2c_adapter->rxflr;
        fifo_len = rx_len < fifo_len ? rx_len : fifo_len;
        for (index = 0; index < fifo_len; index++)
            *receive_buf++ = (uint8_t)i2c_adapter->data_cmd;
        rx_len -= fifo_len;
        fifo_len = 8 - i2c_adapter->txflr;
        fifo_len = receive_buf_len < fifo_len ? receive_buf_len : fifo_len;
        for (index = 0; index < fifo_len; index++)
            i2c_adapter->data_cmd = I2C_DATA_CMD_CMD;
        if (i2c_adapter->tx_abrt_source != 0)
            return 1;
        receive_buf_len -= fifo_len;
    }

    return 0;
}

static void ki2c_setaddr(volatile i2c_t *i2c_adapter, uint16_t addr, int width)
{
    i2c_adapter->tar = I2C_TAR_ADDRESS(addr) & I2C_TAR_ADDRESS_MASK;

    if(width == 10)
    {
        i2c_adapter->tar |= I2C_TAR_10BITADDR_MASTER;
    }
    else
    {
        i2c_adapter->tar &= ~I2C_TAR_10BITADDR_MASTER;
    }
    
}

static int ki2c_waittx(volatile i2c_t *i2c_adapter, int timeout_ms)
{
    rt_tick_t start;

    start = rt_tick_get();
    while ((i2c_adapter->status & I2C_STATUS_ACTIVITY) || !(i2c_adapter->status & I2C_STATUS_TFE))
    {
        if (rt_tick_from_millisecond(rt_tick_get() - start) > timeout_ms)
            break;
    }

    if (i2c_adapter->tx_abrt_source != 0)
        return 1;

    return 0;
}

static void ki2c_clearerr(volatile i2c_t *i2c_adapter)
{
    i2c_adapter->clr_tx_abrt = i2c_adapter->clr_tx_abrt;
}

static rt_size_t _i2c_mst_xfer(struct rt_i2c_bus_device *bus,
                               struct rt_i2c_msg msgs[],
                               rt_uint32_t num)
{
    rt_size_t i;
    ki2c_t *kbus = (ki2c_t *)bus;
    int status;
    int waittx = 0;

    RT_ASSERT(bus != RT_NULL);
    if(msgs[0].flags & RT_I2C_ADDR_10BIT)
    {
        ki2c_setaddr(kbus->i2c, msgs[0].addr, 10);
    }
    else
    {
        ki2c_setaddr(kbus->i2c, msgs[0].addr, 7);
    }
    
    
    ki2c_clearerr(kbus->i2c);

    for (i = 0; i < num; i++)
    {
        waittx = 0;

        if (msgs[i].flags & RT_I2C_RD)
        {
            status = ki2c_recv(kbus->i2c, msgs[i].buf, msgs[i].len);
        }
        else
        {
            status = ki2c_send(kbus->i2c, msgs[i].buf, msgs[i].len);
            waittx = 1;
        }

        if (status != 0)
            goto _out;
    }

    if (waittx)
    {
        if (ki2c_waittx(kbus->i2c, 2000) != 0)
            i = 0;
    }

_out:
    return i;
}

static rt_err_t _i2c_bus_control(struct rt_i2c_bus_device *bus,
                                 rt_uint32_t cmd,
                                 rt_uint32_t arg)
{
    return 0;
}

static const struct rt_i2c_bus_device_ops _i2c_ops =
{
    .master_xfer = _i2c_mst_xfer,
    .slave_xfer = RT_NULL,
    .i2c_bus_control = _i2c_bus_control,
};

#ifdef BSP_USING_I2C0
ki2c_t _i2c0 =
{
    .i2c = (volatile i2c_t *)I2C0_BASE_ADDR,
};
#endif

#ifdef BSP_USING_I2C1
ki2c_t _i2c1 =
{
    .i2c = (volatile i2c_t *)I2C1_BASE_ADDR,
};
#endif

#ifdef BSP_USING_I2C2
ki2c_t _i2c2 =
{
    .i2c = (volatile i2c_t *)I2C2_BASE_ADDR,
};
#endif

int rt_hw_i2c_init(void)
{
#ifdef BSP_USING_I2C0
    _i2c0.busdev.ops = &_i2c_ops;

    i2c_init(I2C_DEVICE_0, 0, 7, 100000);
    rt_i2c_bus_device_register(&_i2c0.busdev, "i2c0");
#endif

#ifdef BSP_USING_I2C1
    _i2c1.busdev.ops = &_i2c_ops;

    i2c_init(I2C_DEVICE_1, 0, 7, 100000);
    rt_i2c_bus_device_register(&_i2c1.busdev, "i2c1");
#endif

#ifdef BSP_USING_I2C2
    _i2c2.busdev.ops = &_i2c_ops;

    i2c_init(I2C_DEVICE_2, 0, 7, 100000);
    rt_i2c_bus_device_register(&_i2c2.busdev, "i2c2");
#endif
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_i2c_init);
#endif
