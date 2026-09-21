#include "layout_define.h"
#include "leo_api.h"

typedef enum home_module_list
{
#ifdef PUBLIC_VERSION
	TRANSFER_MODULE,
	MONITOR_MODULE,
	EVENT_MODULE,
	MEDIA_MODULE,
	SETTING_MODULE,
	MODEL_MODULE,
#ifdef HOME_LIGHT_EN
	LIGHT_MODULE,
#else
	GATE2_MODULE,
#endif
	STANDBY_MODULE,
	TOTAL_MODULE

#else

	SETTING_MODULE,
	MONITOR_MODULE,
	EVENT_MODULE,
	MEDIA_MODULE,
#ifdef HOME_MODEL_DEF
	MODEL_MODULE,
#else
	GATE1_MODULE,
#endif
	TRANSFER_MODULE,
	GATE2_MODULE,
	STANDBY_MODULE,
	TOTAL_MODULE
#endif

} home_module_list;

#define HOME_MODULE_COORDINATE_INIT { \
	{157, 100, 130, 130},             \
	{356, 100, 130, 130},             \
	{554, 100, 130, 130},             \
	{752, 100, 130, 130},             \
	{157, 320, 130, 130},             \
	{356, 320, 130, 130},             \
	{554, 320, 130, 130},             \
	{752, 320, 130, 130},             \
};

static bool layout_is_home;

lv_obj_t *home_btn_create_1(Controls_location coordinate, char *string, btn_data *btn_pdata, const void *img_src1, const void *img_src2)
{
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);

	lv_obj_set_pos(btn, coordinate.x, coordinate.y);
	lv_obj_set_size(btn, coordinate.width, coordinate.high);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	if (img_src1 != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, img_src1);
	}
	if (img_src2 != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, img_src2);
	}
	lv_obj_set_style_local_pattern_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

	if (string != NULL)
	{
		if (user_data_get()->language.index == HEBREW)
		{
			lv_obj_set_base_dir(btn, LV_BIDI_DIR_RTL);
		}
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, string);
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
		// lv_obj_set_style_local_value_ofs_x(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,3);
		lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	}
	// lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	// lv_img_set_src(img, img_src1);
	// lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);
	// lv_obj_set_style_local_image_opa(img, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	// btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	return btn;
}

lv_obj_t *home_btn_create_2(Controls_location coordinate, btn_data *btn_pdata, const void *img_src1, const void *img_src2)
{

	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);

	lv_obj_set_pos(btn, coordinate.x, coordinate.y);
	lv_obj_set_size(btn, coordinate.width, coordinate.high);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	if (img_src1 != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, img_src1);
	}
	if (img_src2 != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, img_src2);
	}
	lv_obj_set_style_local_pattern_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	return btn;
}

static void home_transfer_btn_down(lv_obj_t *obj)
{
}

static void home_transfer_btn_up(lv_obj_t *obj)
{
#ifdef FAMILY_TRANSFER_MODULE
	goto_layout(pLAYOUT(family_transfer));
#else
	goto_layout(pLAYOUT(transfer));
#endif
}

// 创建Transfer按钮
static void home_transfer_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(home_transfer_btn_down, home_transfer_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_TRANSFER_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_TRANSFER_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_INTERCOM), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void home_Setting_btn_down(lv_obj_t *obj)
{
}

static void home_Setting_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}

// 创建Setting按钮
static void home_Setting_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(home_Setting_btn_down, home_Setting_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_SETTING_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_SETTING_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_SETTING), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void home_Monitoring_btn_down(lv_obj_t *obj)
{
}

static void home_Monitoring_btn_up(lv_obj_t *obj)
{
	// lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
	goto_layout(pLAYOUT(monitor_1));
}

// 创建Monitoring按钮
static void home_Monitoring_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(home_Monitoring_btn_down, home_Monitoring_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_MONITOR_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_MONITOR_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_MONITOR), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void home_media_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(media));
}

// 创建Media按钮
static void home_media_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, home_media_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_MEDIA_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_MEDIA_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_MEDIA), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void home_event_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(event));
}

