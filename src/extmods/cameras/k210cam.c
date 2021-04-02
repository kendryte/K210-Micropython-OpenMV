#include "k210cam.h"
#include "plic.h"
#include "dvp.h"
#include "fpioa.h"

#include <rtconfig.h>

extern void cam_ov5640_init(k210sensor_t *s);

static int g_dvp_finish_flag = 0;
static int cambus_irq(void *ctx)
{
    if (g_dvp_finish_flag == 1)
    {
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    }

    // sensor_t *sensor = ctx;
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    { //frame end
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else
    {                               //frame start
        if (g_dvp_finish_flag == 0) //only we finish the convert, do transmit again
            dvp_start_convert();    //so we need deal img ontime, or skip one framebefore next
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    //rt_kprintf("D\n");
    return 0;
}

static void _ndelay(uint32_t ns)
{
    uint32_t i;

    while (ns && ns--)
    {
        for (i = 0; i < 25; i++)
            __asm__ __volatile__("nop");
    }
}

static int cam_run(int enable)
{
    if (enable)
    {
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        plic_irq_enable(IRQN_DVP_INTERRUPT);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
    }
    else
    {
        plic_irq_disable(IRQN_DVP_INTERRUPT);
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
    }

    return 0;
}

static void dvp_sccb_start_transfer(void)
{
    while (dvp->sts & DVP_STS_SCCB_EN)
        ;
    dvp->sts = DVP_STS_SCCB_EN | DVP_STS_SCCB_EN_WE;
    while (dvp->sts & DVP_STS_SCCB_EN)
        ;
}

static int _dvp_sccb_send_data(uint8_t dev_addr, uint16_t reg_addr, uint8_t reg_data, int g_sccb_reg_len)
{
    uint32_t tmp;

    tmp = dvp->sccb_cfg & (~DVP_SCCB_BYTE_NUM_MASK);

    (g_sccb_reg_len == 8) ? (tmp |= DVP_SCCB_BYTE_NUM_3) : (tmp |= DVP_SCCB_BYTE_NUM_4);

    dvp->sccb_cfg = tmp;

    if (g_sccb_reg_len == 8)
    {
        dvp->sccb_ctl = DVP_SCCB_WRITE_DATA_ENABLE | DVP_SCCB_DEVICE_ADDRESS(dev_addr) | DVP_SCCB_REG_ADDRESS(reg_addr) | DVP_SCCB_WDATA_BYTE0(reg_data);
    }
    else
    {
        dvp->sccb_ctl = DVP_SCCB_WRITE_DATA_ENABLE | DVP_SCCB_DEVICE_ADDRESS(dev_addr) | DVP_SCCB_REG_ADDRESS(reg_addr >> 8) | DVP_SCCB_WDATA_BYTE0(reg_addr & 0xff) | DVP_SCCB_WDATA_BYTE1(reg_data);
    }
    dvp_sccb_start_transfer();

    return 0;
}

static uint8_t _dvp_sccb_receive_data(uint8_t dev_addr, uint16_t reg_addr, int g_sccb_reg_len)
{
    uint32_t tmp;

    tmp = dvp->sccb_cfg & (~DVP_SCCB_BYTE_NUM_MASK);

    if (g_sccb_reg_len == 8)
        tmp |= DVP_SCCB_BYTE_NUM_2;
    else
        tmp |= DVP_SCCB_BYTE_NUM_3;

    dvp->sccb_cfg = tmp;

    if (g_sccb_reg_len == 8)
    {
        dvp->sccb_ctl = DVP_SCCB_WRITE_DATA_ENABLE | DVP_SCCB_DEVICE_ADDRESS(dev_addr) | DVP_SCCB_REG_ADDRESS(reg_addr);
    }
    else
    {
        dvp->sccb_ctl = DVP_SCCB_WRITE_DATA_ENABLE | DVP_SCCB_DEVICE_ADDRESS(dev_addr) | DVP_SCCB_REG_ADDRESS(reg_addr >> 8) | DVP_SCCB_WDATA_BYTE0(reg_addr & 0xff);
    }
    dvp_sccb_start_transfer();

    dvp->sccb_ctl = DVP_SCCB_DEVICE_ADDRESS(dev_addr);

    dvp_sccb_start_transfer();

    return (uint8_t)DVP_SCCB_RDATA_BYTE(dvp->sccb_cfg);
}

static int sccb_i2c_read_byte(int8_t i2c, uint8_t addr, uint16_t reg, uint8_t reg_len, uint8_t *data, uint16_t timeout_ms)
{
    *data = 0;

    *data = _dvp_sccb_receive_data(addr, reg, reg_len);

    return 0;
}

static int _getid_type1(k210cambus_t *bus, uint8_t addr, int reglen)
{
    uint16_t device_id = 0;
    uint8_t tmp = 0;
    int ret = 0;
    int i2c_device = 0;
    int sccb_reg_width = 0;

    //TODO: 0x300A 0x300B maybe just for OV3660

    ret |= sccb_i2c_read_byte(i2c_device, addr, 0x300A, reglen, &tmp, 100);
    if (ret != 0)
        return ret;
    device_id = tmp << 8;
    ret |= sccb_i2c_read_byte(i2c_device, addr, 0x300B, reglen, &tmp, 100);
    device_id |= tmp;

    if ((device_id != 0) && (device_id != 0xffff))
        ret = 1;

    return ret;
}

static int _haveack(k210cambus_t *bus, uint8_t addr, int type)
{
    int ret = 0;

    if (type == 1)
    {
        ret = _getid_type1(bus, addr, bus->reg_len);
    }
    else
    {
    }

    return ret;
}

static int k210cambus_sccb_write1(k210cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint8_t d)
{
    return _dvp_sccb_send_data(slv_addr, reg_addr, d, bus->reg_len);
}

static int k210cambus_sccb_read1(k210cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint8_t *d)
{
    *d = _dvp_sccb_receive_data(slv_addr, reg_addr, bus->reg_len);

    return 0;
}

int k210cambus_init(k210cambus_t *bus)
{
    /* Init DVP IO map and function settings */
    fpioa_set_function(BSP_CAMERA_CMOS_RST_PIN, FUNC_CMOS_RST);
    fpioa_set_function(BSP_CAMERA_CMOS_PWDN_PIN, FUNC_CMOS_PWDN);
    fpioa_set_function(BSP_CAMERA_CMOS_XCLK_PIN, FUNC_CMOS_XCLK);
    fpioa_set_function(BSP_CAMERA_CMOS_VSYNC_PIN, FUNC_CMOS_VSYNC);
    fpioa_set_function(BSP_CAMERA_CMOS_HREF_PIN, FUNC_CMOS_HREF);
    fpioa_set_function(BSP_CAMERA_CMOS_PCLK_PIN, FUNC_CMOS_PCLK);
    fpioa_set_function(BSP_CAMERA_SCCB_SCLK_PIN, FUNC_SCCB_SCLK);
    fpioa_set_function(BSP_CAMERA_SCCB_SDA_PIN, FUNC_SCCB_SDA);

    sysctl_set_spi0_dvp_data(1);

    dvp_init(8);
    //dvp_set_output_enable(0, 1);
    //dvp_set_output_enable(1, 1);
    dvp_set_xclk_rate(50000000);
    dvp_enable_burst();

    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, cambus_irq, (void *)&bus);

    dvp_disable_auto();

    return 0;
}

void k210cambus_set_enable(k210cambus_t *bus, int en)
{
}

int k210cambus_scan(k210cambus_t *bus, int type)
{
    int addr = 0;

    bus->type = 0;
    for (uint8_t a = 0x08; a <= 0x7f; a++)
    {
        bus->reg_len = 16;
        if (_haveack(bus, a, type))
        {
            addr = a;
            bus->type = type;
            goto _out;
        }

        bus->reg_len = 8;
        if (_haveack(bus, a, type))
        {
            addr = a;
            bus->type = type;
            goto _out;
        }
    }

_out:
    if (bus->type == 1)
    {
        bus->writereg = k210cambus_sccb_write1;
        bus->readreg = k210cambus_sccb_read1;
    }

    return addr;
}

int k210cambus_ioctl(k210cambus_t *bus, int cmd, int arg)
{
    int ret = -1;

    return ret;
}

int k210cambus_read(k210cambus_t *bus, uint32_t dis, uint32_t ai, int timeout_ms)
{
    uint32_t start;

    g_dvp_finish_flag = 0;

    if (!dis && !ai)
        return -1;

    dvp_set_output_enable(0, 0);
    dvp_set_output_enable(1, 0);

    if (dis)
    {
        dvp_set_display_addr(dis);
        dvp_set_output_enable(1, 1);
    }
    if (ai)
    {
        dvp_set_ai_addr(ai, ai + bus->w * bus->h, ai + (bus->w * bus->h) * 2);
        dvp_set_output_enable(0, 1);
    }

    cam_run(1);

    start = rt_tick_get();
    while (g_dvp_finish_flag == 0)
    {
        _ndelay(50);
        if ((rt_tick_get() - start) > rt_tick_from_millisecond(timeout_ms)) //wait for 30ms
            return -1;
    }

    if (dis)
    {
        uint16_t tmp, *pix = (uint16_t*)dis;
        int i;

        for (i = 0; i < (bus->w * bus->h);)
        {
            tmp = pix[i];
            pix[i] = pix[i + 1];
            pix[i + 1] = tmp;
            i += 2;
        }
    }

    return 0;
}

int k210cambus_set_pixformat(k210cambus_t *bus, int *pixformat)
{
    int ret = 0;

    switch (*pixformat)
    {
    case PIXFORMAT_RGB565:
    {
        dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    }
    break;
    case PIXFORMAT_YUV422:
    {
        dvp_set_image_format(DVP_CFG_YUV_FORMAT);
        *pixformat = PIXFORMAT_RGB565;
    }
    break;
    case PIXFORMAT_GRAYSCALE:
    {
        dvp_set_image_format(DVP_CFG_Y_FORMAT);
        *pixformat = PIXFORMAT_RGB565;
    }
    break;
    default:
    {
        ret = -1;
    }
    break;
    }

    return ret;
}

int k210cambus_set_image_size(k210cambus_t *bus, uint32_t width, uint32_t height)
{
    dvp_set_image_size(width, height);
    bus->w = width;
    bus->h = height;

    return 0;
}

/**
 * 
 * */
int k210sensor_reset(k210sensor_t *sensor)
{
    int chip_id;
    int ret = 0;

    if (!sensor->inited)
    {
        sensor->inited = 1;
        k210cambus_init(&sensor->cambus);
    }

    k210cambus_set_enable(&sensor->cambus, 0);

    sensor->slv_addr = k210cambus_scan(&sensor->cambus, 2);
    if (sensor->slv_addr == 0)
    {
        sensor->slv_addr = k210cambus_scan(&sensor->cambus, 1);
    }

    if (sensor->slv_addr == 0)
        return -1;

    switch (sensor->slv_addr)
    {
    case 0x78:
        cam_ov5640_init(sensor);
        break;
    default:
        ret = -1;
        break;
    }

    if (ret != 0)
        return -1;

    if (!sensor->reset)
        return -1;

    sensor->reset(sensor);

    return ret;
}

int k210sensor_ioctl(k210sensor_t *sensor, int cmd, int arg)
{
    int ret = -1;

    return ret;
}

int k210sensor_read(k210sensor_t *sensor, void *dis, void *ai)
{
    if (!sensor->inited)
        return -1;

    return k210cambus_read(&sensor->cambus, (uint32_t)dis, (uint32_t)ai, 50);
}

int k210sensor_set_pixformat(k210sensor_t *sensor, k210sensor_pixformat_t pixformat)
{
    if (sensor->pixformat == pixformat)
        return 0;

    if (!sensor->set_pixformat || (sensor->set_pixformat(sensor, pixformat) != 0))
        return -1;

    if (k210cambus_set_pixformat(&sensor->cambus, &pixformat) != 0)
        return -1;

    sensor->pixformat = pixformat;
    sensor->bpp = 2;

    return 0;
}

int k210sensor_set_framesize(k210sensor_t *sensor, int width, int height)
{
    if (!sensor->set_framesize || (sensor->set_framesize(sensor, width, height) != 0))
        return -1;

    if (k210cambus_set_image_size(&sensor->cambus, width, height) != 0)
        return -1;

    sensor->w = width;
    sensor->h = height;

    return 0;
}
