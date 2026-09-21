#include "layout_define.h"
#include "leo_api.h"

#define BTN_PRESS_COLOR LV_COLOR_RED

static int network_temp_dev_id;

typedef enum system_module_list
{
	DEVICE_ID_MODULE,
	SYSTEM_TIME_MODULE,
	DATE_FORMAT_MODULE,
	STANDBY_CLOCK_MODULE,
	LANGUAGE_MODULE,
	KEYTONE_MODULE,
	MD_PREVIEW_MODULE,
	RINGBACK_MODULE,
	UNLOCK_DELAY_MODULE,
#ifdef UNLOCK_HINT_AUDIO
	UNLOCK_HINT_MODULE,
#endif
	ADMIN_SET_MODULE,
	TOTAL_MODULE
} system_module_list;

#define SYSTEM_MODULE_COORDINATE_INIT { \
	{199, 23, 700, 52},                 \
	{199, 75, 700, 52},                 \
	{199, 127, 700, 52},                \
	{199, 179, 700, 52},                \
	{199, 231, 700, 52},                \
	{199, 283, 700, 52},                \
	{199, 335, 700, 52},                \
	{199, 387, 700, 52},                \
	{199, 439, 700, 52},                \
	{199, 491, 700, 52},                \
	{199, 543, 700, 52},                \
};

static lv_task_t *sys_time_task_t = NULL;

static void sys_setting_display(void);

static void sys_setting_img_text_display(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_SETTING_UNFOCUS_PNG);
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_value_str(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_SYSTEM_SET));
	lv_obj_set_style_local_value_color(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_align(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
}

static void sys_set_btn_syn_up(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_DEFAULT);
}
static void sys_set_btn_syn_down(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_PRESSED);
}

static void sys_set_btn_syn_event(lv_obj_t *obj, lv_event_t event)
{

	if (LV_EVENT_PRESS_LOST == event)
	{
		sys_set_btn_syn_up(obj);
	}
}

lv_obj_t *sys_setting_btn_create(Controls_location coordinate, char *string1, char *string_lable, btn_data *btn_pdata, btn_data *btn_pdata1, btn_data *btn_pdata2)
{
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);

	lv_obj_set_pos(btn, coordinate.x, coordinate.y);
	lv_obj_set_size(btn, coordinate.width, coordinate.high);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ITEM_FOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
	// lv_obj_set_style_local_pattern_image(btn,LV_OBJ_PART_MAIN,LV_STATE_FOCUSED,&info);
	lv_obj_set_style_local_pattern_align(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_ALIGN_IN_LEFT_MID);

	if (string1 != NULL)
	{
		if (user_data_get()->language.index == HEBREW)
		{
			lv_obj_set_base_dir(btn, LV_BIDI_DIR_RTL);
		}
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string1);
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
		lv_obj_set_style_local_value_ofs_x(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 120);
		lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	}
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_pos(label, coordinate.x + 6, coordinate.y + 6);
	lv_obj_set_size(label, 250, 52);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(label, string_lable);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);

	btn_pdata->user_data = btn;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);

	if (btn_pdata1 != NULL)
	{
		lv_obj_t *imgbtn1 = lv_imgbtn_create(lv_scr_act(), NULL);
		lv_obj_set_pos(imgbtn1, 460, coordinate.y);
		lv_obj_set_size(imgbtn1, 52, 52);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ARROW_LEFT_UNFOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_RELEASED, &info);

		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ARROW_LEFT_FOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_PRESSED, &info1);
		btn_pdata1->user_data = btn;
		imgbtn1->user_data = btn_pdata1;
		lv_imgbtn_set_checkable(imgbtn1, true);
		btn_touch_event_listen(imgbtn1);
	}
	if (btn_pdata2 != NULL)
	{
		lv_obj_t *imgbtn1 = lv_imgbtn_create(lv_scr_act(), NULL);
		lv_obj_set_pos(imgbtn1, 867, coordinate.y);
		lv_obj_set_size(imgbtn1, 52, 52);

		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ARROW_RIGHT_UNFOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_RELEASED, &info);

		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ARROW_RIGHT_FOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_PRESSED, &info1);
		btn_pdata2->user_data = btn;
		imgbtn1->user_data = btn_pdata2;
		// lv_imgbtn_set_checkable(imgbtn1,true);
		btn_touch_event_listen(imgbtn1);
	}
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_pos(img, 472, coordinate.y + 50);
	lv_obj_set_size(img, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img, &info1);
	return btn;
}

