#include <rtthread.h>
#include "k210cam.h"
#include "ov2640_regs.h"
#include "k210cambus.h"

#define CIF_WIDTH       (400)
#define CIF_HEIGHT      (296)

#define SVGA_WIDTH      (800)
#define SVGA_HEIGHT     (600)

#define UXGA_WIDTH      (1600)
#define UXGA_HEIGHT     (1200)

const uint8_t default_regs[][2] = {
//    {0xff, 0x01},
//    {0x12, 0x80},
    {0xff, 0x00},
    {0x2c, 0xff},
    {0x2e, 0xdf},
    {0xff, 0x01},
    {0x3c, 0x32},
    {0x11, 0x00},
    {0x09, 0x02},
    {0x04, 0x58},
    {0x13, 0xe5},
    {0x14, 0x48},
    {0x2c, 0x0c},
    {0x33, 0x78},
    {0x3a, 0x33},
    {0x3b, 0xfb},
    {0x3e, 0x00},
    {0x43, 0x11},
    {0x16, 0x10},
    {0x39, 0x92},
    {0x35, 0xda},
    {0x22, 0x1a},
    {0x37, 0xc3},
    {0x23, 0x00},
    {0x34, 0xc0},
    {0x36, 0x1a},
    {0x06, 0x88},
    {0x07, 0xc0},
    {0x0d, 0x87},
    {0x0e, 0x41},
    {0x4c, 0x00},
    {0x48, 0x00},
    {0x5b, 0x00},
    {0x42, 0x03},
    {0x4a, 0x81},
    {0x21, 0x99},
    {0x24, 0x40},
    {0x25, 0x38},
    {0x26, 0x82},
    {0x5c, 0x00},
    {0x63, 0x00},
    {0x46, 0x22},
    {0x0c, 0x3c},
    {0x61, 0x70},
    {0x62, 0x80},
    {0x7c, 0x05},
    {0x20, 0x80},
    {0x28, 0x30},
    {0x6c, 0x00},
    {0x6d, 0x80},
    {0x6e, 0x00},
    {0x70, 0x02},
    {0x71, 0x94},
    {0x73, 0xc1},
    {0x3d, 0x34},
    {0x5a, 0x57},
    {0x12, 0x40},
    {0x17, 0x11},
    {0x18, 0x43},
    {0x19, 0x00},
    {0x1a, 0x4b},
    {0x32, 0x09},
    {0x37, 0xc0},
    {0x4f, 0xca},
    {0x50, 0xa8},
    {0x5a, 0x23},
    {0x6d, 0x00},
    {0x3d, 0x38},
    {0xff, 0x00},
    {0xe5, 0x7f},
    {0xf9, 0xc0},
    {0x41, 0x24},
    {0xe0, 0x14},
    {0x76, 0xff},
    {0x33, 0xa0},
    {0x42, 0x20},
    {0x43, 0x18},
    {0x4c, 0x00},
    {0x87, 0xd5},
    {0x88, 0x3f},
    {0xd7, 0x03},
    {0xd9, 0x10},
    {0xd3, 0x82},
    {0xc8, 0x08},
    {0xc9, 0x80},
    {0x7c, 0x00},
    {0x7d, 0x00},
    {0x7c, 0x03},
    {0x7d, 0x48},
    {0x7d, 0x48},
    {0x7c, 0x08},
    {0x7d, 0x20},
    {0x7d, 0x10},
    {0x7d, 0x0e},
    {0x90, 0x00},
    {0x91, 0x0e},
    {0x91, 0x1a},
    {0x91, 0x31},
    {0x91, 0x5a},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7e},
    {0x91, 0x88},
    {0x91, 0x8f},
    {0x91, 0x96},
    {0x91, 0xa3},
    {0x91, 0xaf},
    {0x91, 0xc4},
    {0x91, 0xd7},
    {0x91, 0xe8},
    {0x91, 0x20},
    {0x92, 0x00},
    {0x93, 0x06},
    {0x93, 0xe3},
    {0x93, 0x05},
    {0x93, 0x05},
    {0x93, 0x00},
    {0x93, 0x04},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x96, 0x00},
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0c},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    {0xc3, 0xed},
    {0xa4, 0x00},
    {0xa8, 0x00},
    {0xc5, 0x11},
    {0xc6, 0x51},
    {0xbf, 0x80},
    {0xc7, 0x10},
    {0xb6, 0x66},
    {0xb8, 0xa5},
    {0xb7, 0x64},
    {0xb9, 0x7c},
    {0xb3, 0xaf},
    {0xb4, 0x97},
    {0xb5, 0xff},
    {0xb0, 0xc5},
    {0xb1, 0x94},
    {0xb2, 0x0f},
    {0xc4, 0x5c},
    {0xc0, 0x64},
    {0xc1, 0x4b},
    {0x8c, 0x00},
    {0x86, 0x3d},
    {0x50, 0x00},
    {0x51, 0xc8},
    {0x52, 0x96},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    {0x5a, 0xc8},
    {0x5b, 0x96},
    {0x5c, 0x00},
    {0xd3, 0x02},
    {0xc3, 0xed},
    {0x7f, 0x00},
    {0xda, 0x08},
    {0xe5, 0x1f},
    {0xe1, 0x67},
    {0xe0, 0x00},
    {0xdd, 0x7f},
    {0x05, 0x00},
    {0xff, 0x00},
    {0xe0, 0x04},
    {0x5a, 0x50},
    {0x5b, 0x3c},
    {0x5c, 0x00},
    {0xe0, 0x00},
    {0x00, 0x00}
};