// 创建Event按钮
static void home_event_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, home_event_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_EVENT_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_EVENT_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_EVENT), &btn_data, &info, &info1);
	(*coordinate)++;
}

static rom_bin_info *home_model_info_get(int is_focus)
{
	if (user_data_get()->other.model == AT_HOME_PATTERN)
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_MODE_AT_HOME_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_MODE_AT_HOME_UNFOCUS_PNG);
		if (is_focus)
		{
			return &info;
		}
		else
		{
			return &info1;
		}
	}
	else if (user_data_get()->other.model == NOT_AT_HOME_PATTERN)
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_MODE_NOT_AT_HOME_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_MODE_NOT_AT_HOME_UNFOCUS_PNG);
		if (is_focus)
		{
			return &info;
		}
		else
		{
			return &info1;
		}
	}
	else if (user_data_get()->other.model == MUTE_PATTERN)
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_MODE_DORMANT_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_MODE_DORMANT_UNFOCUS_PNG);
		if (is_focus)
		{
			return &info;
		}
		else
		{
			return &info1;
		}
	}
	return NULL;
}
static char *home_model_str_get(void)
{
	if (user_data_get()->other.model == AT_HOME_PATTERN)
	{
		return text_str(STR_AT_HOME);
	}
	else if (user_data_get()->other.model == NOT_AT_HOME_PATTERN)
	{
		return text_str(STR_NOT_AT_HOME);
	}
	else if (user_data_get()->other.model == MUTE_PATTERN)
	{
		return text_str(STR_DORMANT);
	}
	return NULL;
}

void home_Model_btn_switch(void)
{
	static bool manual_set_record1_mode = false;
	static bool manual_set_record2_mode = false;
	static bool manual_set_message1_sw = false;
	static bool manual_set_message2_sw = false;
	static bool audio_ringback = false;
	if (user_data_get()->other.model == MUTE_PATTERN)
	{
		user_data_get()->other.model = AT_HOME_PATTERN;
		user_data_get()->audio.ringback = audio_ringback;
	}
	else if (user_data_get()->other.model == AT_HOME_PATTERN)
	{
		user_data_get()->other.model = NOT_AT_HOME_PATTERN;
		manual_set_message1_sw = user_data_get()->door1.message_sw;
		manual_set_message2_sw = user_data_get()->door2.message_sw;
		manual_set_record1_mode = user_data_get()->door1.record_mode;
		manual_set_record2_mode = user_data_get()->door2.record_mode;

		user_data_get()->door1.record_mode = user_data_get()->door2.record_mode = true;
		user_data_get()->door1.message_sw = user_data_get()->door2.message_sw = true;
	}
	else if (user_data_get()->other.model == NOT_AT_HOME_PATTERN)
	{
		user_data_get()->other.model = MUTE_PATTERN;
		audio_ringback = user_data_get()->audio.ringback;
		user_data_get()->audio.ringback = 0;

		user_data_get()->door1.message_sw = manual_set_message1_sw;
		user_data_get()->door2.message_sw = manual_set_message2_sw;
		user_data_get()->door1.record_mode = manual_set_record1_mode;
		user_data_get()->door2.record_mode = manual_set_record2_mode;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 6);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, home_model_info_get(1));
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, home_model_info_get(0));

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, home_model_str_get());
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, home_model_str_get());
	lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);

	if (user_data_get()->other.model == NOT_AT_HOME_PATTERN && !is_sdcard_insert()) // 只有SD卡插入才会打开留言
	{
		// user_data_get()->door1.message_sw = user_data_get()->door2.message_sw = false;
		// user_data_get()->door1.record_mode = user_data_get()->door2.record_mode = false;
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_PLEASE_INSERT_SD), NULL);
		}
	}
	else
	{
	}
}

#ifdef PUBLIC_VERSION
static void home_Model_btn_up(lv_obj_t *obj)
{
	extern void set_tuya_work_mode(UINT_T mode);
	home_Model_btn_switch();
	set_tuya_work_mode(user_data_get()->other.model);
	// int x = tuya_dp_189_response_work_mode(user_data_get()->other.model);
	// printf("tuya_dp_189_response_work_mode return : %d\n\r", x);
	user_data_save();
}