static void sys_device_id_set_left_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 1);
	static char str1[32] = {0};
	if (--user_data_get()->other.network_device < 1)
	{
		user_data_get()->other.network_device = 6;
	}

	sprintf(str1, "%d", user_data_get()->other.network_device);

	if (user_data_get()->audio.ringback && user_data_get()->other.network_device != DEVICE_INDOOR_ID1)
	{
		user_data_get()->audio.ringback = 0;
		lv_obj_t *ringback_label = lv_obj_get_child_form_id(lv_scr_act(), 8);
		lv_obj_set_style_local_value_str(ringback_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_OFF));
	}

	if (user_data_get()->other.network_device != 1)
	{
		user_data_get()->door1.motion_sensitivity = 0;
		user_data_get()->door2.motion_sensitivity = 0;
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}
static void sys_device_id_set_right_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 1);
	static char str1[32] = {0};
	if (++user_data_get()->other.network_device > 6)
	{
		user_data_get()->other.network_device = 1;
	}

	if (user_data_get()->other.network_device != 1)
	{
		user_data_get()->door1.motion_sensitivity = 0;
		user_data_get()->door2.motion_sensitivity = 0;
	}

	if (user_data_get()->audio.ringback && user_data_get()->other.network_device != DEVICE_INDOOR_ID1)
	{
		user_data_get()->audio.ringback = 0;
		lv_obj_t *ringback_label = lv_obj_get_child_form_id(lv_scr_act(), 8);
		lv_obj_set_style_local_value_str(ringback_label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_OFF));
	}

	sprintf(str1, "%d", user_data_get()->other.network_device);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void sys_device_id_show_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_device_id_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_device_id_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;

	static char str1[32] = {0};
	sprintf(str1, "%d", user_data_get()->other.network_device);

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_DEVICE_ID), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 1);
	(*coordinate)++;
}

static struct tm setting_time_tm;

static void sys_rtc_time_get(struct tm *date)
{
	/***********************************
	用安凯的api获取时间没有问题
	************************************/
	// ak_get_localdate(date);
	time_t seconds = time(NULL);
	localtime_r(&seconds, date);
	date->tm_year += 1900;
	date->tm_mon += 1;
	// date->tm_mday += 1;
}
static void sys_rtc_time_set(struct tm *date)
{
	Debug("=============>>>\n\n");
	char date_str[64] = {0};
	/***********************************
	用安凯的api有时候无法保存时间,使用
	date保存时间
	格式为:%04d-%02d-%02d %02d:%02d:%02d
	************************************/
	sprintf(date_str, "date -s \"%04d-%02d-%02d %02d:%02d:%02d\"", date->tm_year, date->tm_mon, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
	system(date_str);
	/***********************************
	将系统时间与RTC同步
	************************************/
	system("hwclock -w");
	extern bool standby_timer_reset(void);
	standby_timer_reset();
}

/*
static void setting_time_base_create(lv_obj_t* parent)
{
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_1_PNG);
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_t * img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, 91, 94);
	lv_obj_set_size(img1,44,32);
	lv_img_set_src(img1, &info);

	lv_obj_t * img2 = lv_img_create(parent, img1);
	lv_obj_set_pos(img2, 186, 94);

	lv_obj_t * img3 = lv_img_create(parent, img1);
	lv_obj_set_pos(img3, 280, 94);

	lv_obj_t * img4 = lv_img_create(parent, img1);
	lv_obj_set_pos(img4, 435, 94);

	lv_obj_t * img5 = lv_img_create(parent, img1);
	lv_obj_set_pos(img5, 531, 94);

	lv_obj_t * img6 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img6, 91, 286);
	lv_obj_set_size(img6,44,32);
	lv_img_set_src(img6, &info1);

	lv_obj_t * img7 = lv_img_create(parent, img6);
	lv_obj_set_pos(img7, 186, 286);

	lv_obj_t * img8 = lv_img_create(parent, img6);
	lv_obj_set_pos(img8, 280, 286);

	lv_obj_t * img9 = lv_img_create(parent, img6);
	lv_obj_set_pos(img9, 435, 286);

	lv_obj_t * img10 = lv_img_create(parent, img6);
	lv_obj_set_pos(img10, 531, 286);

}
*/
static void setting_time_roller_event(lv_obj_t *obj, lv_event_t event)
{
	// Debug("event :%d\n\r",event);
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t *parent = lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(), 888), 666);
		if (parent == NULL)
		{
			return;
		}
		// if(event == LV_EVENT_CLICKED)
		lv_obj_t *obj_roller = lv_obj_get_child_form_id(parent, 55);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_time_tm.tm_year = 2021 + id;
			return;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 11);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_time_tm.tm_mon = 1 + id;
			return;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 22);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_time_tm.tm_mday = 1 + id;
			return;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 33);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_time_tm.tm_hour = id;
			return;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 44);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			setting_time_tm.tm_min = id;
			return;
		}
	}
}

