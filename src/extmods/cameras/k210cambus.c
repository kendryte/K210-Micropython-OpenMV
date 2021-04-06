#include "plic.h"
#include "dvp.h"
#include "fpioa.h"
#include "sysctl.h"
#include "k210cambus.h"

#include <rtconfig.h>
#include <rtthread.h>

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

static int k210cambus_sccb_write(k210cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint8_t d)
{
    return _dvp_sccb_send_data(slv_addr, reg_addr, d, bus->reg_len);
}

static int k210cambus_sccb_read(k210cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint8_t *d)
{
    *d = _dvp_sccb_receive_data(slv_addr, reg_addr, bus->reg_len);

    return 0;
}

int cambus_writeb(k210cambus_t *bus, uint8_t addr, uint16_t reg, uint8_t d)
{
    return bus->writereg(bus, addr, reg, d);
}

int cambus_readb(k210cambus_t *bus, uint8_t addr, uint16_t reg, uint8_t *d)
{
    return bus->readreg(bus, addr, reg, d);
}

void k210cambus_sccb_sda_switch(int num)
{
    if (num) {
		fpioa_set_function(BSP_CAMERA_SCCB_SDA0_PIN, FUNC_RESV0);
		fpioa_set_function(BSP_CAMERA_SCCB_SDA_PIN, FUNC_SCCB_SDA);
    } else {
        fpioa_set_function(BSP_CAMERA_SCCB_SDA_PIN, FUNC_RESV0);
		fpioa_set_function(BSP_CAMERA_SCCB_SDA0_PIN, FUNC_SCCB_SDA);
    } 
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
    k210cambus_sccb_sda_switch(1);

    sysctl_set_spi0_dvp_data(1);

    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();

    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, cambus_irq, (void *)&bus);

    dvp_disable_auto();

    bus->writereg = k210cambus_sccb_write;
    bus->readreg = k210cambus_sccb_read;

    return 0;
}

int k210cambus_scan(k210cambus_t *bus, sensor_type_t type)
{
    int addr = 0;
    uint8_t device_id = 0;
    uint16_t id_reg = 0;

    if (type == SENSOR_TYPE_OV2640) {            // OV2640
        bus->reg_len = 8;
        addr = OV2640_SLV_ADDR;
        id_reg = 0x0A;
    } else if (type == SENSOR_TYPE_OV5640) {     // OV5640
        bus->reg_len = 16;
        addr = OV5640_SLV_ADDR;
        id_reg = 0x300A;
    } else if (type == SENSOR_TYPE_GC0328) {     // GC0328
        bus->reg_len = 8;
        addr = GC0328_SLV_ADDR;
        id_reg = 0xf0;
    } else {

    }
    bus->readreg(bus, addr, id_reg, &device_id);

    if ((device_id != 0) && (device_id != 0xff))
        return addr;
    else
        return 0;
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

int k210cambus_set_pixformat(k210cambus_t *bus, k210cambus_pixformat_t *pixformat)
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

int k210cambus_set_xclk_rate(k210cambus_t *bus, uint32_t xclk_rate)
{
    dvp_set_xclk_rate(xclk_rate);

    return 0;
}