// 创建Model按钮
static void home_Model_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, home_Model_btn_up, NULL);
	lv_obj_t *btn = home_btn_create_1(**coordinate, home_model_str_get(), &btn_data, home_model_info_get(0), home_model_info_get(1));
	lv_obj_set_id(btn, 6);
	(*coordinate)++;
}
#else
static bool home_Gate1_flag;

static bool layout_is_home;
static lv_task_t *ungate1_task_t = NULL;
static void home_ungate1_task(lv_task_t *task_t)
{

	if (!layout_is_home)
	{
		lv_task_del(ungate1_task_t);
		ungate1_task_t = NULL;
		home_Gate1_flag = 0;
		return;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 6);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LOCK_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_GATE1));
	lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_clear_state(btn, LV_STATE_FOCUSED);

	lv_task_del(ungate1_task_t);
	ungate1_task_t = NULL;
	home_Gate1_flag = 0;
	// 关锁
	tuya_dp_232_response_outdoor_gate1(false);
}

void home_gate1_control(void)
{
	if (home_Gate1_flag == 0)
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 6);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_UNLOCK_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_UNLOCK_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_GATE1));
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_GATE1));
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
		lv_obj_add_state(btn, LV_STATE_FOCUSED);

		// 开锁
		//  if (user_data_get()->other.model != MUTE_PATTERN)
		//  	open_door_ring_play(80);

		network_cmd_data data;
		data.device = DEVICE_OUTDOOR_1;
		data.cmd = NET_COMMON_CMD_UNLOCK;
		data.arg1 = user_data_get()->door1.ungate1_delay;
		data.arg2 = 2 | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
		network_send_cmd_data(&data);

		tuya_dp_232_response_outdoor_gate1(true);
		ungate1_task_t = lv_task_create(home_ungate1_task, user_data_get()->door1.ungate1_delay * 1000, LV_TASK_PRIO_HIGH, NULL);
		home_Gate1_flag = 1;
	}
}
static void home_Gate1_btn_up(lv_obj_t *obj)
{
	home_gate1_control();
}

// 创建Lock按钮
static void home_Gate1_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, home_Gate1_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LOCK_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_LOCK_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(**coordinate, text_str(STR_GATE1), &btn_data, &info, &info1);
	lv_obj_set_id(btn, 6);
	home_Gate1_flag = 0;
	(*coordinate)++;
}
#endif

#ifdef HOME_LIGHT_EN
static bool home_Light_flag;

static lv_task_t *Light_task_t = NULL;
static void home_Light_task(lv_task_t *task_t)
{

	if (!layout_is_home)
	{
		unlock_gpio_set(0);
		lv_task_del(Light_task_t);
		Light_task_t = NULL;
		home_Light_flag = 0;
		return;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 7);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_OFF_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_GATE2));
	lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_clear_state(btn, LV_STATE_FOCUSED);

	lv_task_del(Light_task_t);
	Light_task_t = NULL;
	home_Light_flag = 0;
	// 关锁
	unlock_gpio_set(0);
}

void home_Light_control(void)
{
	if ((home_Light_flag = !home_Light_flag))
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 7);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_ON_UNFOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_LIGHT_ON_FOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);

		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_GATE2));
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_GATE2));
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
		lv_obj_add_state(btn, LV_STATE_FOCUSED);

		// 开灯
		unlock_gpio_set(1);
		// if (user_data_get()->other.model != MUTE_PATTERN)
		// open_door_ring_play(80);
		Light_task_t = lv_task_create(home_Light_task, user_data_get()->other.unlock_time * 1000, LV_TASK_PRIO_HIGH, NULL);
	}
	else
	{
		home_Light_task(NULL);
	}
}
static void home_Light_btn_up(lv_obj_t *obj)
{
	home_Light_control();
}