static void sys_time_roller_btn_syn_prev(lv_obj_t *obj)
{
	// Debug("=====================>\n\r");
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *rooler = (lv_obj_t *)pdata->user_data;
	if (rooler == NULL)
	{
		return;
	}

	lv_obj_t *parent = lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(), 888), 666);
	if (parent == NULL)
	{
		return;
	}

	char up_char = LV_KEY_UP;

	lv_obj_t *obj_roller = lv_obj_get_child_form_id(parent, 55);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		Debug("=====================>%d\n\r", lv_roller_get_selected(obj_roller));
		setting_time_tm.tm_year = 2021 + lv_roller_get_selected(obj_roller);
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 11);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		Debug("=====================>%d\n\r", lv_roller_get_selected(obj_roller));
		setting_time_tm.tm_mon = lv_roller_get_selected(obj_roller) + 1;
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 22);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		Debug("=====================>%d\n\r", lv_roller_get_selected(obj_roller));
		setting_time_tm.tm_mday = lv_roller_get_selected(obj_roller) + 1;
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 33);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		Debug("=====================>%d\n\r", lv_roller_get_selected(obj_roller));
		setting_time_tm.tm_hour = lv_roller_get_selected(obj_roller);
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 44);
	if (obj_roller == rooler)
	{
		int id = lv_roller_get_selected(obj_roller);
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		if (id > 0)
			setting_time_tm.tm_min = id - 1;
		else
			setting_time_tm.tm_min = 59;
		return;
	}
}

static void sys_time_roller_btn_syn_next(lv_obj_t *obj)
{
	// Debug("=====================>\n\r");
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *rooler = (lv_obj_t *)pdata->user_data;
	if (rooler == NULL)
	{
		return;
	}

	lv_obj_t *parent = lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(), 888), 666);
	if (parent == NULL)
	{
		return;
	}

	char down_char = LV_KEY_DOWN;

	lv_obj_t *obj_roller = lv_obj_get_child_form_id(parent, 55);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);
		Debug("=====================>%d\n\r", lv_roller_get_selected(obj_roller));
		setting_time_tm.tm_year = 2021 + lv_roller_get_selected(obj_roller);
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 11);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);
		Debug("=====================>%d\n\r", lv_roller_get_selected(obj_roller));
		setting_time_tm.tm_mon = lv_roller_get_selected(obj_roller) + 1;
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 22);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);
		Debug("=====================>%d\n\r", lv_roller_get_selected(obj_roller));
		setting_time_tm.tm_mday = lv_roller_get_selected(obj_roller) + 1;
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 33);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);
		Debug("=====================>%d\n\r", lv_roller_get_selected(obj_roller));
		setting_time_tm.tm_hour = lv_roller_get_selected(obj_roller);
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 44);
	if (obj_roller == rooler)
	{
		int id = lv_roller_get_selected(obj_roller);
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);

		if (id < lv_roller_get_option_cnt(obj_roller) - 1)
			setting_time_tm.tm_min = id + 1;
		else
			setting_time_tm.tm_min = 0;
		return;
	}
}

