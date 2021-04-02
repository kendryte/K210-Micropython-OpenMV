/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Common macros.
 */
#ifndef __OMV_COMMON_H__
#ifndef OMV_ATTR_ALWAYS_INLINE
#define OMV_ATTR_ALWAYS_INLINE inline __attribute__((always_inline))
#endif

#ifndef BREAK
#define BREAK() __asm__ volatile ("BKPT")
#endif

#ifndef DISABLE_OPT
#define DISABLE_OPT __attribute__((optimize("O0")))
#endif

#ifdef DEBUG_PRINTF
#define debug_printf(fmt, ...) \
            do { printf("%s(): " fmt, __func__, ##__VA_ARGS__);} while (0)
#else
#define debug_printf(...)
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#endif //__OMV_COMMON_H__
