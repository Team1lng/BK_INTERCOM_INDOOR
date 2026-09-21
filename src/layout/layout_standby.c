#include "layout_define.h"
static void motion_detect_func(unsigned long arg1, unsigned long arg2);
static void monitor_video_mode_close(void);
static void motion_2_head_create(void);
static void standby_obj_disable(void);
static void motion_channel_label_display(void);
static void custom_music_play_stop(void);
static void standby_shortcat_disable(void);

bool motion_record_mode_temp = false;
extern bool wifi_control;
static int picture_total;
static int music_total;

static unsigned char *picture_data = NULL;
static int cur_picture_index = 0;
static media_type picture_file_type = FILE_TYPE_SD_PICTURE;
static int cur_music_index = 0;
static media_type music_file_type = FILE_TYPE_SD_MUSIC;

static lv_task_t *digital_picture_ptask = NULL;
static lv_obj_t *back_btn = NULL;
#define MOTION_DELECT_INTERVAL 10000

#define SHORTCUT_KEY_WIDTH 87
#define SHORTCUT_KEY_NUM (SHORTCUT_TOTAL - 1)
#define SHORTCUT_KEY_OFFSET (SHORTCUT_KEY_NUM == 2	 ? 410 \
							 : SHORTCUT_KEY_NUM == 4 ? 258 \
							 : SHORTCUT_KEY_NUM == 5 ? 185 \
							 : SHORTCUT_KEY_NUM == 6 ? 111 \
													 : 134)
#define SHORTCUT_KEY_INTERVAL (1024 - (SHORTCUT_KEY_OFFSET * 2 + SHORTCUT_KEY_NUM * SHORTCUT_KEY_WIDTH)) / (SHORTCUT_KEY_NUM - 1)
#define SHORTCUT_KEY_X(x) SHORTCUT_KEY_OFFSET + ((SHORTCUT_KEY_INTERVAL + SHORTCUT_KEY_WIDTH) * (x - 1))

typedef enum
{
	SHORTCUT_CONT,
#ifdef PUBLIC_VERSION
	MONITOR_MODULE,
	CALL_MODULE,
#ifndef INTERCOM_VERSION
	MESSAGE_MODULE,
	MOTION_MODULE,
#endif
#else
	MONITOR_MODULE,
	// INTERCAM_MODULE,

	CALL_MODULE,
	MESSAGE_MODULE,
	MOTION_MODULE,
#endif

#ifndef INTERCOM_VERSION

#ifndef PUBLIC_VERSION
	OUTDOOR1_LOCK_MODULE,
	OUTDOOR2_LOCK_MODULE,
#endif
#endif
	SHORTCUT_TOTAL,
} standby_shortcut;

typedef enum
{
	WEATHER_MODULE = 8,
	WEATHER_TEMP,
	WEATHER_CONDITION,
	WEATHER_TEMPRANG,
	WEATHER_ICON,
	BLACK_SCREEN,
	TOTAL_MODULE
} standby_module_list;

USER_OBJ_DECLARATION(STORTCAT_OBJ, SHORTCUT_TOTAL);

static rom_bin_info info_monitor = rom_bin_info_get(ROM_RES_MONITOR_MONITOR_UNFOCUS_PNG);
static rom_bin_info info1_monitor = rom_bin_info_get(ROM_RES_MONITOR_MONITOR_FOCUS_PNG);
// static rom_bin_info info_intercam = rom_bin_info_get(ROM_RES_STANDY_TRANSFER_UNFOCUS_PNG);
// static rom_bin_info info1_intercam = rom_bin_info_get(ROM_RES_STANDY_TRANSFER_FOCUS_PNG);

static rom_bin_info info_call = rom_bin_info_get(ROM_RES_STANDY_CALL_UNFOCUS_PNG);
static rom_bin_info info1_call = rom_bin_info_get(ROM_RES_STANDY_CALL_FOCUS_PNG);

#ifndef INTERCOM_VERSION
static rom_bin_info info_msg = rom_bin_info_get(ROM_RES_STANDY_MESSAGE_UNFOCUS_PNG);
static rom_bin_info info1_msg = rom_bin_info_get(ROM_RES_STANDY_MESSAGE_FOCUS_PNG);
static rom_bin_info info_motion = rom_bin_info_get(ROM_RES_STANDY_MOTION_UNFOCUS_PNG);
static rom_bin_info info1_motion = rom_bin_info_get(ROM_RES_STANDY_MOTION_FOCUS_PNG);

#ifndef PUBLIC_VERSION
static rom_bin_info info_lock = rom_bin_info_get(ROM_RES_MONITOR_LOCK_UNFOCUS_PNG);
static rom_bin_info info1_lock = rom_bin_info_get(ROM_RES_MONITOR_LOCK_FOCUS_PNG);
static rom_bin_info info_gate1 = rom_bin_info_get(ROM_RES_STANDY_INDOOR_LOCK_UNFOCUS_PNG);
static rom_bin_info info1_gate1 = rom_bin_info_get(ROM_RES_STANDY_INDOOR_LOCK_FOCUS_PNG);
#endif
#endif

#ifdef MAILBOX_MODULE_EN
static rom_bin_info info_mailbox = rom_bin_info_get(ROM_RES_STANDY_MAILBOX1_PNG);
#endif

extern void carr_record_way_set(media_type way);

extern bool backlight_status_get(void);

static lv_task_t *standby_task_t = NULL;
static void standby_task(lv_task_t *task_t);
static void gui_draw_area_set_2();
extern void send_monitor_talk_cmd(bool talk_en);
extern void send_monitor_hang_cmd(void);
extern void tuya_event_extern_proc(unsigned long arg1, unsigned long arg2);static void standby_click_up(lv_obj_t *obj);

void standby_black_screen(bool black)
{
	lv_obj_t *black_screen_obj = NULL;
	if (black)
	{
		if (lv_obj_get_child_form_id(lv_scr_act(), BLACK_SCREEN) == NULL)
		{
			Debug("!!!!!!!!!!!\n\n");
			black_screen_obj = lv_cont_create(lv_scr_act(), NULL);
			set_location(black_screen_obj, 0, 0, 1024, 600);
			lv_obj_set_id(black_screen_obj, BLACK_SCREEN);
			static btn_data btn_data = btn_data_up_create(standby_click_up);
			black_screen_obj->user_data = &btn_data;
			btn_touch_event_listen(black_screen_obj);
			lv_obj_set_click(black_screen_obj, true);
			lv_obj_set_style_local_bg_opa(black_screen_obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
			lv_obj_set_style_local_bg_color(black_screen_obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0));
		}
		dev_info_status_callback(0, 0);
	}
	else
	{
		if ((black_screen_obj = lv_obj_get_child_form_id(lv_scr_act(), BLACK_SCREEN)))
		{
			Debug("!!!!!!!!!!!\n\n");
			lv_obj_del(black_screen_obj);
		}
	}
	backlight_open(!black, false, user_data_get()->other.brightness);
}

void standby_bg_display(void)
{
	// system_bg_fill_color(0xff002538,0,0,1024,600);
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_disp_set_bg_color(lv_disp_get_default(), lv_color_hex(0xFF1E1E1E));
}
void standby_bg_disable(void)
{
	// system_bg_fill_color(0xff002538,0,0,1024,600);
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
}
void tuya_event_motion_proc(unsigned long arg1, unsigned long arg2)
{
	static bool is_tuya_talking = false;
	tuya_event ev = (tuya_event)arg1;
	Debug("tuya_event_motion_proc ====================+>>>>%d\n\n\n", ev);
	switch (ev)
	{
	/*切换监控*/
	case TUYA_EVENT_MONITOR_SWAP:
	{
		Debug("TUYA_EVENT_MONITOR_SWAP ====================+>>>>\n\n\n");
		extern void tuya_switch_camera(char channel);
		tuya_switch_camera(arg2);
		motion_channel_label_display();
		// monitor_channel_label_display();			  //通道的文本显示
		break;
	}

		/*开锁*/
	case TUYA_EVENT_OPEN_LOCK:
	{
		// Debug("=======>>AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n\n\n");
		MONITOR_CH ch = monitor_channel_get();
		if (ch == MON_CH_DOOR_1)
		{
			tuya_unlock_start();
		}
		else if (ch == MON_CH_DOOR_2)
		{

#ifdef PUBLIC_VERSION
			if (user_data_get()->tuya_info.lock_id == false)
				tuya_unlock_start();
			else
				tuya_ungate1_start();
#else
			tuya_unlock_start();
#endif
		}
		break;
	}

	/* 开锁2*/
	case TUYA_EVENT_OPEN_GATE1:
	{

#if defined(PUBLIC_VERSION)
		if (user_data_get()->tuya_info.lock_id == false)
		{
			tuya_ungate1_start();
		}
		else
		{
			tuya_ungate2_start();
		}
#else
		tuya_ungate1_start();
#endif
		break;
	}
	case TUYA_EVENT_OPEN_GATE2:
	{
		tuya_ungate2_start();
		break;
	}
		/*通话*/
	case TUYA_EVENT_TALK:
		if (arg2 == true)
		{
			MONITOR_CH monitor_ch = monitor_channel_get();
			network_device devcie = monitor_ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
			audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
			audio_talk_open(ctrl);
			send_monitor_talk_cmd(true);
			is_tuya_talking = true;
		}
		// else
		// {
		// 	send_monitor_talk_cmd(false);
		// 	is_talking = false;
		// 	is_tuya_talking = false;
		// }
		break;
		/*进入监控*/
	case TUYA_EVENT_MONITOR_ENTER:
		record_video_stop(true);
		gui_draw_area_set_2();
		if (user_data_get()->other.MD_preview == false)
		{
			if (user_data_get()->other.screen_saver)
				standby_obj_disable();

			motion_2_head_create(); // 画UI
			standby_black_screen(false); //backlight_open(true, false, user_data_get()->other.brightness);
			lv_obj_set_click(lv_scr_act(), false);
		}
		tuya_channel_valid_report();
		monitor_enter_way_set(MONITOR_ENTER_TUYA);
		// send_monitor_talk_cmd(true);
		if (back_btn != NULL)
		{
			lv_obj_set_hidden(back_btn, true);
		}
		fb_background_enable(false);
		break;
		/*退出监控*/
	case TUYA_EVENT_MONITOR_QUIT:
	{

		fb_video_mode_enable(is_fb_video_mode_enable());
		send_monitor_hang_cmd();
		monitor_channel_set(MON_CH_NONE);
		monitor_enter_way_set(MONITOR_ENTER_NONE);
		tuya_event_register(tuya_event_extern_proc); // 涂鸦

		if (is_tuya_talking)
		{
			// send_monitor_talk_cmd(false);
			is_tuya_talking = false;
		}

		goto_layout(pLAYOUT(standby));
	}
	break;
	/* 截屏 */
	case TUYA_EVENT_SCREENSHOT:
	{
		extern int tuya_dp_236_response_screenshot(BOOL_T state);
		extern void screen_capture();
		screen_capture();
		tuya_dp_236_response_screenshot(false);
	}
	break;
	case TUYA_EVENT_MQTT_OFFLINE:
	{
		if (user_data_get()->wifi.wifi_connect_flag)
		{
			system("/app/app/hi3881_reload.sh");
		}
	}
	break;
	default:
		/*Don't do anything*/
		break;
	}
	return;
}

