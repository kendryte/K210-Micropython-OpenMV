#ifndef _K210CAM_H
#define _K210CAM_H

#include <stdint.h>

typedef enum 
{
    PIXFORMAT_INVALID = 0,
    PIXFORMAT_BINARY,    // 1BPP/BINARY
    PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
    PIXFORMAT_RGB565,    // 2BPP/RGB565
    PIXFORMAT_YUV422,    // 2BPP/YUV422
    PIXFORMAT_BAYER,     // 1BPP/RAW
    PIXFORMAT_JPEG,      // JPEG/COMPRESSED
} k210sensor_pixformat_t;

typedef struct _k210cambus k210cambus_t;
struct _k210cambus
{
    int type;
    int reg_len;
    uint32_t w, h;

    int (*readreg)(k210cambus_t *bus, uint8_t  slv_addr, uint16_t reg_addr, uint8_t *d);
    int (*writereg)(k210cambus_t *bus, uint8_t  slv_addr, uint16_t reg_addr, uint8_t d);
};

typedef struct _k210sensor k210sensor_t;
struct _k210sensor
{
    int inited;

    k210cambus_t cambus;
    uint8_t  slv_addr;
    k210sensor_pixformat_t pixformat;
    uint32_t w;
    uint32_t h;
    int bpp;

    int (*reset)(k210sensor_t *sensor);
    int (*get_chipid)(k210sensor_t *sensor);
    int (*set_pixformat)(k210sensor_t *sensor, k210sensor_pixformat_t pf);
    int (*set_framesize)(k210sensor_t *sensor, int width, int height);
};

int k210cambus_init(k210cambus_t *bus);
int k210cambus_scan(k210cambus_t *bus, int type);

int k210sensor_reset(k210sensor_t *sensor);
int k210sensor_read(k210sensor_t *sensor, void *dis, void *ai);
int k210sensor_set_framesize(k210sensor_t *sensor, int width, int height);

#endif
