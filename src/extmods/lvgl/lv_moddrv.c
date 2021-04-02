#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/objarray.h"
#include "py/binary.h"

#include "lvgl.h"
#include "littlevgl2rtt.h"

STATIC mp_obj_t mp_init_lvdrv()
{
    int err;

    err = littlevgl2rtt_init("lcd");

    return err == 0 ? mp_const_true : mp_const_false;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_init_lvdrv_obj, mp_init_lvdrv);

STATIC mp_obj_t mp_deinit_lvdrv()
{
    littlevgl2rtt_deinit();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_deinit_lvdrv_obj, mp_deinit_lvdrv);

STATIC const mp_rom_map_elem_t lvdrv_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvdrv)},
    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_lvdrv_obj)},
    {MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_deinit_lvdrv_obj)},

};

STATIC MP_DEFINE_CONST_DICT(lvdrv_module_globals, lvdrv_globals_table);

const mp_obj_module_t lvdrv_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&lvdrv_module_globals};
