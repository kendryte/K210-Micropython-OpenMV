/*
 * Copyright (c) 2020 panrui <https://github.com/Prry/rtt-sd3068>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-01     panrui      the first version
 */
 
#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>

#ifdef  BSP_USING_SD3068

#define 	SD3068_ARRD			0x32	/* slave address */

#define		REG_SEC				0x00
#define		REG_MIN				0x01
#define		REG_HOUR			0x02
#define		REG_DAY				0x03
#define		REG_WEEK			0x04
#define		REG_MON				0x05
#define		REG_YEAR			0x06
#define 	REG_ALM1_SEC  		0x07
#define 	REG_ALM1_MIN 	  	0x08
#define 	REG_ALM1_HOUR     	0x09
#define 	REG_ALM1_EN         0x0E

#define 	REG_CTR1            0x0F
#define 	REG_CTR2            0x10
#define 	REG_CTR3 		    0x11

/* sd3068 device structure */
struct sd3068_device
{
    struct rt_device rtc_parent;
    struct rt_i2c_bus_device *i2c_device;
};

#ifndef SD3068_I2C_BUS
#define	SD3068_I2C_BUS		"i2c0"		/* i2c linked */
#endif
#ifndef SD3068_DEVICE_NAME
#define	SD3068_DEVICE_NAME	"rtc"		/* register device name */
#endif

static struct sd3068_device sd3068_dev;	/* sd3068 device */

static unsigned char bcd_to_hex(unsigned char data)
{
    unsigned char temp;

    temp = ((data>>4)*10 + (data&0x0f));
    return temp;
}

static unsigned char hex_to_bcd(unsigned char data)
{
    unsigned char temp;

    temp = (((data/10)<<4) + (data%10));
    return temp;
}

static rt_err_t  sd3068_read_reg(rt_uint8_t reg, rt_uint8_t *data,rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[2];

    msg[0].addr  = SD3068_ARRD;
    msg[0].flags = RT_I2C_WR;
    msg[0].len   = 1;
    msg[0].buf   = &reg;
    msg[1].addr  = SD3068_ARRD;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = data_size;
    msg[1].buf   = data;

    if(rt_i2c_transfer(sd3068_dev.i2c_device, msg, 2) == 2)
	{
        return RT_EOK;
    }
    else
    {
	  	LOG_E("i2c bus read failed!\r\n");
        return -RT_ERROR;
    }
}

static rt_err_t  sd3068_write_reg(rt_uint8_t reg, rt_uint8_t *data, rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[2];

    msg[0].addr		= SD3068_ARRD;
    msg[0].flags	= RT_I2C_WR;
    msg[0].len   	= 1;
    msg[0].buf   	= &reg;
    msg[1].addr  	= SD3068_ARRD;
    msg[1].flags	= RT_I2C_WR | RT_I2C_NO_START;
    msg[1].len   	= data_size;
    msg[1].buf   	= data;

    if(rt_i2c_transfer(sd3068_dev.i2c_device, msg, 2) == 2)
	{
        return RT_EOK;
    }
    else
    {
	  	LOG_E("i2c bus write failed!\r\n");
        return -RT_ERROR;
    }
}

static int sd3068_write_enable(void)
{
    int ret = 0;
    uint8_t data[2];

    data[0] = 0xFF;
    data[1] = 0x80;
    ret |= sd3068_write_reg(REG_CTR2, &data[1], 1);
    ret |= sd3068_write_reg(REG_CTR1, &data[0], 1);

    return ret;
}

static rt_err_t rt_sd3068_open(rt_device_t dev, rt_uint16_t flag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* open interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_sd3068_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return RT_EOK;
}

static rt_err_t rt_sd3068_control(rt_device_t dev, int cmd, void *args)
{
	rt_err_t	ret = RT_EOK;
    time_t 		*time;
    struct tm 	time_temp;	
    rt_uint8_t 	buff[7];
	
    RT_ASSERT(dev != RT_NULL);
    rt_memset(&time_temp, 0, sizeof(struct tm));

    switch (cmd)
    {
    	/* read time */
        case RT_DEVICE_CTRL_RTC_GET_TIME:
	        time = (time_t *)args;
	        ret = sd3068_read_reg(REG_SEC,buff,7);
			if(ret == RT_EOK)
			{
				time_temp.tm_year  = bcd_to_hex(buff[6]) + 2000 - 1900;
				time_temp.tm_mon   = bcd_to_hex(buff[5]&0x7f) - 1;
				time_temp.tm_mday  = bcd_to_hex(buff[4]);
				time_temp.tm_hour  = bcd_to_hex(buff[2]&0x7f);
				time_temp.tm_min   = bcd_to_hex(buff[1]);
				time_temp.tm_sec   = bcd_to_hex(buff[0]);
	        	*time = mktime(&time_temp);
			}
        break;

		/* set time */
        case RT_DEVICE_CTRL_RTC_SET_TIME:
        {
        	struct tm *time_new;

            time = (time_t *)args;
            time_new = gmtime(time);
            buff[6] = hex_to_bcd(time_new->tm_year + 1900 - 2000);
            buff[5] = hex_to_bcd(time_new->tm_mon + 1);
            buff[4] = hex_to_bcd(time_new->tm_mday);
            buff[3] = hex_to_bcd(time_new->tm_wday);
            buff[2] = hex_to_bcd(time_new->tm_hour)|0x80;
            buff[1] = hex_to_bcd(time_new->tm_min);
            buff[0] = hex_to_bcd(time_new->tm_sec);
            ret = sd3068_write_reg(REG_SEC, buff, 7);
        }
        break;
        default:
        break;
	}

    return ret;
}

int rt_hw_sd3068_init(void)
{		
    struct rt_i2c_bus_device *i2c_device;
    uint8_t data;

    i2c_device = rt_i2c_bus_device_find(SD3068_I2C_BUS);
    if (i2c_device == RT_NULL)
    {
        LOG_E("i2c bus device %s not found!\r\n", SD3068_I2C_BUS);
        return -RT_ERROR;
    }				 	
    sd3068_dev.i2c_device = i2c_device;
	
    /* register rtc device */
    sd3068_dev.rtc_parent.type   		= RT_Device_Class_RTC;
    sd3068_dev.rtc_parent.init    		= RT_NULL;
    sd3068_dev.rtc_parent.open    		= rt_sd3068_open;
    sd3068_dev.rtc_parent.close   		= RT_NULL;
    sd3068_dev.rtc_parent.read   		= rt_sd3068_read;
    sd3068_dev.rtc_parent.write  	 	= RT_NULL;
    sd3068_dev.rtc_parent.control 		= rt_sd3068_control;
    sd3068_dev.rtc_parent.user_data 	= RT_NULL;			/* no private */
    rt_device_register(&sd3068_dev.rtc_parent, SD3068_DEVICE_NAME, RT_DEVICE_FLAG_RDWR);

    /* init sd3068 */
    sd3068_write_enable();

	LOG_D("the rtc of sd3068 init succeed!\r\n");

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_sd3068_init);
#endif