// 创建Lock按钮
static void home_Light_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, home_Light_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LIGHT_OFF_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_LIGHT_OFF_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(**coordinate, text_str(STR_GATE2), &btn_data, &info, &info1);
	lv_obj_set_id(btn, 7);
	home_Light_flag = 0;
	(*coordinate)++;
}
#else
static bool home_Gate2_flag;

static lv_task_t *ungate2_task_t = NULL;
static void home_ungate2_task(lv_task_t *task_t)
{

	if (!layout_is_home)
	{
		unlock_gpio_set(0);
		lv_task_del(ungate2_task_t);
		ungate2_task_t = NULL;
		home_Gate2_flag = 0;
		return;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 7);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LOCK_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_GATE2));
	lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_clear_state(btn, LV_STATE_FOCUSED);

	lv_obj_set_state(btn, LV_STATE_DEFAULT);
	lv_task_del(ungate2_task_t);
	ungate2_task_t = NULL;
	home_Gate2_flag = 0;
	// 关锁
	tuya_dp_233_response_gate2(false);
	unlock_gpio_set(0);
}

void home_gate2_control(void)
{
	if (home_Gate2_flag == 0)
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 7);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_UNLOCK_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_UNLOCK_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_GATE2));
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_GATE2));
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
		lv_obj_add_state(btn, LV_STATE_FOCUSED);

		// 开锁
		unlock_gpio_set(1);
		// if (user_data_get()->other.model != MUTE_PATTERN)
		// open_door_ring_play(80);

		tuya_dp_233_response_gate2(true);
		ungate2_task_t = lv_task_create(home_ungate2_task, user_data_get()->other.unlock_time * 1000, LV_TASK_PRIO_HIGH, NULL);
		home_Gate2_flag = 1;
	}
}

static void home_Gate2_btn_up(lv_obj_t *obj)
{
	home_gate2_control();
}

// 创建Lock按钮
static void home_Gate2_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, home_Gate2_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_LOCK_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_LOCK_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(**coordinate, text_str(STR_GATE2), &btn_data, &info, &info1);
	lv_obj_set_id(btn, 7);
	home_Gate2_flag = 0;
	(*coordinate)++;
}
#endif

static void home_Standby_btn_down(lv_obj_t *obj)
{
}

static void home_Standby_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(standby));
}

// 创建Standby按钮
static void home_Standby_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(home_Standby_btn_down, home_Standby_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_STANDBY_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_STANDBY_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_STANDBY), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(standby));
}

// 创建back按钮
lv_obj_t *home_back_btn_create(void (*back_btn_up)(lv_obj_t *), void (*back_btn_down)(lv_obj_t *))
{
	static btn_data btn_data;
	btn_data.OPS_DOWN = back_btn_down;
	btn_data.OPS_UP = back_btn_up;
	btn_data.OPS_ANYTHING = NULL;
	btn_data.user_data = NULL;
	btn_data.obj_tone = true;
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_BACK_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_BACK_FOCUS_PNG);
	Controls_location coordinate = {929, 505, 67, 67};
	return home_btn_create_2(coordinate, &btn_data, &info, &info1);
}

static void *wpa_cli_scan_wifi_task(void *arg)
{
	bool a = true;
	wpa_cli_scan_wifi(&a);
	ak_thread_exit();
	return NULL;
}

static void home_gate2_unlock_callback(unsigned long arg1, unsigned long arg2)
{
	home_gate2_control();
}