static const uint8_t svga_regs[][2] = {
    {BANK_SEL,  BANK_SEL_SENSOR},
    {COM7,      COM7_RES_SVGA},
    {COM1,      0x0A | 0x80},
    {HSTART,    0x11},
    {HSTOP,     0x43},
    {VSTART,    0x01}, // 0x01 fixes issue with garbage pixels in the image...
    {VSTOP,     0x97},
    {REG32,     0x09},
    {BANK_SEL,  BANK_SEL_DSP},
    {RESET,     RESET_DVP},
    {SIZEL,     SIZEL_HSIZE8_11_SET(SVGA_WIDTH) | SIZEL_HSIZE8_SET(SVGA_WIDTH) | SIZEL_VSIZE8_SET(SVGA_HEIGHT)},
    {HSIZE8,    HSIZE8_SET(SVGA_WIDTH)},
    {VSIZE8,    VSIZE8_SET(SVGA_HEIGHT)},
    {CTRL2,     CTRL2_DCW_EN | CTRL2_SDE_EN | CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN},
    {0,         0},
};

static const uint8_t uxga_regs[][2] = {
    {BANK_SEL,  BANK_SEL_SENSOR},
    {COM7,      COM7_RES_UXGA},
    {COM1,      0x0F | 0x80},
    {HSTART,    0x11},
    {HSTOP,     0x75},
    {VSTART,    0x01},
    {VSTOP,     0x97},
    {REG32,     0x36},
    {BANK_SEL,  BANK_SEL_DSP},
    {RESET,     RESET_DVP},
    {SIZEL,     SIZEL_HSIZE8_11_SET(UXGA_WIDTH) | SIZEL_HSIZE8_SET(UXGA_WIDTH) | SIZEL_VSIZE8_SET(UXGA_HEIGHT)},
    {HSIZE8,    HSIZE8_SET(UXGA_WIDTH)},
    {VSIZE8,    VSIZE8_SET(UXGA_HEIGHT)},
    {CTRL2,     CTRL2_DCW_EN | CTRL2_SDE_EN | CTRL2_UV_AVG_EN | CTRL2_CMX_EN | CTRL2_UV_ADJ_EN},
    {0,         0},
};

static const uint8_t yuv422_regs[][2] = {
    {BANK_SEL,      BANK_SEL_DSP},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {IMAGE_MODE,    IMAGE_MODE_YUV422},
    {0xd7,          0x03},
    {0x33,          0xa0},
    {0xe5,          0x1f},
    {0xe1,          0x67},
    {RESET,         0x00},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {0,             0},
};

static const uint8_t rgb565_regs[][2] = {
    {BANK_SEL,      BANK_SEL_DSP},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {IMAGE_MODE,    IMAGE_MODE_RGB565},
    {0xd7,          0x03},
    {RESET,         0x00},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {0,             0},
};

static const uint8_t bayer_regs[][2] = {
    {BANK_SEL,      BANK_SEL_DSP},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {IMAGE_MODE,    IMAGE_MODE_RAW10},
    {0xd7,          0x03},
    {RESET,         0x00},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {0,             0},
};

static const uint8_t jpeg_regs[][2] = {
    {BANK_SEL,      BANK_SEL_DSP},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {IMAGE_MODE,    IMAGE_MODE_JPEG_EN},
    {0xd7,          0x03},
    {RESET,         0x00},
    {R_BYPASS,      R_BYPASS_DSP_EN},
    {0,             0},
};