#if 1
#ifdef WEATHER_MOUDLE

typedef enum weather_module_list
{
	HUMIDITY_MODULE,
	PRESSURE_MODULE,
	PM10_MODULE,
	PM2_5_MODULE,
	CONDITION_MODULE,

	TEMP_MODULE,

	HOUR_MODULE,
	SYMBOL_MODULE,
	MINUTE_MODULE,

	DATE_MODULE,
	WEEK_MODULE,

	CONDIT_IMG_MODULE,
	TOTAL_WEATHER
} weather_module_list;

#define WEATHER_COORDINATE_INIT { \
	{12, 60, 184, 80},            \
	{12, 100, 184, 80},           \
	{12, 140, 184, 80},           \
	{12, 180, 184, 80},           \
	{50, 230, 184, 80},           \
	{246, 50, 98, 48},            \
	{124, 290, 130, 100},         \
	{264, 290, 30, 100},          \
	{302, 290, 130, 100},         \
	{205, 450, 138, 30},          \
	{230, 510, 138, 30},          \
	{360, 22, 128, 128},          \
};

char *weather_param_str_get(weather_module_list module)
{
	static char str[128] = {0};

	switch (module)
	{
	case HUMIDITY_MODULE:
		sprintf(str, "%d%%", tuya_weather_get()->humidity);
		break;
	case PRESSURE_MODULE:
		sprintf(str, "%dhPa", tuya_weather_get()->pressure);
		break;
	case PM10_MODULE:
		sprintf(str, "%d", tuya_weather_get()->pm10);
		break;
	case PM2_5_MODULE:
		sprintf(str, "%d", tuya_weather_get()->pm25);
		break;
	case TEMP_MODULE:
		sprintf(str, "%d℃", tuya_weather_get()->temp);
		break;

	default:
		return NULL;
	}
	return str;
}

