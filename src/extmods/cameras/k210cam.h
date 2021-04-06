#ifndef _K210CAM_H
#define _K210CAM_H

#include "k210cambus.h"

#define IM_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define IM_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  IM_LOG2_2((x) >>  2)) :  IM_LOG2_2(x)) // NO ({ ... }) !
#define IM_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  IM_LOG2_4((x) >>  4)) :  IM_LOG2_4(x)) // NO ({ ... }) !
#define IM_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  IM_LOG2_8((x) >>  8)) :  IM_LOG2_8(x)) // NO ({ ... }) !
#define IM_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + IM_LOG2_16((x) >> 16)) : IM_LOG2_16(x)) // NO ({ ... }) !
#define IM_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + IM_LOG2_32((x) >> 32)) : IM_LOG2_32(x)) // NO ({ ... }) !

#define IM_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define IM_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

typedef struct _k210sensor k210sensor_t;
struct _k210sensor
{
    int inited;

    k210cambus_t cambus;
    uint8_t  slv_addr;
    k210cambus_pixformat_t pixformat;
    uint32_t w;
    uint32_t h;
    int bpp;

    int (*reset)(k210sensor_t *sensor);
    int (*get_chipid)(k210sensor_t *sensor);
    int (*set_pixformat)(k210sensor_t *sensor, k210cambus_pixformat_t pf);
    int (*set_framesize)(k210sensor_t *sensor, int width, int height);
};

int k210cambus_init(k210cambus_t *bus);
int k210cambus_scan(k210cambus_t *bus, sensor_type_t type);

int k210sensor_reset(k210sensor_t *sensor);
int k210sensor_read(k210sensor_t *sensor, void *dis, void *ai);
int k210sensor_set_pixformat(k210sensor_t *sensor, k210cambus_pixformat_t pixformat);
int k210sensor_set_framesize(k210sensor_t *sensor, int width, int height);
void k210sensor_switch(k210sensor_t *sensor, int sel);

#endif