static lv_obj_t *setting_time_roller_create(lv_obj_t *parent, int x, int y, int w, char *opt, btn_data *btn_data2, btn_data *btn_data3)
{
	lv_obj_t *rooler = lv_roller_create(parent, NULL);
	lv_obj_set_style_local_text_line_space(rooler, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 20);
	lv_obj_set_style_local_text_color(rooler, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x80, 0x80, 0x80));

	lv_obj_set_style_local_bg_opa(rooler, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_text_color(rooler, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, lv_color_make(0xF8, 0xCD, 0xA5));
	lv_obj_set_style_local_text_font(rooler, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_roller_set_options(rooler, opt, LV_ROLLER_MODE_INFINITE);
	lv_roller_set_visible_row_count(rooler, 3);
	lv_obj_set_pos(rooler, x + 5, y + 10);
	lv_roller_set_auto_fit(rooler, false);
	lv_obj_set_width(rooler, w + 50);

	static btn_data btn_data1 = btn_data_anything_create(setting_time_roller_event);
	btn_data2->OPS_DOWN = sys_time_roller_btn_syn_prev;
	btn_data3->OPS_DOWN = sys_time_roller_btn_syn_next;

	rooler->user_data = &btn_data1;
	btn_touch_event_listen(rooler);

	lv_obj_t *up_btn = lv_btn_create(parent, NULL);
	lv_obj_set_style_local_bg_opa(up_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(up_btn, x + 30, y - 40);
	lv_obj_set_size(up_btn, w, 52);
	up_btn->user_data = btn_data2;
	btn_data2->user_data = rooler;
	btn_touch_event_listen(up_btn);

	lv_obj_t *down_btn = lv_btn_create(parent, NULL);
	lv_obj_set_style_local_bg_opa(down_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(down_btn, x + 30, y + 150);
	lv_obj_set_size(down_btn, w, 52);
	down_btn->user_data = btn_data3;
	btn_data3->user_data = rooler;
	btn_touch_event_listen(down_btn);
	return rooler;
}
static void setting_time_year_roller_create(lv_obj_t *parent)
{
	char opt[512] = {0};
	for (int i = 2021; i < 2037; i++)
	{
		char buf[8] = {0};
		sprintf(buf, "%d%s", i, i == 2036 ? "" : "\n");
		strcat(opt, buf);
	}

	static btn_data btn_data2 = btn_data_create(NULL, NULL, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	lv_obj_t *obj = setting_time_roller_create(parent, 53, 122, 60, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 55);

	int id = setting_time_tm.tm_year > 2021 ? (setting_time_tm.tm_year - 2021) : 0;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void setting_time_month_roller_create(lv_obj_t *parent)
{
	char opt[64] = {0};
	for (int i = 1; i < 13; i++)
	{
		char buf[8] = {0};
		sprintf(buf, "%d%s", i, i == 12 ? "" : "\n");
		strcat(opt, buf);
	}
	static btn_data btn_data2 = btn_data_create(NULL, NULL, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	lv_obj_t *obj = setting_time_roller_create(parent, 148, 122, 60, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 11);
	int id = setting_time_tm.tm_mon - 1;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void setting_time_day_roller_create(lv_obj_t *parent)
{
	char opt[64] = {0};
	for (int i = 1; i < 32; i++)
	{
		char buf[8] = {0};
		sprintf(buf, "%d%s", i, i == 31 ? "" : "\n");
		strcat(opt, buf);
	}
	static btn_data btn_data2 = btn_data_create(NULL, NULL, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	lv_obj_t *obj = setting_time_roller_create(parent, 243, 122, 60, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 22);

	int id = setting_time_tm.tm_mday - 1;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void setting_time_hour_roller_create(lv_obj_t *parent)
{

	char opt[128] = {0};
	for (int i = 0; i < 24; i++)
	{
		char buf[8] = {0};
		sprintf(buf, "%d%s", i, i == 23 ? "" : "\n");
		strcat(opt, buf);
	}
	static btn_data btn_data2 = btn_data_create(NULL, NULL, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	lv_obj_t *obj = setting_time_roller_create(parent, 397, 122, 60, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 33);

	int id = setting_time_tm.tm_hour;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void setting_time_min_roller_create(lv_obj_t *parent)
{
	char opt[512] = {0};
	for (int i = 0; i < 60; i++)
	{
		char buf[8] = {0};
		sprintf(buf, "%d%s", i, i == 59 ? "" : "\n");
		strcat(opt, buf);
	}
	static btn_data btn_data2 = btn_data_create(NULL, NULL, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	lv_obj_t *obj = setting_time_roller_create(parent, 493, 122, 60, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 44);

	int id = setting_time_tm.tm_min;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void sys_time_task(lv_task_t *task_t)
{
	lv_obj_t *btn1 = lv_obj_get_child_form_id(lv_scr_act(), 2);
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	static char date_str[64] = {0};
	if (user_data_get()->other.date_format == 0)
	{
		sprintf(date_str, "%04d-%02d-%02d  %02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
	}
	else if (user_data_get()->other.date_format == 1)
	{
		sprintf(date_str, "%02d-%02d-%04d  %02d:%02d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	}
	else if (user_data_get()->other.date_format == 2)
	{
		sprintf(date_str, "%02d-%02d-%04d  %02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	}
	lv_obj_set_style_local_value_str(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, date_str);
}

static void sys_time_set_ok_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
	printf("year ===========>>>>:%d\n\r", setting_time_tm.tm_year);
	sys_rtc_time_set(&setting_time_tm);
	sys_time_task(NULL);
	ring_init();
}

static void sys_time_set_cancel_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
}

static void sys_time_setting_window_create(void)
{
	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	lv_obj_set_id(window_cont, 888);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_1_PNG);
	lv_obj_t *window_img = lv_img_create(window_cont, NULL);
	lv_obj_set_pos(window_img, 228, 103);
	lv_obj_set_size(window_img, 648, 441);
	lv_img_set_src(window_img, &info);
	lv_obj_set_id(window_img, 666);

	lv_obj_t *window_head_label = lv_label_create(window_img, NULL);
	lv_obj_set_pos(window_head_label, 300, 20);
	lv_obj_set_size(window_head_label, 48, 40);
	lv_label_set_text(window_head_label, text_str(STR_TIME));
	lv_label_set_align(window_head_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	// setting_time_base_create(window_cont);
	setting_time_year_roller_create(window_img);
	setting_time_month_roller_create(window_img);
	setting_time_day_roller_create(window_img);
	setting_time_hour_roller_create(window_img);
	setting_time_min_roller_create(window_img);

	lv_obj_t *window_ok_btn = lv_btn_create(window_img, NULL);
	lv_obj_set_pos(window_ok_btn, 0, 360);
	lv_obj_set_size(window_ok_btn, 320, 77);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static btn_data btn_data1 = btn_data_create(NULL, sys_time_set_ok_btn_up, NULL);

	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	lv_obj_t *window_cancel_btn = lv_btn_create(window_img, window_ok_btn);
	lv_obj_set_x(window_cancel_btn, 324);

	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CANCEL));
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CANCEL));
	static btn_data btn_data2 = btn_data_create(NULL, sys_time_set_cancel_btn_up, NULL);
	window_cancel_btn->user_data = &btn_data2;
	btn_touch_event_listen(window_cancel_btn);
}

static void sys_time_set_right_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	sys_rtc_time_get(&setting_time_tm);
	sys_time_setting_window_create();
}
static void sys_time_set_btn_up(lv_obj_t *obj)
{
	sys_rtc_time_get(&setting_time_tm);
	sys_time_setting_window_create();
}

static void sys_time_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_time_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, sys_time_set_btn_up, NULL);
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;

	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	static char date_str[64] = {0};
	if (user_data_get()->other.date_format == 0)
	{
		sprintf(date_str, "%04d-%02d-%02d  %02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
	}
	else if (user_data_get()->other.date_format == 1)
	{
		sprintf(date_str, "%02d-%02d-%04d  %02d:%02d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	}
	else if (user_data_get()->other.date_format == 2)
	{
		sprintf(date_str, "%02d-%02d-%04d  %02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	}

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, date_str, text_str(STR_TIME), &btn_data3, NULL, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, 2);
	(*coordinate)++;

	sys_time_task_t = lv_task_create(sys_time_task, 3000, LV_TASK_PRIO_HIGH, NULL);
}

static void sys_Date_format_set_left_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	if (--user_data_get()->other.date_format < 0)
	{
		user_data_get()->other.date_format = 2;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 3);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(user_data_get()->other.date_format + STR_YY_MM_DD));

	lv_obj_t *btn1 = lv_obj_get_child_form_id(lv_scr_act(), 2);
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	static char date_str[64] = {0};
	if (user_data_get()->other.date_format == 0)
	{
		sprintf(date_str, "%04d-%02d-%02d  %02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
	}
	else if (user_data_get()->other.date_format == 1)
	{
		sprintf(date_str, "%02d-%02d-%04d  %02d:%02d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	}
	else if (user_data_get()->other.date_format == 2)
	{
		sprintf(date_str, "%02d-%02d-%04d  %02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	}
	lv_obj_set_style_local_value_str(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, date_str);
}
static void sys_Date_format_set_right_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	if (++user_data_get()->other.date_format > 2)
	{
		user_data_get()->other.date_format = 0;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 3);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(user_data_get()->other.date_format + STR_YY_MM_DD));

	lv_obj_t *btn1 = lv_obj_get_child_form_id(lv_scr_act(), 2);
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	static char date_str[64] = {0};
	if (user_data_get()->other.date_format == 0)
	{
		sprintf(date_str, "%04d-%02d-%02d  %02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
	}
	else if (user_data_get()->other.date_format == 1)
	{
		sprintf(date_str, "%02d-%02d-%04d  %02d:%02d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	}
	else if (user_data_get()->other.date_format == 2)
	{
		sprintf(date_str, "%02d-%02d-%04d  %02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	}
	lv_obj_set_style_local_value_str(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, date_str);
}

static void sys_Date_format_set_btn_create(Controls_location **coordinate)
{

	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_Date_format_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_Date_format_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, text_str(user_data_get()->other.date_format + STR_YY_MM_DD), text_str(STR_DATE_FORMAT), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 3);
	(*coordinate)++;
}

static void sys_Standby_clock_switch_set_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	user_data_get()->other.screen_saver = !user_data_get()->other.screen_saver;
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 4);
	char *str1 = {0};
	if (user_data_get()->other.screen_saver == 1)
		str1 = text_str(STR_ON);
	else
		str1 = text_str(STR_OFF);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void sys_Standby_clock_switch_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_Standby_clock_switch_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_Standby_clock_switch_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;

	char *str1 = {0};
	if (user_data_get()->other.screen_saver == 1)
		str1 = text_str(STR_ON);
	else
		str1 = text_str(STR_OFF);

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_CLOCK), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 4);
	(*coordinate)++;
}

#if 1
static int sys_language_switch(bool next)
{
	int language_total = language_total_get();
	if (next)
	{
		int index = ((user_data_get()->language.index + 1) >= language_total) ? 0 : user_data_get()->language.index + 1;
		for (; index < sizeof(user_data_get()->language.en); index++)
		{
			// Debug("index:%d:en:%d\n", index, user_data_get()->language.en[index]);
			if (user_data_get()->language.en[index])
			{
				user_data_get()->language.index = index;
				return index;
			}
		}
		for (index = 0; index < sizeof(user_data_get()->language.en); index++)
		{
			if (user_data_get()->language.en[index])
			{
				user_data_get()->language.index = index;
				return index;
			}
		}
	}
	else
	{
		int index = ((user_data_get()->language.index) <= 0) ? language_total - 1 : user_data_get()->language.index - 1;
		for (; index >= 0; index--)
		{
			if (user_data_get()->language.en[index])
			{
				user_data_get()->language.index = index;
				return index;
			}
		}
		for (index = language_total - 1; index >= 0; index--)
		{
			if (user_data_get()->language.en[index])
			{
				user_data_get()->language.index = index;
				return index;
			}
		}
	}
	return 0;
}
#endif
static void sys_Language_set_left_btn_up(lv_obj_t *obj)
{

	sys_set_btn_syn_up(obj);
	// #ifdef BCOM_OID_VERSION
	// if(--user_data_get()->language.index < 0){
	// 	user_data_get()->language.index = FRENCH;
	// }
	// #else
	// if(--user_data_get()->language.index < 0){
	// 	user_data_get()->language.index = (language_total_get() - 1);
	// }
	// #endif
	sys_language_switch(false);
	Debug("=======================>>>>《 %d 》\n\n", user_data_get()->language.index);
	tuya_set_current_language(user_data_get()->language.index);
	btn_data *btn_ev = (btn_data *)obj->user_data;
	btn_ev->OPS_ANYTHING = NULL;
	goto_layout(pLAYOUT(setting_sys));
}
static void sys_Language_set_right_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	// #ifdef BCOM_OID_VERSION
	// if(++user_data_get()->language.index > FRENCH){
	// 	user_data_get()->language.index = 0;
	// }
	// #else
	// if(++user_data_get()->language.index > (language_total_get() - 1)){
	// 		user_data_get()->language.index = 0;
	// }
	// #endif

	sys_language_switch(true);
	Debug("=======================>>>>《 %d 》\n\n", user_data_get()->language.index);
	tuya_set_current_language(user_data_get()->language.index);
	btn_data *btn_ev = (btn_data *)obj->user_data;
	btn_ev->OPS_ANYTHING = NULL;
	goto_layout(pLAYOUT(setting_sys));
}

static void sys_Language_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_Language_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_Language_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, text_str(STR_LANGUAGE_TYPE), text_str(STR_LANGUAGE), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 5);
	(*coordinate)++;
}

static void sys_Keytone_set_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	user_data_get()->audio.key_sound = !user_data_get()->audio.key_sound;
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 6);
	char *str1 = {0};
	if (user_data_get()->audio.key_sound == 1)
		str1 = text_str(STR_ON);
	else
		str1 = text_str(STR_OFF);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void sys_Keytone_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_Keytone_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_Keytone_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;
	char *str1 = {0};
	if (user_data_get()->audio.key_sound == 1)
		str1 = text_str(STR_ON);
	else
		str1 = text_str(STR_OFF);

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_KEYTONE), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 6);
	(*coordinate)++;
}

static void sys_MD_preview_set_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	user_data_get()->other.MD_preview = !user_data_get()->other.MD_preview;
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 7);
	char *str1 = {0};
	if (user_data_get()->other.MD_preview == 1)
		str1 = text_str(STR_ON);
	else
		str1 = text_str(STR_OFF);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void sys_MD_preview_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_MD_preview_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_MD_preview_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;
	char *str1 = {0};
	if (user_data_get()->other.MD_preview == 1)
		str1 = text_str(STR_ON);
	else
		str1 = text_str(STR_OFF);

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_DETECTION_PREVIEW), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 7);
	(*coordinate)++;
}

static lv_obj_t *ringback_disable_msg_box = NULL;
static void ringback_disable_msg_btn_up(lv_obj_t *obj)
{
	if (ringback_disable_msg_box != NULL)
	{
		lv_obj_del(ringback_disable_msg_box);
		ringback_disable_msg_box = NULL;
	}
}

static lv_obj_t *ringback_disable_msgbox_create(char *str)
{

	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	// lv_obj_set_id(window_cont,666);

	lv_obj_t *msgbox_cont = lv_cont_create(window_cont, NULL);

	lv_obj_set_pos(msgbox_cont, 350, 187);
	lv_obj_set_size(msgbox_cont, 324, 226);
	lv_obj_set_style_local_bg_color(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00131D));
	lv_obj_set_style_local_bg_opa(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 18);

	lv_obj_t *img = lv_img_create(msgbox_cont, NULL);
	lv_obj_set_pos(img, 52, 42);
	lv_obj_set_size(img, 220, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE2_PNG);
	lv_img_set_src(img, &info1);

	lv_obj_t *window_head_label = lv_label_create(msgbox_cont, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_obj_set_size(window_head_label, 300, 150);
	printf("lv_obj_get_width window_head_label :%d\n", lv_obj_get_width(window_head_label));
	lv_label_set_text(window_head_label, str);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_align(window_head_label, msgbox_cont, LV_ALIGN_CENTER, 0, -30);

	lv_obj_t *window_ok_btn = lv_btn_create(msgbox_cont, NULL);
	lv_obj_set_size(window_ok_btn, 160, 48);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_COMFIRM_PNG);

	static btn_data btn_data1 = {0};
	btn_data1.OPS_UP = ringback_disable_msg_btn_up;
	lv_obj_set_style_local_pattern_image(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
	// lv_obj_set_style_local_pattern_image(window_ok_btn,LV_OBJ_PART_MAIN,LV_STATE_FOCUSED,&info2);
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_align(window_ok_btn, msgbox_cont, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	return window_cont;
}

static void sys_Ringback_set_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	if (user_data_get()->other.network_device != DEVICE_INDOOR_ID1 || user_data_get()->other.model == MUTE_PATTERN)
	{
		if (ringback_disable_msg_box == NULL)
		{
			ringback_disable_msg_box = ringback_disable_msgbox_create(user_data_get()->other.network_device == DEVICE_INDOOR_ID1 ? text_str(STR_SLEEP_CANNOT_SET) : text_str(STR_ONLY_DEVICE_1_SET));
		}
		return;
	}
	user_data_get()->audio.ringback = !user_data_get()->audio.ringback;
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 8);
	char *str1 = {0};
	if (user_data_get()->audio.ringback == 1)
		str1 = text_str(STR_ON);
	else
		str1 = text_str(STR_OFF);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void sys_Ringback_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_Ringback_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_Ringback_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;
	char *str1 = {0};
	user_data_get()->audio.ringback = user_data_get()->other.network_device == DEVICE_INDOOR_ID1 ? user_data_get()->audio.ringback : 0;
	if (user_data_get()->audio.ringback == 1)
		str1 = text_str(STR_ON);
	else
		str1 = text_str(STR_OFF);

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_RINGBACK), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 8);
	(*coordinate)++;
}