char *weather_time_str_get(weather_module_list module)
{
	static char str[128] = {0};
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	memset(str, 0, sizeof(str));

	switch (module)
	{
	case HOUR_MODULE:
		sprintf(str, "%02d", tm.tm_hour);
		Debug("%s", str);
		break;
	case MINUTE_MODULE:
		sprintf(str, "%02d", tm.tm_min);
		Debug("%s", str);
		break;
	case DATE_MODULE:
		if (user_data_get()->other.date_format == 0)
		{
			sprintf(str, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
		}
		else if (user_data_get()->other.date_format == 1)
		{
			sprintf(str, "%02d-%02d-%04d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);
		}
		else if (user_data_get()->other.date_format == 2)
		{
			sprintf(str, "%02d-%02d-%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
		}
		break;

	default:
		return NULL;
	}
	return str;
}

enum btn_string_id weather_condition_num_get(void)
{
	switch (tuya_weather_get()->condition)
	{
	case 120:
	case 146:
	case 119:
		return STR_SUNDAY;
	case 101:
	case 107:
	case 108:
	case 111:
	case 112:
	case 113:
	case 118:
	case 122:
	case 123:
	case 125:
	case 134:
	case 136:
	case 137:
	case 139:
	case 141:
	case 143:
	case 144:
	case 145:
		return STR_RAIN;
	case 129:
	case 132:
	case 142:
		return STR_CLOUDY;
	case 104:
	case 105:
	case 124:
	case 126:
	case 127:
	case 128:
	case 130:
	case 131:
	case 138:
		return STR_SNOW;
	case 102:
	case 110:
		return STR_THUNDER_SHOWER;
	case 106:
	case 121:
	case 140:
		return STR_FOG;
	}
	return STR_CLOUDY;
}

void *weather_condition_icon_get(int id)
{
	static rom_bin_info sunday = rom_bin_info_get(ROM_RES_WEATHER_SUN_PNG);
	static rom_bin_info rain = rom_bin_info_get(ROM_RES_WEATHER_RAIN_PNG);
	static rom_bin_info cloudy = rom_bin_info_get(ROM_RES_WEATHER_CLOUD_PNG);
	static rom_bin_info snow = rom_bin_info_get(ROM_RES_WEATHER_SNOW_PNG);
	static rom_bin_info thunder = rom_bin_info_get(ROM_RES_WEATHER_THUNDER_PNG);
	static rom_bin_info fog = rom_bin_info_get(ROM_RES_WEATHER_FOG_PNG);
	switch (id)
	{
	case STR_SUNDAY:
		// Debug("img:%p",&sunday);
		return &sunday;
	case STR_RAIN:
		// Debug("img:%p",&rain);
		return &rain;
	case STR_CLOUDY:
		// Debug("img:%p",&cloudy);
		return &cloudy;
	case STR_SNOW:
		// Debug("img:%p",&snow);
		return &snow;
	case STR_THUNDER_SHOWER:
		// Debug("img:%p",&thunder);
		return &thunder;
	case STR_FOG:
		// Debug("img:%p",&fog);
		return &fog;
	}
	// Debug("img:%p",&cloudy);
	return &cloudy;
}

lv_obj_t *weather_module_create(weather_module_list module, lv_obj_t *parent, char *weather_param, char *param_vol, const lv_font_t *value, void *img)
{
	Controls_location locat[] = WEATHER_COORDINATE_INIT;

	lv_obj_t *obj = NULL;
	if (weather_param)
	{
		lv_obj_t *label = lv_label_create(parent, NULL);
		set_location(label, locat[module].x, locat[module].y, locat[module].width, locat[module].high);
		lv_obj_set_click(label, false);
		lv_obj_set_style_local_bg_opa(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

		char str[128] = {0};
		if (param_vol != NULL)
			sprintf(str, "%s: %s", weather_param, param_vol);
		else
			sprintf(str, "%s", weather_param);
		lv_label_set_text(label, str);
		lv_obj_set_style_local_text_font(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, value);
		lv_obj_set_id(label, module);

		obj = label;
	}
	else if (img)
	{
		lv_obj_t *condition_img = lv_img_create(parent, NULL); // 时针
		set_location(condition_img, locat[module].x, locat[module].y, locat[module].width, locat[module].high);

		// rom_bin_info img = rom_bin_info_get(ROM_RES_WEATHER_CLOUD_PNG);
		// Debug("img:%p",img);
		lv_img_set_src(condition_img, img);
		lv_obj_set_id(condition_img, module);
		obj = condition_img;
	}
	return obj;
}

static void widgets_update_refresh(void *obj, lv_anim_value_t val)
{
	int32_t x = rand() % (LV_HOR_RES_MAX - lv_obj_get_width(obj));
	int32_t y = rand() % (LV_VER_RES_MAX - lv_obj_get_height(obj));
	lv_obj_set_pos(obj, x, y);

	lv_obj_t *humidity = lv_obj_get_child_form_id(obj, HUMIDITY_MODULE);
	if (humidity)
	{
		char str[128] = {0};
		sprintf(str, "%s: %s", text_str(STR_HUMIDITY), weather_param_str_get(HUMIDITY_MODULE));
		lv_label_set_text(humidity, str);
	}

	lv_obj_t *pressure = lv_obj_get_child_form_id(obj, PRESSURE_MODULE);
	if (pressure)
	{
		char str[128] = {0};
		sprintf(str, "%s: %s", text_str(STR_PRESSURE), weather_param_str_get(PRESSURE_MODULE));
		lv_label_set_text(pressure, str);
	}

	lv_obj_t *pm10 = lv_obj_get_child_form_id(obj, PM10_MODULE);
	if (pm10)
	{
		char str[128] = {0};
		sprintf(str, "%s: %s", text_str(STR_PM10), weather_param_str_get(PM10_MODULE));
		lv_label_set_text(pm10, str);
	}

	lv_obj_t *pm25 = lv_obj_get_child_form_id(obj, PM2_5_MODULE);
	if (pm25)
	{
		char str[128] = {0};
		sprintf(str, "%s: %s", text_str(STR_PM25), weather_param_str_get(PM2_5_MODULE));
		lv_label_set_text(pm25, str);
	}

	lv_obj_t *condition = lv_obj_get_child_form_id(obj, CONDITION_MODULE);
	if (condition)
	{
		char str[128] = {0};
		sprintf(str, "%s", text_str(weather_condition_num_get()));
		lv_label_set_text(condition, str);
	}

	lv_obj_t *temp = lv_obj_get_child_form_id(obj, TEMP_MODULE);
	if (temp)
	{
		char str[128] = {0};
		sprintf(str, "%s", weather_param_str_get(TEMP_MODULE));
		lv_label_set_text(temp, str);
	}
}

static void time_symbol_refresh(void *obj, lv_anim_value_t val)
{
	time_t seconds = time(NULL);
	struct tm tm = {0};
	static struct tm prev_tm = {0};
	localtime_r(&seconds, &tm);
	if (tm.tm_min != prev_tm.tm_min)
	{
		if (tm.tm_hour != prev_tm.tm_hour)
		{
			lv_obj_t *hour = lv_obj_get_child_form_id(obj, HOUR_MODULE);
			if (hour)
			{
				char buffer[10]; // 目标缓冲区
				snprintf(buffer, sizeof(buffer), "%02d", tm.tm_hour);
				lv_label_set_text(hour, buffer);
			}
		}

		lv_obj_t *minute = lv_obj_get_child_form_id(obj, MINUTE_MODULE);
		if (minute)
		{
			char buffer[10]; // 目标缓冲区
			snprintf(buffer, sizeof(buffer), "%02d", tm.tm_min);
			lv_label_set_text(minute, buffer);
		}

		prev_tm = tm;
	}

	lv_obj_t *symbol = lv_obj_get_child_form_id(obj, SYMBOL_MODULE);
	if (symbol)
	{
		lv_obj_set_hidden(symbol, !lv_obj_get_hidden(symbol));
	}

	lv_obj_t *date = lv_obj_get_child_form_id(obj, DATE_MODULE);
	if (date)
	{
		char buffer[128]; // 目标缓冲区
		snprintf(buffer, sizeof(buffer), "%s", weather_time_str_get(DATE_MODULE));
		lv_label_set_text(date, buffer);
	}

	lv_obj_t *week = lv_obj_get_child_form_id(obj, WEEK_MODULE);
	if (week)
	{
		char buffer[128]; // 目标缓冲区
		snprintf(buffer, sizeof(buffer), "%s", text_str(STR_WEEK_SUN + get_curr_data_week()));
		lv_label_set_text(week, buffer);
	}
}

static void weather_widgets_refresh(lv_obj_t *obj, uint32_t duration, lv_anim_exec_xcb_t exec_cb)
{
	lv_anim_t anim;
	lv_anim_init(&anim);
	lv_anim_set_time(&anim, duration);
	lv_anim_set_exec_cb(&anim, exec_cb);
	lv_anim_set_var(&anim, obj);
	lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&anim);

	// Debug("start weather animation %p...\n",&anim);
	obj->user_data = &anim;
}
static bool weather_widgets_create = false;
void weather_anim_deleted_cb(struct _lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_DELETE)
	{
		weather_widgets_create = false;

		lv_anim_t *anim = (lv_anim_t *)obj->user_data;
		lv_anim_del(anim, NULL);
		Debug("Delete weather animation %p...\n", anim);
	}
}

void standby_weather_widgets_up(lv_obj_t *obj /* ,lv_event_t event */)
{
	goto_layout(pLAYOUT(home));
}

void standby_weather_widgets_create(void)
{
	if (weather_widgets_create)
		return;

	weather_widgets_create = true;
	lv_obj_t *weather_page = lv_cont_create(lv_scr_act(), NULL);
	set_location(weather_page, 0, 0, 1024, 600);
	lv_cont_set_layout(weather_page, LV_LAYOUT_OFF);
	lv_obj_set_style_local_bg_opa(weather_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(weather_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00090E));
	lv_obj_set_click(weather_page, true);
	lv_obj_set_id(weather_page, WEATHER_MODULE);

	static btn_data btn_data = btn_data_create(NULL, standby_weather_widgets_up, NULL);
	weather_page->user_data = &btn_data;
	btn_touch_event_listen(weather_page);

	lv_obj_t *weather_cont = lv_cont_create(weather_page, NULL);
	lv_obj_set_size(weather_cont, 512, 550);
	lv_obj_set_click(weather_cont, false);
	lv_obj_align(weather_cont, weather_page, LV_ALIGN_IN_LEFT_MID, 0, 0);
	lv_obj_set_style_local_bg_opa(weather_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	weather_module_create(HUMIDITY_MODULE, weather_cont, text_str(STR_HUMIDITY), weather_param_str_get(HUMIDITY_MODULE), FONT_SIZE_L(28), NULL);
	weather_module_create(PRESSURE_MODULE, weather_cont, text_str(STR_PRESSURE), weather_param_str_get(PRESSURE_MODULE), FONT_SIZE_L(28), NULL);
	weather_module_create(PM10_MODULE, weather_cont, text_str(STR_PM10), weather_param_str_get(PM10_MODULE), FONT_SIZE_L(28), NULL);
	weather_module_create(PM2_5_MODULE, weather_cont, text_str(STR_PM25), weather_param_str_get(PM2_5_MODULE), FONT_SIZE_L(28), NULL);
	weather_module_create(CONDITION_MODULE, weather_cont, text_str(weather_condition_num_get()), NULL, FONT_SIZE_L(28), NULL);
	weather_module_create(TEMP_MODULE, weather_cont, weather_param_str_get(TEMP_MODULE), NULL, FONT_SIZE_L(36), NULL);

	weather_module_create(HOUR_MODULE, weather_cont, weather_time_str_get(HOUR_MODULE), NULL, FONT_SIZE_L(120), NULL);
	weather_module_create(SYMBOL_MODULE, weather_cont, ":", NULL, FONT_SIZE_L(120), NULL);
	weather_module_create(MINUTE_MODULE, weather_cont, weather_time_str_get(MINUTE_MODULE), NULL, FONT_SIZE_L(120), NULL);
	weather_module_create(DATE_MODULE, weather_cont, weather_time_str_get(DATE_MODULE), NULL, FONT_SIZE_L(28), NULL);
	weather_module_create(WEEK_MODULE, weather_cont, text_str(STR_WEEK_SUN + get_curr_data_week()), NULL, FONT_SIZE_L(28), NULL);

	weather_module_create(CONDIT_IMG_MODULE, weather_cont, NULL, NULL, NULL, weather_condition_icon_get(weather_condition_num_get()));

	weather_widgets_refresh(weather_cont, 1000000, widgets_update_refresh);

	lv_obj_set_event_cb(weather_cont, weather_anim_deleted_cb);

	weather_widgets_refresh(weather_cont, 100000, time_symbol_refresh);

	// lv_obj_set_event_cb(symbol,weather_anim_deleted_cb);
}
void standby_weather_widgets_hidden(bool hidden)
{
	if (weather_widgets_create == false)
		return;

	lv_obj_t *w_obj = lv_obj_get_child_form_id(lv_scr_act(), WEATHER_MODULE);

	if (w_obj != NULL)
	{
		if (lv_obj_get_hidden(w_obj) != hidden)
		{
			lv_obj_set_hidden(w_obj, hidden);
		}
	}
}
#endif
#endif

static void digital_picture_diplay(media_info info)
{

#if 1
	extern bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h);
	static rom_bin_info img = rom_bin_raw_get();
	if (picture_data == NULL)
	{
		picture_data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_GUI, 1024 * 600 * 4);
		rom_bin_raw_init(img, picture_data, 1024, 600);
	}
	char picture_name[64] = {0};
	bzero(picture_name, sizeof(picture_name));
	sprintf(picture_name, SD_PICTURE_PATH "%s", info.file_name);
	lv_jpg_decode_data(picture_name, &img, 1024, 600);
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_disp_set_bg_image(lv_disp_get_default(), &img);
#endif
}

static void digital_picture_task(lv_task_t *task)
{
	if (cur_picture_index >= picture_total)
	{
		cur_picture_index = 0;
	}
	media_info *info = media_info_get(picture_file_type, cur_picture_index);
	printf("digital_picture_task index :%d       name:%s\n\r", cur_picture_index, info->file_name);
	if (info->file_name == NULL)
	{
		if (digital_picture_ptask != NULL)
		{
			lv_task_del(digital_picture_ptask);
			digital_picture_ptask = NULL;
		}
		return;
	}
	digital_picture_diplay(*info);
	cur_picture_index++;
}
/*
static lv_task_t* standby_clock_ptask = NULL;
static void standby_clock_update_task(lv_task_t* task)
{
	struct tm* pre_clock = NULL;
	if(task != NULL)
	{
		pre_clock  = (struct tm*)task->user_data;
	}

	time_t seconds = time(NULL);
	struct tm cur_clock;
	localtime_r(&seconds,&cur_clock);
	if((task == NULL)||(cur_clock.tm_sec != pre_clock->tm_sec))
	{
		if(pre_clock  != NULL)
		{
			*pre_clock = cur_clock;
		}

		lv_obj_t* parent = lv_obj_get_child_form_id(lv_scr_act(), 0);
		if(parent != NULL)
		{
			lv_obj_t* hour_obj = lv_obj_get_child_form_id(parent, 0);
			if(hour_obj != NULL)
			{
				float hour;
				if (cur_clock.tm_hour > 12)
					hour = (float)(cur_clock.tm_hour - 12);
				else
					hour = (float)cur_clock.tm_hour;

				hour += cur_clock.tm_min / 60.0f;

				int16_t angle = 360.0f / 12.0f * hour*10;
				lv_img_set_angle(hour_obj, (int16_t) angle);
			}

			lv_obj_t* min_obj = lv_obj_get_child_form_id(parent, 1);
			if(min_obj != NULL)
			{
				int16_t angle = 360.0f / 60.0f * cur_clock.tm_min*10;
				lv_img_set_angle(min_obj, (int16_t) angle);
			}

			lv_obj_t* sec_obj = lv_obj_get_child_form_id(parent, 2);
			if(sec_obj != NULL)
			{
				int16_t angle = 360.0f / 60.0f * cur_clock.tm_sec*10;
				lv_img_set_angle(sec_obj, (int16_t) angle);
			}
		}
	}
}
static void standby_analog_create(void)
{

	lv_obj_t * bg_obj = lv_img_create(lv_scr_act(),NULL);
	lv_obj_set_id(bg_obj, 0);

	rom_bin_info img_bg = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_BG_PNG);
	lv_img_set_src(bg_obj, &img_bg);
	lv_obj_set_size(bg_obj, 380, 380);
	lv_obj_set_auto_realign(bg_obj, true);
	lv_obj_align(bg_obj, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_opa(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);
	lv_obj_set_style_local_radius(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,360);
	lv_obj_set_style_local_bg_color(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0x27,0x27,0x27));
	lv_obj_set_style_local_bg_grad_color(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(0x15,0x15,0x15));
	lv_obj_set_style_local_bg_main_stop(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,0);
	lv_obj_set_style_local_bg_grad_stop(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,128);
	lv_obj_set_style_local_bg_grad_dir(bg_obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_GRAD_DIR_VER);



	lv_obj_t* hour_obj = lv_img_create(bg_obj,NULL);  // 时针
	lv_obj_set_id(hour_obj, 0);
	rom_bin_info img_hour = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_HOUR_PNG);
	lv_img_set_src( hour_obj, &img_hour);
	lv_obj_align(  hour_obj, bg_obj,LV_ALIGN_CENTER, 0, -23);
	lv_img_set_pivot(hour_obj,lv_obj_get_width(hour_obj)/2,lv_obj_get_height(hour_obj) - 26);

 // uint16_t h = Hour * 300 + Minute / 12 % 12 * 60;
 // lv_img_set_angle(hour_obj, h);
	lv_obj_t* min_obj = lv_img_create(bg_obj,NULL);
	lv_obj_set_id(min_obj, 1);
	rom_bin_info img_min = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_MIN_PNG);
	lv_img_set_src( min_obj, &img_min);
	lv_obj_align( min_obj, bg_obj,LV_ALIGN_CENTER, 0, -36);
	lv_img_set_pivot(min_obj,lv_obj_get_width(min_obj)/2,lv_obj_get_height(min_obj) - 26);


	lv_obj_t* sec_obj  = lv_img_create(bg_obj,NULL);  //秒针
	lv_obj_set_id(sec_obj, 2);
	rom_bin_info img_sec = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_SEC_PNG);
	lv_img_set_src( sec_obj, &img_sec);
	lv_obj_align(  sec_obj, bg_obj,LV_ALIGN_CENTER, 0, -50);
	lv_img_set_pivot(sec_obj,lv_obj_get_width(sec_obj)/2,lv_obj_get_height(sec_obj) - 26);


	lv_obj_t* dot_obj = lv_img_create(bg_obj,NULL);
	rom_bin_info img_dot = rom_bin_info_get(ROM_RES_STANDBY_CLOCK_DOT_PNG);
	lv_img_set_src(dot_obj, &img_dot);
	lv_obj_align(dot_obj, bg_obj, LV_ALIGN_CENTER, 0, 0);

	static struct tm clock;
	time_t seconds = time(NULL);
	localtime_r(&seconds,&clock);
	standby_clock_update_task(NULL);
	standby_clock_ptask = lv_task_create(standby_clock_update_task, 100, LV_TASK_PRIO_MID, &clock);  // 1秒任务
}
*/
static void monitor_indoor_cmd_func(unsigned long arg1, unsigned long arg2)
{
	char cmd = (arg1 >> 8) & 0xFF;
	network_device out_device = arg2 & 0xFF;

	MONITOR_CH ch = monitor_channel_get();
	MONITOR_CH out_ch = out_device == DEVICE_OUTDOOR_1 ? MON_CH_DOOR_1 : MON_CH_DOOR_2;

	printf("======net_common_outdoor_hand_func= %d ,%d======>>>\n\n", cmd, out_ch);
	fflush(stdout);
	switch (cmd)
	{
	case 1: /* in device 与 out_device 通话 */
		if (((out_ch != ch)) || monitor_enter_way_get() == MONITOR_ENTER_TUYA)
		{
			/*不做任何处理*/
		}
		else
		{
			monitor_channel_set(MON_CH_NONE);
			goto_layout(pLAYOUT(standby));
		}
		break;
	default:
		printf("Parameter error :%d\n\r", cmd);
		break;
	}
}

bool get_outdoor_talk_state(MONITOR_CH ch);
static void standby_click_up(lv_obj_t *obj)
{

	Debug("==================>>>>:%d  %d\n\n\n", tuya_monitor_state_get(), get_outdoor_talk_state(MON_CH_DOOR_1));
	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2) || tuya_monitor_state_get())
	{
		return;
	}

	if (backlight_status_get() == false && user_data_get()->other.screen_saver)
	{
		goto_layout(pLAYOUT(standby));
		// backlight_open(true,false,user_data_get()->other.brightness);
		// if(standby_task_t == NULL)
		// standby_task_t = lv_task_create(standby_task, 60000, LV_TASK_PRIO_HIGH, NULL);
	}
	else /*  if(backlight_status_get()) */
	{
		goto_layout(pLAYOUT(home));
	}
}

