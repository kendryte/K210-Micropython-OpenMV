#ifndef _K210CAMBUS_H
#define _K210CAMBUS_H

#include <stdint.h>

// #define TYPE_OV2640     0
// #define TYPE_OV5640     1
// #define TYPE_GC0328     2

#define OV2640_SLV_ADDR     (0x60)
#define OV5640_SLV_ADDR     (0x78)
#define GC0328_SLV_ADDR     (0x42)

#define OV5640_XCLK_RATE    (40000000)
#define OV2640_XCLK_RATE    (24000000)
#define GC0328_XCLK_RATE    (24000000)

typedef enum 
{
    PIXFORMAT_INVALID = 0,
    PIXFORMAT_BINARY,    // 1BPP/BINARY
    PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
    PIXFORMAT_RGB565,    // 2BPP/RGB565
    PIXFORMAT_YUV422,    // 2BPP/YUV422
    PIXFORMAT_BAYER,     // 1BPP/RAW
    PIXFORMAT_JPEG,      // JPEG/COMPRESSED
} k210cambus_pixformat_t;

typedef enum {
    SENSOR_TYPE_OV2640,
    SENSOR_TYPE_OV5640,
    SENSOR_TYPE_GC0328
} sensor_type_t;

typedef struct _k210cambus k210cambus_t;
struct _k210cambus
{
    int type;
    int reg_len;
    uint32_t w, h;

    int (*readreg)(k210cambus_t *bus, uint8_t  slv_addr, uint16_t reg_addr, uint8_t *d);
    int (*writereg)(k210cambus_t *bus, uint8_t  slv_addr, uint16_t reg_addr, uint8_t d);
};

int k210cambus_init(k210cambus_t *bus);
void k210cambus_set_enable(k210cambus_t *bus, int en);
int k210cambus_scan(k210cambus_t *bus, sensor_type_t type);
int k210cambus_read(k210cambus_t *bus, uint32_t dis, uint32_t ai, int timeout_ms);
int k210cambus_set_pixformat(k210cambus_t *bus, k210cambus_pixformat_t *pixformat);
int k210cambus_set_image_size(k210cambus_t *bus, uint32_t width, uint32_t height);
int k210cambus_set_xclk_rate(k210cambus_t *bus, uint32_t xclk_rate);
int cambus_writeb(k210cambus_t *bus, uint8_t addr, uint16_t reg, uint8_t d);
int cambus_readb(k210cambus_t *bus, uint8_t addr, uint16_t reg, uint8_t *d);
void k210cambus_sccb_sda_switch(int num);

#endif /* _K210CAMBUS_H */