static void sys_Indoor_unlock_delay_set_left_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	if (--user_data_get()->other.unlock_time < 1)
	{
		user_data_get()->other.unlock_time = 10;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 9);
	static char str1[32] = {0};
	sprintf(str1, "%d %s", user_data_get()->other.unlock_time, text_str(STR_S));
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}
static void sys_Indoor_unlock_delay_set_right_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	if (++user_data_get()->other.unlock_time > 10)
	{
		user_data_get()->other.unlock_time = 1;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 9);
	static char str1[32] = {0};
	sprintf(str1, "%d %s", user_data_get()->other.unlock_time, text_str(STR_S));
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void sys_Indoor_unlock_delay_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_Indoor_unlock_delay_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_Indoor_unlock_delay_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;
	static char str1[32] = {0};
	sprintf(str1, "%d %s", user_data_get()->other.unlock_time, text_str(STR_S));

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_INDOOR_UNLOCK_TIME), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, 9);
	(*coordinate)++;
}

#ifdef UNLOCK_HINT_AUDIO
static void sys_unlock_hint_set_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	user_data_get()->other.unlock_hint = !user_data_get()->other.unlock_hint;
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 11);
	static char str1[32] = {0};
	sprintf(str1, "%s", text_str(user_data_get()->other.unlock_hint ? STR_ON : STR_OFF));
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void sys_unlock_hint_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_unlock_hint_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(sys_set_btn_syn_down, sys_unlock_hint_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = sys_set_btn_syn_event;
	static char str1[32] = {0};
	sprintf(str1, "%s", text_str(user_data_get()->other.unlock_hint ? STR_ON : STR_OFF));

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_UNLOCK_HINT), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, 11);
	(*coordinate)++;
}
#endif
extern int get_pwd_str;
static void sys_Admin_set_btn_up_1(lv_obj_t *obj)
{
	get_pwd_str = 0;
	goto_layout(pLAYOUT(password_input));
}