static lv_task_t *standby_timer_ptask = NULL;
static lv_obj_t *time_obj_group[4] = {NULL};
static bool MAOHAO_HIDE = true;
typedef enum time_module_list
{
	HOUR_FOUMAT,
	PUNCTUATION,
	MIN_FOUMAT,
	DATE_FOUMAT,
	TIME_TOUCH,
} time_module_list;

#define DATA_TIME_COORDINATE_INIT { \
	{14, 0, 50, 36},                \
	{60, 0, 10, 36},                \
	{74, 0, 50, 36},                \
	{4, 46, 122, 28},               \
	{447, 204, 130, 74},            \
};

static void standby_time_display(struct tm *time)
{
	// rom_bin_info time_res_group[10] = {rom_bin_info_get(ROM_RES_STANDY_TIME_0_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_1_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_2_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_3_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_4_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_5_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_6_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_7_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_8_PNG), rom_bin_info_get(ROM_RES_STANDY_TIME_9_PNG)};

	char time_str[8] = {0};
	memset(time_str, 0, sizeof(time_str));
	sprintf(time_str, "%02d", time->tm_hour);
	lv_label_set_text(time_obj_group[HOUR_FOUMAT], time_str);

	memset(time_str, 0, sizeof(time_str));
	sprintf(time_str, "%02d", time->tm_min);
	lv_label_set_text(time_obj_group[MIN_FOUMAT], time_str);

	lv_obj_set_style_local_text_font(time_obj_group[HOUR_FOUMAT], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE_L(42));
	lv_obj_set_style_local_text_font(time_obj_group[MIN_FOUMAT], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE_L(42));
}

#include "tuya_ipc_stream_storage.h"
static void syandby_punctuation_display(void)
{
	// rom_bin_info btn_info = rom_bin_info_get(ROM_RES_STANDY_TIME_MAOHAO_PNG);
	if ((MAOHAO_HIDE = !MAOHAO_HIDE))
	{
		lv_obj_set_hidden(time_obj_group[PUNCTUATION], false);
		lv_label_set_text(time_obj_group[PUNCTUATION], ":");
	}
	else
	{
		lv_obj_set_hidden(time_obj_group[PUNCTUATION], true);
		lv_label_set_text(time_obj_group[PUNCTUATION], ":");
	}
	lv_obj_set_style_local_text_font(time_obj_group[PUNCTUATION], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE_L(42));
}

static void standby_date_display(struct tm *tm)
{
	char date_str[100] = {0};

	if (user_data_get()->other.date_format == 0)
	{
		sprintf(date_str, "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
	}
	else if (user_data_get()->other.date_format == 1)
	{
		sprintf(date_str, "%02d-%02d-%04d", tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900);
	}
	else if (user_data_get()->other.date_format == 2)
	{
		sprintf(date_str, "%02d-%02d-%04d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
	}

	lv_label_set_text(time_obj_group[DATE_FOUMAT], date_str);
}

static void standby_time_display_task(struct _lv_task_t *task_t)
{
	// if(backlight_status_get() ==  false)
	// {
	// 	return;
	// }

	time_t seconds = time(NULL);
	struct tm tm = {0};
	static struct tm prev_tm = {0};
	localtime_r(&seconds, &tm);
	if (prev_tm.tm_mday != tm.tm_mday || task_t == NULL)
	{
		standby_date_display(&tm);
	}
	if (prev_tm.tm_min != tm.tm_min || task_t == NULL)
	{
		standby_time_display(&tm);
	}
	syandby_punctuation_display();
	prev_tm = tm;
}

static void standby_time_touch_up(lv_obj_t *obj)
{
	Debug("==================>>>>:%d\n\n\n", backlight_status_get());
	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2) || tuya_monitor_state_get())
	{
		return;
	}
	extern bool backlight_status_get(void);
	if (backlight_status_get() == false /*  && user_data_get()->other.screen_saver */)
	{
		goto_layout(pLAYOUT(standby));
		// backlight_open(true,false,user_data_get()->other.brightness);
		// if(standby_task_t == NULL)
		// standby_task_t = lv_task_create(standby_task, 60000, LV_TASK_PRIO_HIGH, NULL);
	}
	else /*  if(backlight_status_get()) */
	{
		goto_layout(pLAYOUT(home));
	}
}

static void standby_time_display_disable(void)
{

	for (int i = 0; i < 4; i++)
	{
		lv_obj_set_hidden(time_obj_group[i], true);
	}
	if (standby_timer_ptask != NULL)
	{
		lv_task_del(standby_timer_ptask);
	}
	standby_timer_ptask = NULL;

	dev_info_status_callback(0, 0);
}

static void standby_time_display_create(void)
{
	if (standby_timer_ptask != NULL)
	{
		lv_task_del(standby_timer_ptask);
		standby_timer_ptask = NULL;
	}

	MAOHAO_HIDE = true;

	Controls_location *module_coordinate = NULL;

	Controls_location coordinate[] = DATA_TIME_COORDINATE_INIT;

	{
		module_coordinate = coordinate;
	}

	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_pos(cont, module_coordinate[TIME_TOUCH].x, module_coordinate[TIME_TOUCH].y);
	lv_obj_set_size(cont, module_coordinate[TIME_TOUCH].width, module_coordinate[TIME_TOUCH].high);
	static btn_data btn_data = btn_data_up_create(standby_time_touch_up);
	cont->user_data = &btn_data;
	btn_touch_event_listen(cont);


	time_obj_group[PUNCTUATION] = lv_label_create(cont,NULL);
	lv_obj_set_x(time_obj_group[PUNCTUATION], module_coordinate[PUNCTUATION].x);
	lv_obj_set_width(time_obj_group[PUNCTUATION], module_coordinate[PUNCTUATION].width);

	time_obj_group[HOUR_FOUMAT] = lv_label_create(cont,  time_obj_group[PUNCTUATION]);
	lv_obj_set_pos(time_obj_group[HOUR_FOUMAT], module_coordinate[HOUR_FOUMAT].x, module_coordinate[HOUR_FOUMAT].y);
	lv_obj_set_size(time_obj_group[HOUR_FOUMAT], module_coordinate[HOUR_FOUMAT].width, module_coordinate[HOUR_FOUMAT].high);

	time_obj_group[MIN_FOUMAT] = lv_label_create(cont, time_obj_group[HOUR_FOUMAT]);
	lv_obj_set_x(time_obj_group[MIN_FOUMAT], module_coordinate[MIN_FOUMAT].x);

	time_obj_group[DATE_FOUMAT] = lv_label_create(cont, NULL);
	lv_label_set_long_mode(time_obj_group[DATE_FOUMAT], LV_LABEL_LONG_CROP);
	lv_obj_set_pos(time_obj_group[DATE_FOUMAT], module_coordinate[DATE_FOUMAT].x, module_coordinate[DATE_FOUMAT].y);
	lv_obj_set_size(time_obj_group[DATE_FOUMAT], module_coordinate[DATE_FOUMAT].width, module_coordinate[DATE_FOUMAT].high);
	lv_label_set_align(time_obj_group[DATE_FOUMAT], LV_LABEL_ALIGN_CENTER);

	standby_timer_ptask = lv_task_create(standby_time_display_task, 1000, LV_TASK_PRIO_LOWEST, standby_timer_ptask);
	lv_task_ready(standby_timer_ptask);
	standby_time_display_task(NULL);
}

/*

static void standy_call_records_btn_down(lv_obj_t *obj)
{
	//home_btn_state_set(obj, LV_STATE_PRESSED);
	btn_data *pdata = (btn_data *) obj->user_data;
	lv_obj_t * img = (lv_obj_t *) pdata->user_data;
	rom_bin_info info = rom_bin_info_get(ROM_RES_STANDY_CALL_FOCUS_PNG);
	lv_img_set_src(img, &info);
	lv_obj_align(img, obj, LV_ALIGN_CENTER, 0, 0);

	lv_obj_t * label = lv_obj_get_child_form_id(lv_scr_act(),1);
	char str[3] = {0};
	sprintf(str, "%d",user_data_get()->other.senser_records);
	lv_label_set_text(label, str);

}


static void standy_call_records_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(call_record));
}


//创建按钮
static void standy_call_records_btn_create(void)
{
	static btn_data btn_data = btn_data_create(standy_call_records_btn_down, standy_call_records_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_STANDY_CALL_UNFOCUS_PNG);
	lv_obj_t * obj = standy_btn_create(258, 500, SHORTCUT_KEY_WIDTH,SHORTCUT_KEY_WIDTH, NULL, &btn_data, &info, false);

	lv_obj_t * img = (lv_obj_t *) btn_data.user_data;
	lv_obj_align(img, obj, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_auto_realign(img, true);

	rom_bin_info info1 = rom_bin_info_get(ROM_RES_STANDY_TIP_PNG);
	lv_obj_t * img1 = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img1, &info1);
	lv_obj_set_pos(img1, 315, 494);
	lv_obj_set_size(img1, 40, 28);

	char str[3] = {0};
	sprintf(str, "%d",user_data_get()->other.call_records);
	lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(label, str);
	lv_obj_align(label, img1, LV_ALIGN_CENTER, 0, 0);

	lv_obj_set_id(label, 1);
	lv_obj_set_auto_realign(label, true);
}


*/

