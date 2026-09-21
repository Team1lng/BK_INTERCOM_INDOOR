#ifndef _LV_JPG_DECODE_H_
#define _LV_JPG_DECODE_H_
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "../lvgl/lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif
#include <stdbool.h>
/***
**   日期:2022-07-01 13:50:29
**   作者: leo.liu
**   函数作用：获取解码的数据
**   参数说明:
***/
bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h);

#endif