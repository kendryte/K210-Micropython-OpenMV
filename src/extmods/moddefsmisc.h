#ifndef __MODDEFSMISC_H
#define __MODDEFSMISC_H

#ifdef EXTMODS_MISC_USING_K210
extern const struct _mp_obj_module_t k210_module;
#define MPY_MODDEFS_K210 {MP_ROM_QSTR(MP_QSTR_k210), MP_ROM_PTR(&k210_module) },
#else
#define MPY_MODDEFS_K210
#endif

#if (defined(EXTMODS_MISC_USING_LVGL))
extern const struct _mp_obj_module_t mp_module_lvgl;
extern const struct _mp_obj_module_t lvdrv_module;
#define MPY_MODDEFS_LVGL \
    {MP_ROM_QSTR(MP_QSTR_lvgl), MP_ROM_PTR(&mp_module_lvgl) }, \
    {MP_ROM_QSTR(MP_QSTR_lvdrv), MP_ROM_PTR(&lvdrv_module) }, \

#else
#define MPY_MODDEFS_LVGL
#endif

#define MPY_MODDEFS_MISC \
    MPY_MODDEFS_K210 \
    MPY_MODDEFS_LVGL \

#endif