#ifndef PUBLIC_VERSION
static char lock_2_task_lock_flag = 0;
static lv_task_t *unlock_2_task_t = NULL;
static void standby_Lock_2_task(lv_task_t *task_t)
{
	printf("%s =================>>%d\n\r", __func__, __LINE__);
	lv_obj_t *btn = ((lv_obj_t *)(task_t->user_data));
	// lv_obj_t* btn = lv_obj_get_child_form_id(obj,OUTDOOR2_LOCK_MODULE);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_STANDY_INDOOR_LOCK_UNFOCUS_PNG);

	lv_imgbtn_set_src(btn, LV_BTN_STATE_RELEASED, &info);
	lv_imgbtn_set_src(btn, LV_BTN_STATE_PRESSED, &info);

	tuya_dp_148_response_accessory_lock(false);

	lock_2_task_lock_flag = 0;
	if (unlock_2_task_t)
	{
		lv_task_del(unlock_2_task_t);
		unlock_2_task_t = NULL;
	}
}

static char lock_1_task_lock_flag = 0;
static lv_task_t *unlock_1_task_t = NULL;
static void standby_Lock_1_task(lv_task_t *task_t)
{
	printf("%s =================>>%d\n\r", __func__, __LINE__);
	lv_obj_t *btn = ((lv_obj_t *)(task_t->user_data));
	// lv_obj_t* btn = lv_obj_get_child_form_id(obj,OUTDOOR1_LOCK_MODULE);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LOCK_UNFOCUS_PNG);

	lv_imgbtn_set_src(btn, LV_BTN_STATE_RELEASED, &info);
	lv_imgbtn_set_src(btn, LV_BTN_STATE_PRESSED, &info);

	tuya_dp_148_response_accessory_lock(false);

	lock_1_task_lock_flag = 0;
	if (unlock_1_task_t)
	{
		lv_task_del(unlock_1_task_t);
		unlock_1_task_t = NULL;
	}
}
#endif

static void standy_menu_btn_up(lv_obj_t *obj)
{
	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2) || tuya_monitor_state_get()) // 正在视频对讲其他机子无法操作
	{
		return;
	}

	standby_black_screen(false); //backlight_open(true, false, user_data_get()->other.brightness);

	switch (obj->obj_id)
	{
	case MONITOR_MODULE:
		system_bg_data_backup();			// 背景颜色恢复
		monitor_channel_set(MON_CH_DOOR_1); // 通道选择 手动进入就是DOOR1

		monitor_enter_way_set(MONITOR_ENTER_MANUAL);
		goto_layout(pLAYOUT(monitor)); // 页面跳转
		audio_talk_ctrl ctrl = {{DEVICE_OUTDOOR_1}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
		audio_talk_open(ctrl);
		break;
	// case INTERCAM_MODULE:
	// 	goto_layout(pLAYOUT(transfer));
	// 	break;
	case CALL_MODULE:
		carr_record_way_set(FILE_TYPE_SD_CALL);
		goto_layout(pLAYOUT(file_list));
		break;

#ifndef INTERCOM_VERSION
	case MESSAGE_MODULE:
		carr_record_way_set(FILE_TYPE_SD_MSG);
		goto_layout(pLAYOUT(file_list));
		break;
	case MOTION_MODULE:
		carr_record_way_set(FILE_TYPE_SD_MOTION);
		goto_layout(pLAYOUT(file_list));
		break;

#ifndef PUBLIC_VERSION
	case OUTDOOR1_LOCK_MODULE:
		if (lock_1_task_lock_flag == 0)
		{
			// monitor_ring_timeout_val = 0;
			audio_play_stop_set();

			static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK_UNFOCUS_PNG);
			lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, &info);
			lv_imgbtn_set_src(obj, LV_BTN_STATE_PRESSED, &info);
			// 开锁
			network_cmd_data data;
			data.device = DEVICE_OUTDOOR_1;
			data.cmd = NET_COMMON_CMD_UNLOCK;
			data.arg1 = user_data_get()->door1.unlock_delay;
			data.arg2 = 1 | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
			network_send_cmd_data(&data);
			tuya_dp_148_response_accessory_lock(true);

			if (unlock_1_task_t == NULL)
			{
				unlock_1_task_t = lv_task_create(standby_Lock_1_task, user_data_get()->door1.unlock_delay * 1000, LV_TASK_PRIO_HIGH, obj);
				lock_1_task_lock_flag = 1;
			}

			// if (user_data_get()->other.model != MUTE_PATTERN)
			// 	open_door_ring_play(80);
		}
		break;
	case OUTDOOR2_LOCK_MODULE:
		if (lock_2_task_lock_flag == 0)
		{
			// monitor_ring_timeout_val = 0;
			audio_play_stop_set();

			static rom_bin_info info = rom_bin_info_get(ROM_RES_STANDY_INDOOR_UNLOCK_UNFOCUS_PNG);
			lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, &info);
			lv_imgbtn_set_src(obj, LV_BTN_STATE_PRESSED, &info);
			// 开锁
			network_cmd_data data;
			data.device = DEVICE_OUTDOOR_2;
			data.cmd = NET_COMMON_CMD_UNLOCK;
			data.arg1 = user_data_get()->door2.unlock_delay;
			data.arg2 = 1 | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
			network_send_cmd_data(&data);
			tuya_dp_148_response_accessory_lock(true);
			if (unlock_2_task_t == NULL)
			{
				unlock_2_task_t = lv_task_create(standby_Lock_2_task, user_data_get()->door2.unlock_delay * 1000, LV_TASK_PRIO_HIGH, obj);
				lock_2_task_lock_flag = 1;
			}

			// if (user_data_get()->other.model != MUTE_PATTERN)
			// 	open_door_ring_play(80);
		}
		break;
#endif
#endif

	default:
		break;
	}
}

static void standby_shorcut_refresh(void)
{
	if (USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT).obj == NULL)
		return;

	lv_obj_t *parent = USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT).obj;
	lv_obj_t *child = NULL;
	standby_shortcut index;
	for (index = 0; index < SHORTCUT_TOTAL; index++)
	{
		switch (index)
		{
		case CALL_MODULE:
			if ((child = lv_obj_get_child_form_id(parent, CALL_MODULE * SHORTCUT_TOTAL)) != NULL)
			{
				char str[5] = {0};
				int total = media_file_total_get(FILE_TYPE_SD_CALL, 1);
				if (total < 99)
				{
					sprintf(str, "%d", total);
				}
				else
				{
					sprintf(str, "%s", "99+");
				}
				lv_label_set_text(child, str);
				lv_obj_set_auto_realign(child, true);
			}
			break;

#ifndef INTERCOM_VERSION
		case MESSAGE_MODULE:

			if ((child = lv_obj_get_child_form_id(parent, MESSAGE_MODULE * SHORTCUT_TOTAL)) != NULL)
			{
				char str[5] = {0};
				int total = media_file_total_get(FILE_TYPE_SD_MSG, 1);
				if (total < 99)
				{
					sprintf(str, "%d", total);
				}
				else
				{
					sprintf(str, "%s", "99+");
				}
				lv_label_set_text(child, str);
				lv_obj_set_auto_realign(child, true);
			}
			break;

		case MOTION_MODULE:

			if ((child = lv_obj_get_child_form_id(parent, MOTION_MODULE * SHORTCUT_TOTAL)) != NULL)
			{
				char str[5] = {0};
				int total = media_file_total_get(FILE_TYPE_SD_MOTION, 1);
				if (total < 99)
				{
					sprintf(str, "%d", total);
				}
				else
				{
					sprintf(str, "%s", "99+");
				}
				lv_label_set_text(child, str);
				lv_obj_set_auto_realign(child, true);
			}
			break;

#endif
		default:
			break;
		}
	}
}

static lv_obj_t *standy_shortcut_btn_create(user_obj *obj, media_type file_type, rom_bin_info *src_info, rom_bin_info *src_info1)
{
	static btn_data btn_data = btn_data_create(NULL, standy_menu_btn_up, NULL);

	int obj_x = SHORTCUT_KEY_X(obj->id);
	// Debug("SHORTCUT_KEY_OFFSET:%d   SHORTCUT_KEY_INTERVAL:%d SHORTCUT_KEY_X:%d\n",SHORTCUT_KEY_OFFSET,SHORTCUT_KEY_INTERVAL,obj_x);
	// Debug("parent obj:%p\n",obj->parent);

	lv_obj_t *imgbtn1 = lv_imgbtn_create(obj->parent->obj, NULL);
	lv_obj_set_pos(imgbtn1, obj_x, obj->area.y);
	lv_obj_set_size(imgbtn1, obj->area.width, obj->area.high);

	lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_RELEASED, src_info);
	lv_obj_set_id(imgbtn1, obj->id);

	lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_PRESSED, src_info1);
	imgbtn1->user_data = &btn_data;
	lv_imgbtn_set_checkable(imgbtn1, true);
	btn_touch_event_listen(imgbtn1);

	if (FILE_TYPE_NONE != file_type)
	{
		rom_bin_info info2 = rom_bin_info_get(ROM_RES_STANDY_TIP_PNG);
		lv_obj_t *img1 = lv_img_create(obj->parent->obj, NULL);
		lv_img_set_src(img1, &info2);
		lv_obj_set_pos(img1, obj_x + 60, 10);
		lv_obj_set_size(img1, 40, 28);

		char str[5] = {0};
		int total = media_file_total_get(file_type, 1);
		if (total < 99)
		{
			sprintf(str, "%d", total);
		}
		else
		{
			sprintf(str, "%s", "99+");
		}
		// Debug("%p,%p\n",obj->parent->obj,USER_OBJ_GET(STORTCAT_OBJ,SHORTCUT_CONT).obj);
		lv_obj_t *label = lv_label_create(obj->parent->obj, NULL);
		lv_obj_set_id(label, obj->id * SHORTCUT_TOTAL);
		lv_label_set_text(label, str);
		lv_obj_align(label, img1, LV_ALIGN_CENTER, 0, 0);
		btn_data.user_data = label;

		lv_obj_set_auto_realign(label, true);
	}
	else
	{
	}
	return imgbtn1;
}

