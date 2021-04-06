#if (defined(EXTMODS_K210_DVP))
QDEF(MP_QSTR_camera, (const byte*)"\xdc\x06" "camera")
QDEF(MP_QSTR_picture, (const byte*)"\xa9\x07" "picture")
#endif

#ifndef PKG_USING_OPENMV_CP
QDEF(MP_QSTR_set_pixformat, (const byte*)"\x3a\x0d" "set_pixformat")        
QDEF(MP_QSTR_set_framesize, (const byte*)"\x20\x0d" "set_framesize")        
QDEF(MP_QSTR_snapshot, (const byte*)"\x49\x08" "snapshot")
QDEF(MP_QSTR_sw, (const byte*)"\xe1\x02" "sw")
QDEF(MP_QSTR_RGB565, (const byte*)"\x64\x06" "RGB565")
QDEF(MP_QSTR_YUV422, (const byte*)"\x0b\x06" "YUV422")

#endif
