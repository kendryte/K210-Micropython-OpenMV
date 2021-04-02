#include <stdlib.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/objarray.h"
#include "py/binary.h"

#ifdef EXTMODS_K210_DVP
#include "../cameras/k210cam.h"
#include "py_picture.h"

typedef struct
{
    mp_obj_base_t base;

    k210sensor_t sensor;
    void *dis;
    void *ai;
}k210_cam_obj_t;

const mp_obj_type_t k210_cam_type;

STATIC mp_obj_t k210_cam_reset(mp_obj_t self)
{
    k210_cam_obj_t *obj = (k210_cam_obj_t *)self;

    if (k210sensor_reset(&obj->sensor) != 0)
    {
        mp_raise_OSError(-1);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(k210_cam_reset_obj, k210_cam_reset);

STATIC mp_obj_t k210_cam_set_pixformat(mp_obj_t self, mp_obj_t pixformat)
{
    k210_cam_obj_t *obj = (k210_cam_obj_t *)self;

    if (k210sensor_set_pixformat(&obj->sensor, mp_obj_get_int(pixformat)) != 0)
    {
        mp_raise_OSError(-1);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(k210_cam_set_pixformat_obj, k210_cam_set_pixformat);

static mp_obj_t k210_cam_set_framesize(mp_obj_t self, mp_obj_t width, mp_obj_t height) 
{
    k210_cam_obj_t *obj = (k210_cam_obj_t *)self;
    int w, h, ow, oh;

    ow = obj->sensor.w;
    oh = obj->sensor.h;
    w = mp_obj_get_int(width);
    h = mp_obj_get_int(height);
    if ((w < 1) || (h < 1) || (w > 2048) || (h > 2048))
    {
        mp_raise_ValueError("invalid Width or Height");
    }

    if (k210sensor_set_framesize(&obj->sensor, w, h) != 0)
    {
        mp_raise_OSError(-1);
    }

    if ((w * h) != (ow * oh))
    {
        char *ptr;

        ptr = (char*)realloc(obj->dis, (w * h) * (2 + 3));
        if (!ptr)
        {
            mp_raise_msg_varg(&mp_type_MemoryError, MP_ERROR_TEXT("no memory"));
        }

        obj->dis = ptr;
        obj->ai = ptr + (w * h * 2);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(k210_cam_set_framesize_obj, k210_cam_set_framesize);

static mp_obj_t k210_cam_snapshot(mp_obj_t self)
{
    k210_cam_obj_t *obj = (k210_cam_obj_t *)self;
    mp_obj_t img;
    k210sensor_t *s = &obj->sensor;
    int ret;

    if (!obj->dis)
    {
        mp_raise_ValueError("framebuffer not init");
    }

    img = py_picture(s->w, s->h, s->bpp, obj->dis, obj->ai);
    ret = k210sensor_read(s, obj->dis, obj->ai);
    if (ret < 0) 
    {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_RuntimeError, "Capture Failed: %d", ret));
    }

    return img;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(k210_cam_snapshot_obj, k210_cam_snapshot);

static mp_obj_t k210_cam_del(mp_obj_t self)
{
    k210_cam_obj_t *obj = (k210_cam_obj_t *)self;

    if (obj->dis)
    {
        realloc(obj->dis, 0);
        obj->dis = 0;
        obj->ai = 0;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(k210_cam_del_obj, k210_cam_del);

STATIC mp_obj_t make_new()
{
	k210_cam_obj_t *self = m_new_obj_with_finaliser(k210_cam_obj_t);

	self->base.type = &k210_cam_type;

    return self;
}

STATIC const mp_rom_map_elem_t locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&k210_cam_del_obj)},

    {MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&k210_cam_reset_obj)},
    {MP_ROM_QSTR(MP_QSTR_set_pixformat), MP_ROM_PTR(&k210_cam_set_pixformat_obj)},
    {MP_ROM_QSTR(MP_QSTR_set_framesize), MP_ROM_PTR(&k210_cam_set_framesize_obj)},
    {MP_ROM_QSTR(MP_QSTR_snapshot), MP_ROM_PTR(&k210_cam_snapshot_obj)},

    {MP_ROM_QSTR(MP_QSTR_RGB565), MP_ROM_INT(PIXFORMAT_RGB565)},   /* 2BPP/RGB565*/
    {MP_ROM_QSTR(MP_QSTR_YUV422), MP_ROM_INT(PIXFORMAT_YUV422)},   /* 2BPP/YUV422*/

};
STATIC MP_DEFINE_CONST_DICT(k210_cam_dict, locals_dict_table);

const mp_obj_type_t k210_cam_type = {
    .base = { &mp_type_type },
    .make_new = make_new,
    .name = MP_QSTR_camera,
    .locals_dict = (mp_obj_t)&k210_cam_dict,
};
#endif
