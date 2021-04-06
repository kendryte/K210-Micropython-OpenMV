#include "k210cam.h"
#include "k210cambus.h"

extern void cam_ov5640_init(k210sensor_t *s);
extern void cam_ov2640_init(k210sensor_t *s);
extern void cam_gc0328_init(k210sensor_t *s);
extern void open_gc0328_x(k210sensor_t *sensor, int num);

sensor_type_t sensor_type[] = {
    SENSOR_TYPE_OV2640,
    SENSOR_TYPE_OV5640,
    SENSOR_TYPE_GC0328
};
/**
 * 
 * */
int k210sensor_reset(k210sensor_t *sensor)
{
    int chip_id;
    int ret = 0;

    k210cambus_init(&sensor->cambus);

    for (int i = 0; i < sizeof(sensor_type) / sizeof(sensor_type[0]); i++) {
        sensor->slv_addr = k210cambus_scan(&sensor->cambus, sensor_type[i]);
        if (sensor->slv_addr)
            break;
    }

    if (sensor->slv_addr == 0)
        return -1;

    switch (sensor->slv_addr)
    {
    case OV5640_SLV_ADDR:
        sensor->cambus.reg_len = 16;
        k210cambus_set_xclk_rate(&sensor->cambus, OV5640_XCLK_RATE);
        cam_ov5640_init(sensor);
        break;
    case OV2640_SLV_ADDR:
        sensor->cambus.reg_len = 8;
        k210cambus_set_xclk_rate(&sensor->cambus, OV2640_XCLK_RATE);
        cam_ov2640_init(sensor);
        break;
    case GC0328_SLV_ADDR:
        sensor->cambus.reg_len = 8;
        k210cambus_set_xclk_rate(&sensor->cambus, GC0328_XCLK_RATE);
        cam_gc0328_init(sensor);
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

    sensor->inited = 1;

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

    return k210cambus_read(&sensor->cambus, (uint32_t)dis, (uint32_t)ai, 200);
}

int k210sensor_set_pixformat(k210sensor_t *sensor, k210cambus_pixformat_t pixformat)
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

void k210sensor_switch(k210sensor_t *sensor, int sel)
{
    if(sensor->slv_addr != GC0328_SLV_ADDR){
        return;
    }
    if(sel == 0){
        open_gc0328_x(sensor, 0);
    }
    if(sel == 1){
        open_gc0328_x(sensor, 1);
    }
}