static lv_obj_t *SHORTCUT_CONT_CREATE(user_obj *obj)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(cont, obj->id);
	set_location(cont, obj->area.x, obj->area.y, obj->area.width, obj->area.high);

	return cont;
}

static lv_obj_t *MONITOR_MODULE_CREATE(user_obj *obj)
{
	return standy_shortcut_btn_create(obj, FILE_TYPE_NONE, &info_monitor, &info1_monitor);
}

static lv_obj_t *CALL_MODULE_CREATE(user_obj *obj)
{
	return standy_shortcut_btn_create(obj, FILE_TYPE_SD_CALL, &info_call, &info1_call);
}

#ifndef INTERCOM_VERSION
static lv_obj_t *MESSAGE_MODULE_CREATE(user_obj *obj)
{
	return standy_shortcut_btn_create(obj, FILE_TYPE_SD_MSG, &info_msg, &info1_msg);
}

static lv_obj_t *MOTION_MODULE_CREATE(user_obj *obj)
{
	return standy_shortcut_btn_create(obj, FILE_TYPE_SD_MOTION, &info_motion, &info1_motion);
}

#ifndef PUBLIC_VERSION
static lv_obj_t *OUTDOOR1_LOCK_MODULE_CREATE(user_obj *obj)
{
	return standy_shortcut_btn_create(obj, FILE_TYPE_NONE, &info_lock, &info1_lock);
}

static lv_obj_t *OUTDOOR2_LOCK_MODULE_CREATE(user_obj *obj)
{
	return standy_shortcut_btn_create(obj, FILE_TYPE_NONE, &info_gate1, &info1_gate1);
}
#endif
#endif

#ifdef MAILBOX_MODULE_EN
void mailbox_status_callback(unsigned long arg1, unsigned long arg2)
{

	lv_obj_t *mailbox = NULL;
	if ((mailbox = lv_obj_get_child_form_id(lv_scr_act(), 101)) != NULL)
	{
		if (user_data_get()->door1.mailbox_num == 0)
		{
			lv_obj_set_hidden(mailbox, true);
			return;
		}
		else
		{
			lv_obj_set_hidden(mailbox, false);
		}

		// lv_obj_t *mailbox_label = (lv_obj_t *)mailbox->user_data;
		// if(mailbox_label)
		// {
		// 	char str[5] = {0};
		// 	if(user_data_get()->door1.mailbox_num < 99)
		// 	{
		// 		sprintf(str, "%d",user_data_get()->door1.mailbox_num);
		// 	}
		// 	else
		// 	{
		// 		sprintf(str, "%s","99");
		// 	}

		// 	lv_label_set_text(mailbox_label, str);
		// }
	}
}
static void mailbox_status_disable(void)
{
	lv_obj_t *mailbox = NULL;
	if ((mailbox = lv_obj_get_child_form_id(lv_scr_act(), 101)) != NULL)
	{
		{
			mailbox_status_event_register(NULL);
			lv_obj_set_hidden(mailbox, true);
		}
	}
}
static void mailbox_status_create(rom_bin_info *src_info)
{
	lv_obj_t *mailbox_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_pos(mailbox_cont, 40, 14);
	lv_obj_set_size(mailbox_cont, 55, 35);
	lv_obj_set_style_local_bg_opa(mailbox_cont, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
	lv_obj_set_id(mailbox_cont, 101);

	lv_obj_t *imgbtn1 = lv_img_create(mailbox_cont, NULL);
	lv_img_set_src(imgbtn1, src_info);
	lv_obj_set_pos(imgbtn1, 0, 7);
	lv_obj_set_size(imgbtn1, 40, 28);

	{
		mailbox_status_event_register(mailbox_status_callback);
		// rom_bin_info info2 = rom_bin_info_get(ROM_RES_STANDY_TIP2_PNG);
		// lv_obj_t * img1 = lv_img_create(mailbox_cont, NULL);
		// lv_img_set_src(img1, &info2);
		// lv_obj_set_pos(img1, 33,0);
		// lv_obj_set_size(img1, 22, 16);

		// char str[5] = {0};
		// if(user_data_get()->door1.mailbox_num < 99)
		// {
		// 	sprintf(str, "%d",user_data_get()->door1.mailbox_num);
		// }
		// else
		// {
		// 	sprintf(str, "%s","99");
		// }

		// lv_obj_t * label = lv_label_create(mailbox_cont, NULL);
		// lv_label_set_text(label, str);
		// lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE_L(16));
		// lv_obj_align(label, img1, LV_ALIGN_CENTER, 0, 0);
		// mailbox_cont->user_data = label;
		// lv_obj_set_auto_realign(label, true);
	}
	Debug("%d\n", user_data_get()->door1.mailbox_num);
	if (user_data_get()->door1.mailbox_num == 0)
	{
		lv_obj_set_hidden(mailbox_cont, true);
	}
}
#endif

static lv_obj_t *motion_channel_label = NULL;

static void motion_channel_label_display(void)
{

	MONITOR_CH ch = monitor_channel_get();
	if (ch == MON_CH_DOOR_1)
	{
		lv_label_set_text(motion_channel_label, text_str(STR_DOOR1));
	}
	else if (ch == MON_CH_DOOR_2)
	{
		lv_label_set_text(motion_channel_label, text_str(STR_DOOR2));
	}
	else if (ch == MON_CH_CCTV_1)
	{
		lv_label_set_text(motion_channel_label, text_str(STR_CAMERA1));
	}
	else if (ch == MON_CH_CCTV_2)
	{
		lv_label_set_text(motion_channel_label, text_str(STR_CAMERA1));
	}
}

static void motion_back_btn_up(lv_obj_t *obj)
{
	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2) || tuya_monitor_state_get())
	{
		return;
	}
	monitor_channel_set(MON_CH_NONE);
	goto_layout(pLAYOUT(standby));
}

static lv_task_t *motion_time_ptask = NULL;
static lv_task_t *motion_record_task_t = NULL;
static int motion_timeout_val = 60;
static void motion_record_task(lv_task_t *task_t);
extern bool get_video_data_display_state(void);

static void motion_timer_task(struct _lv_task_t *task_t)
{
	if (device_online_state_get(DEVICE_OUTDOOR_1 + monitor_channel_get() - 1) == false)
	{
		record_video_stop(0x00); // 停止录像
		if (monitor_enter_way_get() != MONITOR_ENTER_TUYA)
		{
			monitor_video_mode_close();
			monitor_channel_set(MON_CH_NONE);
			// if(motion_time_ptask != NULL)
			// {
			// 	lv_task_del(motion_time_ptask);
			// 	motion_time_ptask = NULL;
			// }
			goto_layout(pLAYOUT(standby));
		}
		return;
	}

	if (tuya_monitor_state_get())
	{
		record_video_stop(0x00); // 停止录像
		if (monitor_enter_way_get() != MONITOR_ENTER_TUYA)
		{
			monitor_video_mode_close();
			monitor_channel_set(MON_CH_NONE);
			goto_layout(pLAYOUT(standby));
		}
		return;
	}

	if (task_t->user_data != NULL)
	{
		lv_obj_t *time_label = (lv_obj_t *)task_t->user_data;
		if (monitor_enter_way_get() == MONITOR_ENTER_TUYA)
		{
			lv_label_set_text_fmt(time_label, "%s", text_str(STR_APP_PREVIEW));
			outdoor_order_set(NET_COMMON_PARAM_CAMERA_TUYA);
			lv_obj_align(time_label, LV_LABEL_PART_MAIN, LV_ALIGN_IN_RIGHT_MID, 0 - 10, 0);
			return;
		}
		else if (is_jpg_record_ing())
		{
			lv_label_set_text_fmt(time_label, "%s %02d", text_str(STR_SNAPSHOT), motion_timeout_val);
		}
		else if (is_video_recording())
		{
			lv_label_set_text_fmt(time_label, "%s %02d", text_str(STR_REC), motion_timeout_val);
		}
		else
		{
			lv_label_set_text_fmt(time_label, "%02d", motion_timeout_val);
		}
		lv_obj_align(time_label, time_label->parent, LV_ALIGN_IN_TOP_RIGHT, -40, 20);
	}

	static int time_1s_count = 2;
	;
	// int motion_timeout = monitor_channel_get() == MON_CH_DOOR_1?user_data_get()->door1.motion_duration:user_data_get()->door2.motion_duration;
	time_1s_count--;

	if (time_1s_count == 0)
	{
		time_1s_count = 2;
		if (get_video_data_display_state())
		{

			if (motion_timeout_val == 0)
			{
				record_video_stop(0x00); // 停止录像
				monitor_video_mode_close();
				monitor_channel_set(MON_CH_NONE);
				// if(motion_time_ptask != NULL)
				// {
				// 	lv_task_del(motion_time_ptask);
				// 	motion_time_ptask = NULL;
				// }
				goto_layout(pLAYOUT(standby));
			}
			else
			{
				motion_timeout_val--;
				// printf("motion_timeout_val :%d\n\r",motion_timeout_val);
			}
		}
	}
}

static void motion_timer_task_init(void *data)
{

	if (motion_time_ptask != NULL)
	{
		lv_task_del(motion_time_ptask);
	}
	monitor_enter_way_set(MONITOR_ENTER_MONTION);
	tuya_event_register(tuya_event_motion_proc);

	motion_time_ptask = lv_task_create(motion_timer_task, 500, LV_TASK_PRIO_HIGH, data);
	lv_task_ready(motion_time_ptask);

	// motion_timer_task(motion_time_ptask);
}