static void sys_Admin_set_btn_up(lv_obj_t *obj)
{
	sys_set_btn_syn_up(obj);
	get_pwd_str = 0;
	goto_layout(pLAYOUT(password_input));
}
static void sys_Admin_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data3 = btn_data_create(NULL, sys_Admin_set_btn_up_1, NULL);
	static btn_data btn_data1 = btn_data_create(sys_set_btn_syn_down, sys_Admin_set_btn_up, NULL);
	btn_data1.OPS_ANYTHING = sys_set_btn_syn_event;
	char *str1 = {"******"};
	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_ADMIN_SETTING), &btn_data3, NULL, &btn_data1);
	lv_obj_set_id(btn, 10);
	(*coordinate)++;
}

static void sys_setting_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}

static void sys_setting_display(void)
{
	Controls_location module_coordinate[] = SYSTEM_MODULE_COORDINATE_INIT;
	Controls_location *module_p = &module_coordinate[0];

	sys_setting_img_text_display();
	sys_device_id_show_create(&module_p);
	sys_time_set_btn_create(&module_p);
	sys_Date_format_set_btn_create(&module_p);
	sys_Standby_clock_switch_set_btn_create(&module_p);
	sys_Language_set_btn_create(&module_p);
	sys_Keytone_set_btn_create(&module_p);
	sys_MD_preview_set_btn_create(&module_p);
	sys_Ringback_set_btn_create(&module_p);
	sys_Indoor_unlock_delay_set_btn_create(&module_p);
#ifdef UNLOCK_HINT_AUDIO
	sys_unlock_hint_set_btn_create(&module_p);
#endif
	sys_Admin_set_btn_create(&module_p);
	home_back_btn_create(sys_setting_back_btn_up, NULL);
}

static void LAYOUT_ENETER_FUNC(setting_sys)
{
	Debug("================================\n\r");
	setting_bg_display();
	sys_setting_display();
	network_temp_dev_id = user_data_get()->other.network_device;
}

static void LAYOUT_QUIT_FUNC(setting_sys)
{
	if (ringback_disable_msg_box != NULL)
	{
		lv_obj_del(ringback_disable_msg_box);
		ringback_disable_msg_box = NULL;
	}

	if (sys_time_task_t != NULL) // 计时任务未退出
	{
		lv_task_del(sys_time_task_t); // 退出计时任务
		sys_time_task_t = NULL;		  // 置空指针
	}
	user_data_save();

	if (network_temp_dev_id != user_data_get()->other.network_device)
	{
		network_local_device_set(user_data_get()->other.network_device);
		network_local_mac_set();
	}
}

CREATE_LAYOUT(setting_sys);
