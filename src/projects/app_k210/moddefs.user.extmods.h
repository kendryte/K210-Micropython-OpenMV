#ifndef _MODDEFS_USER_EXTMODS
#define _MODDEFS_USER_EXTMODS

#ifdef PKG_USING_OPENMV_CP
#include "moddefsomv.h"
#else
#define MPY_MODDEFS_OMV
#endif

#ifdef PRJ_USING_EXTMODS_MISC
#include "moddefsmisc.h"
#else
#define MPY_MODDEFS_MISC
#endif

#define MICROPY_USER_MODULES \
    MPY_MODDEFS_OMV\
    MPY_MODDEFS_MISC\

#define MICROPY_REGISTERED_MODULES \
    {MP_ROM_QSTR(MP_QSTR_uarray), MP_ROM_PTR(&mp_module_uarray)},\


#endif