static void motion_2_head_create(void)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_cont_set_layout(cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(cont, LV_FIT_NONE);
	lv_obj_set_pos(cont, 0, 0);
	lv_obj_set_size(cont, 1024, 60);
	lv_obj_set_style_local_bg_opa(cont, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
	lv_obj_set_style_local_bg_color(cont, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
	lv_obj_set_auto_realign(cont, true);

	motion_channel_label = lv_label_create(cont, NULL);
	lv_obj_set_size(motion_channel_label, 120, 60);

	lv_label_set_align(motion_channel_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_color(motion_channel_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xdd3d3d));
	lv_obj_set_style_local_text_font(motion_channel_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_align(motion_channel_label, cont, LV_ALIGN_IN_LEFT_MID, 30, 0);

	motion_channel_label_display();

	lv_obj_t *head_label = lv_label_create(cont, NULL);
	lv_label_set_text(head_label, text_str(STR_MOTION_DETECTION));
	lv_label_set_long_mode(head_label, LV_LABEL_LONG_EXPAND);
	lv_obj_set_style_local_text_color(head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xdd3d3d));
	lv_obj_set_style_local_text_font(head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_align(head_label, LV_LABEL_PART_MAIN, LV_ALIGN_CENTER, 0, 0);

	lv_obj_t *timer_label = lv_label_create(cont, NULL);
	lv_label_set_long_mode(timer_label, LV_LABEL_LONG_EXPAND);
	lv_obj_set_style_local_text_color(timer_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xdd3d3d));
	lv_obj_set_style_local_text_font(timer_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_align(timer_label, LV_LABEL_PART_MAIN, LV_ALIGN_IN_RIGHT_MID, -140, 0);

	back_btn = home_back_btn_create(motion_back_btn_up, NULL);

	motion_timer_task_init(timer_label);
}

static int motion_rec_timer = 0;
// static int motion_detect_t = 15;
// static lv_task_t * motion_record_colse_task_t = NULL;
// static void auto_record_colse_task(lv_task_t *task_t)
// {
// 	extern bool is_video_recording(void);
// 	extern bool is_jpg_record_ing(void);
// 	if(is_video_recording() == false && is_jpg_record_ing() == false){
// 		lv_task_del(motion_record_colse_task_t);
//     	motion_record_colse_task_t = NULL;
// 		return;
// 	}

// 	if(++motion_rec_timer >motion_detect_t){
// 		monitor_video_mode_close();
// 		lv_task_del(motion_record_colse_task_t);
//     	motion_record_colse_task_t = NULL;
// 	}

// }

static void motion_record_task(lv_task_t *task_t)
{
	if (is_sdcard_insert())
	{
		if (motion_record_mode_temp)
		{
			// Debug("==================>>>>%d\n\n\n",get_video_data_display_state());

			int free_space = sd_free_space_insufficient();
			if (free_space < 500)
			{
				// Debug("==================>>>>\n\n\n");
				if (sdcard_insert_msg_box == NULL)
				{
					sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
				}
			}

			if (free_space > 200 && record_video_start(REC_MODE_MOTION, true, monitor_channel_get()) == true)
			{
				// Debug("==================>>>>%d\n\n\n",get_video_data_display_state());
				motion_rec_timer = 0;
			}
			else if (free_space < 200)
			{
				extern void detect_sd_free_space(void);
				detect_sd_free_space();
			}

			// printf("%s======================>>%d\n",__func__,__LINE__);
			if (get_video_data_display_state())
			{
				Debug("==================>>>>%d\n\n\n", get_video_data_display_state());
				if (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED)
				{
					send_tuya_record(REC_MODE_MOTION);
				}
				lv_task_del(motion_record_task_t);
				motion_record_task_t = NULL;
			}
		}
		else if (get_video_data_display_state())
		{
			// Debug("==================>>>>\n\n\n");

			int free_space = sd_free_space_insufficient();
			if (free_space < 500)
			{
				// Debug("==================>>>>\n\n\n");
				if (sdcard_insert_msg_box == NULL)
				{
					sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
				}
			}
			extern void jpg_push_to_tuya(int type);
			jpg_push_to_tuya(1);
			if (free_space > 200 && record_pictrue_start(REC_MODE_MOTION, monitor_channel_get()) == true)
			{
				Debug("===========>>>record_pictrue_start\n\n");
			}
			else if (free_space < 200)
			{
				extern void detect_sd_free_space(void);
				detect_sd_free_space();
			}
			lv_task_del(motion_record_task_t);
			motion_record_task_t = NULL;
		}
	}
	else
	{

		printf("%s======================>>%d\n", __func__, __LINE__);
		if (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED)
		{
			send_tuya_record(REC_MODE_MOTION);
		}

		lv_task_del(motion_record_task_t);
		motion_record_task_t = NULL;
	}

	// motion_record_colse_task_t = lv_task_create(auto_record_colse_task, 1000, LV_TASK_PRIO_HIGH, NULL);
}

static void standby_obj_disable(void)
{
	standby_time_display_disable();
	standby_shortcat_disable();

#if defined(WEATHER_MOUDLE)
	standby_weather_widgets_hidden(true);
#endif

#ifdef MAILBOX_MODULE_EN
	mailbox_status_disable();
#endif
}

static lv_task_t *motion_detect_task_t = NULL;
static void motion_detect_task(lv_task_t *task_t)
{

	if (motion_detect_task_t != NULL)
	{
		lv_task_del(motion_detect_task_t);
		motion_detect_task_t = NULL;
	}
	if (is_sdcard_insert() == false)
	{
		user_data_get()->door1.motion_record_mode = user_data_get()->door2.motion_record_mode = false;
#ifndef PUBLIC_VERSION
		user_data_get()->door1.motion_sensitivity = user_data_get()->door2.motion_sensitivity = 0;
#endif
	}
	Debug("==============================>>>\n\r");
	motion_detect_event_register(motion_detect_func); // 启动移动侦测
}

static void standby_task(lv_task_t *task_t)
{
	Debug("==============================>>>\n\r");

	sdcard_event_register(NULL);

	standby_shortcat_disable();
	if (sdcard_insert_msg_box != NULL)
	{
		lv_obj_del(sdcard_insert_msg_box);
		sdcard_insert_msg_box = NULL;
	}
	// lv_obj_invalidate(lv_scr_act());

	if (standby_timer_ptask != NULL)
	{
		lv_task_del(standby_timer_ptask);
	}
	standby_timer_ptask = NULL;

#if defined(WEATHER_MOUDLE)
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	int curr_time = tm.tm_hour * 100 + tm.tm_min;
	// Debug("curr_time:%d duration_start:%d duration_end:%d\n",curr_time,user_data_get()->weather.duration_start,user_data_get()->weather.duration_end);
	if (user_data_get()->weather.weather_switch && (user_data_get()->weather.duration_start > user_data_get()->weather.duration_end ? (curr_time >= user_data_get()->weather.duration_start || curr_time < user_data_get()->weather.duration_end) : (curr_time >= user_data_get()->weather.duration_start && curr_time < user_data_get()->weather.duration_end)))
	{
		standby_weather_widgets_hidden(false);
		extern bool get_p2p_online_status(void);
		if (backlight_status_get() == false)
		{
			standby_black_screen(false); //backlight_open(true, false, user_data_get()->other.brightness);
		}

		if (get_p2p_online_status())
		{
			standby_weather_widgets_create(); // 19ms
			return;
		}
	}
	else if (user_data_get()->weather.weather_switch)
	{
		standby_weather_widgets_hidden(true);
	}
	else
	{
		if (standby_task_t != NULL)
		{
			lv_task_del(standby_task_t);
		}
		standby_task_t = NULL;
	}
#else
	if (standby_task_t != NULL)
	{
		lv_task_del(standby_task_t);
	}
	standby_task_t = NULL;
#endif

	if (backlight_status_get() == true)
	{
		standby_black_screen(true); //backlight_open(false, false, user_data_get()->other.brightness);
		extern void gui_raw_clear(void);
		extern void screen_force_refresh(void);
		gui_raw_clear();
		screen_force_refresh();
		system_bg_fill_color(0x00, 0, 0, 1024, 600);
	}
}

static bool custom_music_power = false;
static void custom_music_play_stop(void)
{
	custom_music_power = false;
}
static void custom_music_play_finsih_callback(void)
{
	Debug("custom_music_power:%d\n", custom_music_power);
	if (custom_music_power == false)
	{
		return;
	}
	if (cur_music_index >= music_total)
	{
		cur_music_index = 0;
	}
	media_info *info = media_info_get(music_file_type, cur_music_index);
	printf("custom_music_play index :%d       name:%s\n\r", cur_music_index, info->file_name);
	custom_music_play(info->file_name, user_data_get()->scene.bg_music_vol, false, NULL, custom_music_play_finsih_callback);
	cur_music_index++;
}
static void music_play_parameter_init(void)
{
	Debug("============================\n\r");
	music_file_type = FILE_TYPE_SD_MUSIC;
	cur_music_index = 0;
	music_total = media_file_total_get(music_file_type, 0);
	if (music_total > 0)
	{
		custom_music_power = true;
		custom_music_play_finsih_callback();
	}
}

static int picture_play_parameter_init(void)
{
	picture_file_type = FILE_TYPE_SD_PICTURE;
	cur_picture_index = 0;
	picture_total = 0;
	picture_total = media_file_total_get(picture_file_type, 0);
	Debug("============================>picture_total:%d\n\r", picture_total);
	if (picture_total > 0)
	{
		if (digital_picture_ptask == NULL)
			digital_picture_ptask = lv_task_create(digital_picture_task, user_data_get()->scene.digital_photo_sw_time * 1000, LV_TASK_PRIO_MID, &clock); // 1秒任务
		lv_task_ready(digital_picture_ptask);
	}
	return picture_total;
}

USER_OBJ_INIT(STORTCAT_OBJ, SHORTCUT_TOTAL){
	[SHORTCUT_CONT] = {.area = {0, 480, 1024, 120}, .parent = NULL, USER_OBJ_CONSTRUCTOR(SHORTCUT_CONT)},
	[MONITOR_MODULE] = {.area = {0, 15, SHORTCUT_KEY_WIDTH, SHORTCUT_KEY_WIDTH}, .parent = &USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT), USER_OBJ_CONSTRUCTOR(MONITOR_MODULE)},
	[CALL_MODULE] = {.area = {0, 15, SHORTCUT_KEY_WIDTH, SHORTCUT_KEY_WIDTH}, .parent = &USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT), USER_OBJ_CONSTRUCTOR(CALL_MODULE)},
#ifndef INTERCOM_VERSION
	[MESSAGE_MODULE] = {.area = {0, 15, SHORTCUT_KEY_WIDTH, SHORTCUT_KEY_WIDTH}, .parent = &USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT), USER_OBJ_CONSTRUCTOR(MESSAGE_MODULE)},
	[MOTION_MODULE] = {.area = {0, 15, SHORTCUT_KEY_WIDTH, SHORTCUT_KEY_WIDTH}, .parent = &USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT), USER_OBJ_CONSTRUCTOR(MOTION_MODULE)},

#ifndef PUBLIC_VERSION
	[OUTDOOR1_LOCK_MODULE] = {.area = {0, 15, SHORTCUT_KEY_WIDTH, SHORTCUT_KEY_WIDTH}, .parent = &USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT), USER_OBJ_CONSTRUCTOR(OUTDOOR1_LOCK_MODULE)},
	[OUTDOOR2_LOCK_MODULE] = {.area = {0, 15, SHORTCUT_KEY_WIDTH, SHORTCUT_KEY_WIDTH}, .parent = &USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT), USER_OBJ_CONSTRUCTOR(OUTDOOR2_LOCK_MODULE)},
#endif
#endif
};

static void standby_shortcat_disable(void)
{
	user_obj obj = USER_OBJ_GET(STORTCAT_OBJ, SHORTCUT_CONT);
	lv_obj_set_hidden(obj.obj, true);
}

