#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/objarray.h"
#include "py/binary.h"

#if (defined(EXTMODS_K210_DVP))
#include "py_picture.h"
#include <drv_lcd.h>

typedef struct
{
    mp_obj_base_t base;
    picture_t _cobj;
} py_picture_obj_t;

const mp_obj_type_t py_picture_type;

mp_obj_t py_picture(int w, int h, int bpp, void *pixels, void *ai)
{
    py_picture_obj_t *o = m_new0(py_picture_obj_t, 1);

    o->base.type = &py_picture_type;
    o->_cobj.w = w;
    o->_cobj.h = h;
    o->_cobj.bpp = bpp;
    o->_cobj.pixels = pixels;
    o->_cobj.ai = ai;

    return o;
}

picture_t *py_picture_cobj(mp_obj_t self)
{
    py_picture_obj_t *obj = (py_picture_obj_t *)self;

    if (obj->base.type != &py_picture_type)
    {
        mp_raise_TypeError("not a picture");
    }

    return &obj->_cobj;
}

static mp_obj_t py_picture_show(mp_obj_t self)
{
    picture_t *p = py_picture_cobj(self);

    if (p->pixels)
    {
        lcd_show_image(0, 0, p->h, p->w, p->pixels);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_picture_show_obj, py_picture_show);

STATIC mp_obj_t make_new()
{
    mp_obj_t *self = py_picture(0, 0, 0, 0, 0);

    return self;
}

STATIC const mp_rom_map_elem_t locals_dict_table[] = 
{
    {MP_ROM_QSTR(MP_QSTR_show), MP_ROM_PTR(&py_picture_show_obj)},

};
STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

const mp_obj_type_t py_picture_type =
{
    {&mp_type_type},
    .name = MP_QSTR_picture,
    .make_new = make_new,
    .locals_dict = (mp_obj_t)&locals_dict
};
#endif
