/**
 * @file lv_port_disp_templ.h
 *
 */

/*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lvgl/lvgl.h"

	/*********************
	 *      DEFINES
	 *********************/

	/**********************
	 *      TYPEDEFS
	 **********************/

	/**********************
	 * GLOBAL PROTOTYPES
	 **********************/

	/**********************
	 *      MACROS
	 **********************/
	extern void lv_port_disp_init(void);

	extern void gui_draw_area_set(const lv_area_t *area_t, int count);

	extern void fb_background_enable(bool en);

	extern void fb_video_mode_enable(bool en);

	extern bool is_fb_video_mode_enable(void);
	
	extern bool system_bg_loading(rom_bin_info *info,bool extern_src);

	extern unsigned long system_bg_phyaddres_get(void);

	extern bool system_bg_data_backup(void);

	extern bool system_bg_fill_color(unsigned int color, int x, int y, int w, int h);

	extern void system_bg_enable_set(bool en);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

#endif /*Disable/Enable content*/