static void alarm_event_callback(unsigned long arg1, unsigned long arg2)
{
}

void standby_sdcard_callback(unsigned long arg1, unsigned long arg2)
{
	if (standby_task_t != NULL)
		lv_task_reset(standby_task_t);
	setting_sdcard_callback(arg1, arg2);
	standby_shorcut_refresh();
}

static void LAYOUT_ENETER_FUNC(standby)
{
	sdcard_event_register(standby_sdcard_callback);
	alarm_event_register(alarm_event_callback);
	indoor_cmd_event_register(monitor_indoor_cmd_func);

	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	fb_video_mode_enable(false);
	lv_obj_t *obj = lv_scr_act();
	static btn_data btn_data = btn_data_up_create(standby_click_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
	lv_obj_set_click(obj, true);
#if defined(INTERCOM_VERSION)
	static rom_bin_info info = rom_bin_info_get(ROM_RES_BG_BG5_JPG);
#else
	static rom_bin_info info = rom_bin_info_get(ROM_RES_BG_BG3_JPG);
#endif

	if (is_sdcard_insert() == true && !user_data_get()->door1.motion_sensitivity && !user_data_get()->door2.motion_sensitivity && user_data_get()->scene.digital_photo_frame_sw && picture_play_parameter_init())
	{
		standby_black_screen(false); //backlight_open(true, false, user_data_get()->other.brightness);
	}
	else if (user_data_get()->other.screen_saver == 1)
	{
		standby_task_t = lv_task_create(standby_task, 60000, LV_TASK_PRIO_HIGH, NULL);
		lv_obj_invalidate(lv_scr_act());

		standby_time_display_create(); // 73ms

#ifdef MAILBOX_MODULE_EN
		mailbox_status_create(&info_mailbox);
#endif

		USER_OBJ_GROUP_CREATE(STORTCAT_OBJ);
		standby_black_screen(false); //backlight_open(true, false, user_data_get()->other.brightness);

		system_bg_loading(&info, false);

		dev_info_status_event_register(dev_info_status_callback);
		dev_info_status_callback(1, 0);
	}
	else
	{
		standby_black_screen(true); //backlight_open(false, false, user_data_get()->other.brightness);
		system_bg_loading(&info, false);

		extern void gui_raw_clear(void);
		extern void screen_force_refresh(void);
		gui_raw_clear();
		screen_force_refresh();

		system_bg_fill_color(0x00, 0, 0, 1024, 600);
	}
	// x = os_get_ms();
	if (motion_detect_task_t == NULL && user_data_get()->other.network_device == DEVICE_INDOOR_ID1 && (user_data_get()->door1.motion_sensitivity || user_data_get()->door2.motion_sensitivity))
	{
		motion_detect_task_t = lv_task_create(motion_detect_task, MOTION_DELECT_INTERVAL, LV_TASK_PRIO_HIGH, NULL);
	}
	// Debug("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();
	if (user_data_get()->other.model != MUTE_PATTERN && is_sdcard_insert() == true && !user_data_get()->door1.motion_sensitivity && !user_data_get()->door2.motion_sensitivity && user_data_get()->scene.bg_music_sw)
	{
		music_play_parameter_init();
	}
	wifi_control = false;
}

static void LAYOUT_QUIT_FUNC(standby)
{
	sdcard_event_register(setting_sdcard_callback);

	if (monitor_enter_way_get() != MONITOR_ENTER_CALL)
	{
		monitor_video_mode_close();
		audio_talk_close(true);
	}
	else
		record_video_stop(0x00);

	// standby_timer_open(-1, NULL);
	fb_background_enable(true);
	system_bg_enable_set(true);
	lv_obj_set_click(lv_scr_act(), false);
	motion_detect_event_register(NULL);
	custom_music_play_stop();
	audio_play_stop_set();
	dev_info_status_event_register(NULL);
	alarm_event_register(NULL);
	indoor_cmd_event_register(NULL);

	back_btn = NULL;
	if (standby_task_t != NULL)
	{
		lv_task_del(standby_task_t);
		standby_task_t = NULL;
	}
	if (standby_timer_ptask != NULL)
	{
		lv_task_del(standby_timer_ptask);
		standby_timer_ptask = NULL;
	}
	if (motion_time_ptask != NULL)
	{
		lv_task_del(motion_time_ptask);
		motion_time_ptask = NULL;

		if (monitor_enter_way_get() == MONITOR_ENTER_MONTION)
		{
			monitor_enter_way_set(MONITOR_ENTER_NONE);
		}
		tuya_event_register(tuya_event_extern_proc);
	}
	if (motion_record_task_t != NULL)
	{
		lv_task_del(motion_record_task_t);
		motion_record_task_t = NULL;
	}
	if (motion_detect_task_t != NULL)
	{
		lv_task_del(motion_detect_task_t);
		motion_detect_task_t = NULL;
	}

	if (digital_picture_ptask != NULL)
	{
		lv_task_del(digital_picture_ptask);
		digital_picture_ptask = NULL;
	}

	if (picture_data != NULL)
	{
		ak_mem_dma_free(picture_data);
		picture_data = NULL;
		lv_disp_set_bg_image(lv_disp_get_default(), NULL);
	}

#ifndef PUBLIC_VERSION
	if (unlock_1_task_t)
	{
		lv_task_del(unlock_1_task_t);
		unlock_1_task_t = NULL;
	}
	if (unlock_2_task_t)
	{
		lv_task_del(unlock_2_task_t);
		unlock_2_task_t = NULL;
	}

	lock_1_task_lock_flag = 0;
	lock_2_task_lock_flag = 0;
#endif

	// ak_sleep_ms(1000);

	// home_bg_display();
	outdoor_order_set(NET_COMMON_CMD_NONE);
	// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();
	if (monitor_enter_way_get() == MONITOR_ENTER_CALL)
	{
		wait_video_record_finish();
		// bool video_decode_wait_thread_quit(void);
		// video_decode_wait_thread_quit();
	}
	else
	{
		video_raw_clear();
		// void screen_force_refresh(void);
		// screen_force_refresh();
	}
	// 	printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();
	motion_timeout_val = 0;

	USER_OBJ_GROUP_DESTROY(STORTCAT_OBJ);
}

static void gui_draw_area_set_2(void)
{
	lv_area_t area[] = {{0, 0, 1024, 600}

	};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));
}
static void gui_draw_area_set_1(void)
{
	lv_area_t area[] = {
		{0, 0, 1024, 60},
		// {929, 505, 67, 67},
		{929, 505, 929 + 67, 505 + 67},
		{350, 187, 350 + 324, 187 + 226},
	};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));
}
static void monitor_video_mode_open(bool fb_video_enable)
{
	audio_talk_ctrl ctrl = {{monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2},
							(OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)),
							AI_AO_C,
							true,
							false,
							monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
	audio_talk_open(ctrl);
	Debug("==============monitor_video_mode_open====>>>>\n\n\n");

	extern void video_raw_clear(void);
	video_raw_clear();
	monitor_open(false); // 打开指定通道的监控
	// 这一段代码要放在monitor_open后面，monitor_open内部也调用fb_video_mode_enable函数，因此需要出来再次调用一次fb_video_mode_enable
	fb_video_mode_enable(fb_video_enable);
}

static void monitor_video_mode_close(void)
{
	Debug("==============monitor_video_mode_close====>>>>\n\n\n");
	record_video_stop(0x00); // 停止录像
	fb_video_mode_enable(false);
	monitor_close_1();
	gui_draw_area_set_2();
}

void motion_detect_func(unsigned long arg1, unsigned long arg2)
{
	Debug("==============MOTION_DETECT_FUNC====>>>>%d,%d\n\n\n", get_outdoor_talk_state(MON_CH_DOOR_1), get_outdoor_talk_state(MON_CH_DOOR_2));

	// int brightness = 4;
	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2) || tuya_monitor_state_get() || tuya_online_clinet_num_get() > 0) // 正在视频对讲其他机子无法操作
	{
		return;
	}

	network_device device = (network_device)arg1;
	if (!((device == DEVICE_OUTDOOR_1) && user_data_get()->door1.motion_sensitivity) && !((device == DEVICE_OUTDOOR_2) && user_data_get()->door2.motion_sensitivity))
	{
		return;
	}

	if (device == DEVICE_OUTDOOR_1)
	{
		monitor_channel_set(MON_CH_DOOR_1); // 选择通道
		// brightness = user_data_get()->door1.brightness;
		motion_record_mode_temp = user_data_get()->door1.motion_record_mode;
	}
	else if (device == DEVICE_OUTDOOR_2)
	{
		monitor_channel_set(MON_CH_DOOR_2);
		// brightness = user_data_get()->door2.brightness;
		motion_record_mode_temp = user_data_get()->door2.motion_record_mode;
	}
	else
	{
		return;
	}

	if (user_data_get()->other.screen_saver == 1 && user_data_get()->other.MD_preview)
	{
		dev_info_status_event_register(NULL);
		standby_obj_disable();
	}

	// outdoor_order_set(NET_COMMON_PARAM_CAMERA_TUYA);
	// network_cmd_data data;
	// data.cmd = NET_COMMON_CMD_STREAM_STATUS;
	// data.arg1 = get_video_data_display_state() | NET_COMMON_PARAM_CAMERA_TUYA;
	// data.arg2 = 1;//移动侦测开启视频接收
	// data.device = device;
	// network_send_cmd_data(&data);

	if (motion_record_task_t == NULL /* && motion_timeout == motion_timeout_val */)
	{
		motion_record_task_t = lv_task_create(motion_record_task, 500, LV_TASK_PRIO_HIGH, NULL);
		motion_record_task(NULL);
	}

	motion_timeout_val = device == DEVICE_OUTDOOR_1 ? user_data_get()->door1.motion_duration : user_data_get()->door2.motion_duration;
	monitor_video_mode_open(user_data_get()->other.MD_preview);
	motion_detect_event_register(NULL);

	// if(!get_video_data_display_state())
	// {
	// Debug("==============MOTION_DETECT_FUNC====>>>>\n\n\n");
	// 	return;
	// }
	if (user_data_get()->other.MD_preview)
	{
		gui_draw_area_set_1();
		Debug("==================>>>>\n\n\n");
		motion_2_head_create(); // 画UI
		standby_black_screen(false); //backlight_open(true, false, brightness);
		lv_obj_set_click(lv_scr_act(), false);
		standby_bg_disable();
	}
	else
	{
		Debug("==================>>>>\n\n\n");
		motion_timer_task_init(NULL);
		// backlight_open(false,false,user_data_get()->other.brightness);
	}

	if (standby_task_t != NULL)
	{
		lv_task_del(standby_task_t);
		standby_task_t = NULL;
	}
}

CREATE_LAYOUT(standby);
