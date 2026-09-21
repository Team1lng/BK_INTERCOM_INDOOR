#include "layout_define.h"

static void keyboard_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *children = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(children, state);
}

static void keyboard_btn_img_transform_set(lv_obj_t *obj)
{
	lv_obj_set_style_local_transform_zoom(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 256);
	lv_obj_set_style_local_transform_zoom(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 300);

	lv_obj_set_style_local_transition_prop_1(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_STYLE_TRANSFORM_ZOOM);
	lv_obj_set_style_local_transition_prop_2(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_STYLE_TRANSFORM_ZOOM);

	lv_obj_set_style_local_transition_time(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 200);
	lv_obj_set_style_local_transition_time(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 200);

	static lv_anim_path_t path;
	path.cb = lv_anim_path_overshoot,
	path.user_data = NULL;
	lv_obj_set_style_local_transition_path(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &path);
	lv_obj_set_style_local_transition_path(obj, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, &path);
}

static lv_obj_t *keyboard_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color)
{
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);

	if (bg_color == true)
	{
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_100);

		lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 45);
		lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 45);
	}
	else
	{
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
	}

	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img, img_src);

	keyboard_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);

	return btn;
}

static void keyboard_cancel_btn_down(lv_obj_t *obj)
{
	keyboard_btn_state_set(obj, LV_STATE_PRESSED);
}

static void keyboard_cancel_btn_up(lv_obj_t *obj)
{
	keyboard_btn_state_set(obj, LV_STATE_DEFAULT);

	goto_layout(pLAYOUT(home));
}
static void keyboard_cancel_btn_create(void)
{
	static btn_data btn_data = btn_data_create(keyboard_cancel_btn_down, keyboard_cancel_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	keyboard_btn_create(25, 25, 60, 60, &btn_data, &info, true);
}

static void keyboard_create(void)
{

	lv_obj_t *obj = lv_keyboard_create(lv_scr_act(), NULL);

	static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
	lv_keyboard_img_set(obj, 11, &info11);

	static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
	lv_keyboard_img_set(obj, 22, &info22);

	static rom_bin_info info35 = rom_bin_info_get(ROM_RES_KB_CANCEL_PNG);
	lv_keyboard_img_set(obj, 35, &info35);

	static rom_bin_info info36 = rom_bin_info_get(ROM_RES_KB_LEFT_PNG);
	lv_keyboard_img_set(obj, 36, &info36);

	static rom_bin_info info37 = rom_bin_info_get(ROM_RES_KB_SPACE_PNG);
	lv_keyboard_img_set(obj, 37, &info37);

	static rom_bin_info info38 = rom_bin_info_get(ROM_RES_KB_RIGHT_PNG);
	lv_keyboard_img_set(obj, 38, &info38);

	static rom_bin_info info39 = rom_bin_info_get(ROM_RES_KB_APPLY_PNG);
	lv_keyboard_img_set(obj, 39, &info39);
}

static void LAYOUT_ENETER_FUNC(keyboard)
{
	Debug("=======================================+++>\n");
	keyboard_cancel_btn_create();

	keyboard_create();
}

static void LAYOUT_QUIT_FUNC(keyboard)
{
}

CREATE_LAYOUT(keyboard);