static int reset(k210sensor_t *sensor)
{
    // Reset all registers
    int ret = cambus_writeb(&sensor->cambus, sensor->slv_addr, BANK_SEL, BANK_SEL_SENSOR);
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, COM7, COM7_SRST);

    // Delay 5 ms
    rt_thread_mdelay(5);

    // Write default regsiters
    for (int i = 0; default_regs[i][0]; i++) {
        ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, default_regs[i][0], default_regs[i][1]);
    }

    // Delay 300 ms
    rt_thread_mdelay(300);

    return ret;
}

static int get_chipid(k210sensor_t *sensor)
{
    uint8_t tmp;
    int ret = 0;
    uint16_t device_id = 0;

    ret |= cambus_readb(&sensor->cambus, sensor->slv_addr, 0x0A, &tmp);
    device_id = tmp << 8;
    ret |= cambus_readb(&sensor->cambus, sensor->slv_addr, 0x0B, &tmp);
    device_id |= tmp;
    return device_id;

    // return 0;
}

static int set_pixformat(k210sensor_t *sensor, k210cambus_pixformat_t pixformat)
{
    const uint8_t (*regs)[2];
    int ret = 0;

    switch (pixformat) {
        case PIXFORMAT_RGB565:
            regs = rgb565_regs;
            break;
        case PIXFORMAT_YUV422:
        case PIXFORMAT_GRAYSCALE:
            regs = yuv422_regs;
            break;
        case PIXFORMAT_BAYER:
            regs = bayer_regs;
            break;
        case PIXFORMAT_JPEG:
            regs = jpeg_regs;
            break;
        default:
            return -1;
    }

    // Write regsiters
    for (int i = 0; regs[i][0]; i++) {
        ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, regs[i][0], regs[i][1]);
    }

    return ret;
}

static int set_framesize(k210sensor_t *sensor, int width, int height)
{
    const uint8_t (*regs)[2];
    uint16_t sensor_w = 0;
    uint16_t sensor_h = 0;
    int ret = 0;
    // uint16_t w = resolution[framesize][0];
    // uint16_t h = resolution[framesize][1];

    uint16_t w = width;
    uint16_t h = height;

    if ((w % 4) || (h % 4) || (w > UXGA_WIDTH) || (h > UXGA_HEIGHT)) { // w/h must be divisble by 4
        return -1;
    }

    // Looks really bad.
    /* if ((w <= CIF_WIDTH) && (h <= CIF_HEIGHT)) {
        regs = cif_regs;
        sensor_w = CIF_WIDTH;
        sensor_h = CIF_HEIGHT;
    } else */ if ((w <= SVGA_WIDTH) && (h <= SVGA_HEIGHT)) {
        regs = svga_regs;
        sensor_w = SVGA_WIDTH;
        sensor_h = SVGA_HEIGHT;
    } else {
        regs = uxga_regs;
        sensor_w = UXGA_WIDTH;
        sensor_h = UXGA_HEIGHT;
    }

    // Write setup regsiters
    for (int i = 0; regs[i][0]; i++) {
        ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, regs[i][0], regs[i][1]);
    }

    uint64_t tmp_div = IM_MIN(sensor_w / w, sensor_h / h);
    uint16_t log_div = IM_MIN(IM_LOG2(tmp_div) - 1, 3);
    uint16_t div = 1 << log_div;
    uint16_t w_mul = w * div;
    uint16_t h_mul = h * div;
    uint16_t x_off = (sensor_w - w_mul) / 2;
    uint16_t y_off = (sensor_h - h_mul) / 2;

    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, CTRLI, CTRLI_LP_DP | CTRLI_V_DIV_SET(log_div) | CTRLI_H_DIV_SET(log_div));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, HSIZE, HSIZE_SET(w_mul));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, VSIZE, VSIZE_SET(h_mul));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, XOFFL, XOFFL_SET(x_off));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, YOFFL, YOFFL_SET(y_off));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, VHYX, VHYX_HSIZE_SET(w_mul) | VHYX_VSIZE_SET(h_mul) | VHYX_XOFF_SET(x_off) | VHYX_YOFF_SET(y_off));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, TEST, TEST_HSIZE_SET(w_mul));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, ZMOW, ZMOW_OUTW_SET(w));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, ZMOH, ZMOH_OUTH_SET(h));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, ZMHH, ZMHH_OUTW_SET(w) | ZMHH_OUTH_SET(h));
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, R_DVP_SP, div);
    ret |= cambus_writeb(&sensor->cambus, sensor->slv_addr, RESET, 0x00);

    return ret;
}

void cam_ov2640_init(k210sensor_t *s)
{
    s->reset = reset;
    s->get_chipid = get_chipid;
    s->set_pixformat = set_pixformat;
    s->set_framesize = set_framesize;
}
