#ifndef __PY_PICTURE_H
#define __PY_PICTURE_H

#include <stdint.h>

typedef struct picture 
{
    int w;
    int h;
    int bpp;
    union 
    {
        uint8_t *pixels;
        uint8_t *data;
    };
    void *ai;
} picture_t;

mp_obj_t py_picture(int w, int h, int bpp, void *pixels, void *ai);

#endif