bool need_load_bg_flag = false;
static void LAYOUT_ENETER_FUNC(home)
{
	Debug("================================\n\r");
	backlight_open(true, false, user_data_get()->other.brightness);
	Controls_location module_coordinate[] = HOME_MODULE_COORDINATE_INIT;
	Controls_location *module_p = &module_coordinate[0];

	monitor_channel_set(MON_CH_NONE);
	home_bg_display();
	layout_is_home = true;
	// user_data_reset();
	// sat_canvas_create();return;
	home_transfer_btn_create(&module_p);
	home_Monitoring_create(&module_p);
	home_event_btn_create(&module_p);
	home_media_btn_create(&module_p);
	home_Setting_btn_create(&module_p);
#ifdef PUBLIC_VERSION
	home_Model_btn_create(&module_p);
#else
	home_Gate1_btn_create(&module_p);
#endif

#ifdef HOME_LIGHT_EN
	home_Light_btn_create(&module_p);
#else
	home_Gate2_btn_create(&module_p);
#endif
	home_Standby_btn_create(&module_p);
	home_back_btn_create(back_btn_up, NULL);
	dev_info_status_event_register(dev_info_status_callback);

	device_gate2_unlock_register(home_gate2_unlock_callback);
	dev_info_status_callback(1, 0);

	/*进入playback 页面处理*/
	// system_bg_data_recovery();//背景颜色处理函数
	// media_thumb_device_close(); //关闭媒体设备

	if (wifi_usb_module_enable())
	{
		ak_pthread_t wpa_cli_scan_wifi_thread = 0;
		ak_thread_create(&wpa_cli_scan_wifi_thread, wpa_cli_scan_wifi_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	}


#if 0
	extern bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h);
	static rom_bin_info img = rom_bin_raw_get();
	static unsigned char *data = NULL;
	if (data == NULL)
	{
		data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_GUI, 1024 * 600 * 4);
		rom_bin_raw_init(img, data, 1024, 600);
		lv_jpg_decode_data("/mnt/nfs/2.jpg", &img, 1024, 600);
	}
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_disp_set_bg_image(lv_disp_get_default(), &img);
#endif
	Debug("================================\n\r");
}

static void LAYOUT_QUIT_FUNC(home)
{
	Debug("======LAYOUT_QUIT_FUNC=====>>\n\n");
	layout_is_home = false;
	dev_info_status_event_register(NULL);
	if (prompt_window != NULL)
	{
		lv_obj_del(prompt_window);
		prompt_window = NULL;
	}

	device_gate2_unlock_register(default_gate2_unlock_callback);
}

CREATE_LAYOUT(home);

static void window_no_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
}

static void ID_repeat_window_create(void (*window_yse_btn_up)(lv_obj_t *), char *str)
{
	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	lv_obj_set_id(window_cont, 888);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_3_PNG);
	lv_obj_t *window_img = lv_img_create(window_cont, NULL);
	lv_obj_set_pos(window_img, 228, 103);
	lv_obj_set_size(window_img, 648, 441);
	lv_img_set_src(window_img, &info);

	lv_obj_t *window_head_label = lv_label_create(window_img, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(window_head_label, str);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_align(window_head_label, window_img, LV_ALIGN_CENTER, 0, -30);

	lv_obj_t *window_ok_btn = lv_btn_create(window_img, NULL);
	lv_obj_set_pos(window_ok_btn, 0, 360);
	lv_obj_set_size(window_ok_btn, 320, 77);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static btn_data btn_data1 = {0};
	btn_data1.OPS_UP = window_yse_btn_up;

	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_YES));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_YES));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	lv_obj_t *window_cancel_btn = lv_btn_create(window_img, window_ok_btn);
	lv_obj_set_x(window_cancel_btn, 324);
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_NO));
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_NO));
	static btn_data btn_data2 = btn_data_create(NULL, window_no_btn_up, NULL);
	window_cancel_btn->user_data = &btn_data2;
	btn_touch_event_listen(window_cancel_btn);
}

static void window_device_id_repeat_yse_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
	goto_layout(pLAYOUT(setting_sys));
}

void device_id_repeat_func(unsigned long arg1, unsigned long arg2)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL) // 若窗口已打开
	{
		if (device_repeat_state_get() == false) // 且已无重复ID
		{
			window_no_btn_up(NULL); // 即退出窗口
		}

		return;
	}
	if ((current_layout_get() == &layout_home || current_layout_get() == &layout_setting) && device_repeat_state_get() == true)
	{
		static char buf[32] = {0};
		sprintf(buf, "%s %d0%d", text_str(STR_DEVICE_CONFLICT), user_data_get()->other.family_id, (int)arg1 & 0x0F);
		ID_repeat_window_create(window_device_id_repeat_yse_btn_up, buf);
	}
}
