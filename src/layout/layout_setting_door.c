#include "layout_define.h"
#include "leo_api.h"

// #define AUTO_RECORD_TIME
// #define MOTION_DETECT_SWITCH
#define MOTION_DETECT_SENSITIVITY
static bool set_door_flag = 0; // 0:door1   1:door2
extern bool set_door_ring_flag;

static door_info door1_temp;
static door_info door2_temp;

typedef enum door_module_list
{
	DOOR_ENABLE_MODULE,
	UNLOCK_DELAY_MODULE,
	UNGATE1_DELAY_MODULE,
	RECORD_MODE_MODULE,
#ifdef AUTO_RECORD_TIME
	RECORD_TIME_MODULE,
#endif

#ifdef MOTION_DETECT_SWITCH
	DETECT_SWITCH_MODULE,
#endif

#ifdef MOTION_DETECT_SENSITIVITY
	DETECT_SENSITIVITY_MODULE,
#endif

	MOTION_RECORD_MODE_MODULE,
	DETECT_DURATION_MODULE,
	MESSAGE_SWITCH_MODULE,
	MESSAGE_TIME_MODULE,
	RING_SET_MODULE,
	TOTAL_MODULE
} door_module_list;

#define DOOR_MODULE_COORDINATE_INIT { \
	{199, 75, 700, 52},               \
	{199, 127, 700, 52},              \
	{199, 179, 700, 52},              \
	{199, 231, 700, 52},              \
	{199, 283, 700, 52},              \
	{199, 335, 700, 52},              \
	{199, 387, 700, 52},              \
	{199, 439, 700, 52},              \
	{199, 491, 700, 52},              \
	{199, 543, 700, 52},              \
	{199, 595, 700, 52},              \
};

static void door_setting_display(void);

static void set_door1_flag_up(lv_obj_t *obj)
{
	printf("door1_temp.record_mode==============================>>%d\n\r", door1_temp.record_mode);
	extern void def_unlock_time_cmd(network_device device, door_info door);
	def_unlock_time_cmd(set_door_flag + DEVICE_OUTDOOR_1, set_door_flag ? door2_temp : door1_temp);
	if (set_door_flag != 0)
	{
		set_door_flag = 0;
		lv_obj_clean(lv_scr_act());
		door_setting_display();
	}
}
static void set_door2_flag_up(lv_obj_t *obj)
{
	printf("door2_temp.record_mode==============================>>%d\n\r", door2_temp.record_mode);
	extern void def_unlock_time_cmd(network_device device, door_info door);
	def_unlock_time_cmd(set_door_flag + DEVICE_OUTDOOR_1, set_door_flag ? door2_temp : door1_temp);
	if (set_door_flag != 1)
	{
		set_door_flag = 1;
		lv_obj_clean(lv_scr_act());
		door_setting_display();
	}
}

static void door_setting_btn_text_create(void)
{
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(btn, 58, 162);
	lv_obj_set_size(btn, 66, 66);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_DOOR1_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_DOOR1_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, set_door_flag ? &info1 : &info);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_DOOR1));
	if (set_door_flag)
	{
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	}
	else
	{
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	}
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	static btn_data btn_data1 = btn_data_create(NULL, set_door1_flag_up, NULL);
	btn->user_data = &btn_data1;
	btn_touch_event_listen(btn);

	lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(btn1, 58, 321);
	lv_obj_set_size(btn1, 66, 66);
	lv_obj_set_style_local_bg_opa(btn1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn1, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_DOOR2_FOCUS_PNG);
	static rom_bin_info info3 = rom_bin_info_get(ROM_RES_SETTING_DOOR2_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, set_door_flag ? &info2 : &info3);

	lv_obj_set_style_local_value_str(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_DOOR2));
	if (!set_door_flag)
	{
		lv_obj_set_style_local_value_color(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
		lv_obj_set_style_local_value_color(btn1, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	}
	else
	{
		lv_obj_set_style_local_value_color(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_color(btn1, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	}
	lv_obj_set_style_local_value_align(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(btn1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	static btn_data btn_data2 = btn_data_create(NULL, set_door2_flag_up, NULL);
	btn1->user_data = &btn_data2;
	btn_touch_event_listen(btn1);

	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_pos(label, 470, 28);
	lv_obj_set_size(label, 124, 38);

	lv_label_set_text(label, set_door_flag ? text_str(STR_DOOR2) : text_str(STR_DOOR1));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
}

static void door_set_btn_syn_up(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_DEFAULT);
}
static void door_set_btn_syn_down(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_PRESSED);
}

static void door_set_btn_syn_event(lv_obj_t *obj, lv_event_t event)
{

	if (LV_EVENT_PRESS_LOST == event)
	{
		door_set_btn_syn_up(obj);
	}
}

static void door_enable_sw_right_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);

	char *str1 = NULL;
	if (set_door_flag)
	{
		door2_temp.enable_sw = !door2_temp.enable_sw;
		str1 = door2_temp.enable_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	else
	{
		str1 = door1_temp.enable_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), DOOR_ENABLE_MODULE);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void door_enable_sw_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_enable_sw_right_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_enable_sw_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;

	char *str1 = NULL;

	if (set_door_flag)
	{
		str1 = door2_temp.enable_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	else
	{
		str1 = door1_temp.enable_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_STATE), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, DOOR_ENABLE_MODULE);
}

static void door_unlock_delay_set_left_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), UNLOCK_DELAY_MODULE);
	// Debug("pdata :%p         btn:%p    btn1:%p\n",pdata,btn,btn1);
	static char str1[32] = {0};
	bzero(str1, sizeof(str1));
	if (set_door_flag)
	{
		if (--door2_temp.unlock_delay < 1)
		{
			door2_temp.unlock_delay = 10;
		}
		sprintf(str1, "%d %s", door2_temp.unlock_delay, text_str(STR_S));
	}
	else
	{
		if (--door1_temp.unlock_delay < 1)
		{
			door1_temp.unlock_delay = 10;
		}
		sprintf(str1, "%d %s", door1_temp.unlock_delay, text_str(STR_S));
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void door_unlock_delay_set_right_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), UNLOCK_DELAY_MODULE);
	// Debug("obj :%p         btn:%p    btn1:%p\n",obj,btn,btn1);
	static char str1[32] = {0};
	bzero(str1, sizeof(str1));
	if (set_door_flag)
	{
		if (++door2_temp.unlock_delay > 10)
		{
			door2_temp.unlock_delay = 1;
		}
		sprintf(str1, "%d %s", door2_temp.unlock_delay, text_str(STR_S));
	}
	else
	{
		if (++door1_temp.unlock_delay > 10)
		{
			door1_temp.unlock_delay = 1;
		}
		sprintf(str1, "%d %s", door1_temp.unlock_delay, text_str(STR_S));
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void door_unlock_delay_set_btn_create(Controls_location coordinate)
{

	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_unlock_delay_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_unlock_delay_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;

	static char str1[32] = {0};
	bzero(str1, sizeof(str1));
	if (set_door_flag)
	{
		sprintf(str1, "%d %s", door2_temp.unlock_delay, text_str(STR_S));
	}
	else
	{
		sprintf(str1, "%d %s", door1_temp.unlock_delay, text_str(STR_S));
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_UNLOCK_TIME), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, UNLOCK_DELAY_MODULE);
}

#ifndef MOTION_DETECT_SWITCH
static void door_ungate1_delay_set_left_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), UNGATE1_DELAY_MODULE);
	// Debug("pdata :%p         btn:%p    btn1:%p\n",pdata,btn,btn1);
	static char str1[32] = {0};
	bzero(str1, sizeof(str1));
	if (set_door_flag)
	{
		if (--door2_temp.ungate1_delay < 1)
		{
			door2_temp.ungate1_delay = 10;
		}
		sprintf(str1, "%d %s", door2_temp.ungate1_delay, text_str(STR_S));
	}
	else
	{
		if (--door1_temp.ungate1_delay < 1)
		{
			door1_temp.ungate1_delay = 10;
		}
		sprintf(str1, "%d %s", door1_temp.ungate1_delay, text_str(STR_S));
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void door_ungate1_delay_set_right_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), UNGATE1_DELAY_MODULE);
	// Debug("obj :%p         btn:%p    btn1:%p\n",obj,btn,btn1);
	static char str1[32] = {0};
	bzero(str1, sizeof(str1));
	if (set_door_flag)
	{
		if (++door2_temp.ungate1_delay > 10)
		{
			door2_temp.ungate1_delay = 1;
		}
		sprintf(str1, "%d %s", door2_temp.ungate1_delay, text_str(STR_S));
	}
	else
	{
		if (++door1_temp.ungate1_delay > 10)
		{
			door1_temp.ungate1_delay = 1;
		}
		sprintf(str1, "%d %s", door1_temp.ungate1_delay, text_str(STR_S));
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void door_ungate1_delay_set_btn_create(Controls_location coordinate)
{

	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_ungate1_delay_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_ungate1_delay_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;

	static char str1[32] = {0};
	bzero(str1, sizeof(str1));
	if (set_door_flag)
	{
		sprintf(str1, "%d %s", door2_temp.ungate1_delay, text_str(STR_S));
	}
	else
	{
		sprintf(str1, "%d %s", door1_temp.ungate1_delay, text_str(STR_S));
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_UNGATE_TIME), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, UNGATE1_DELAY_MODULE);
}
#endif

static void door_record_mode_set_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);

	if (is_sdcard_insert() && user_data_get()->other.model != NOT_AT_HOME_PATTERN)
	{

		char *str1 = NULL;
		if (set_door_flag)
		{
			door2_temp.record_mode = !door2_temp.record_mode;

			str1 = door2_temp.record_mode ? text_str(STR_VIDEO) : text_str(STR_SNAP);

			if (door2_temp.record_mode == false)
			{
				door2_temp.message_sw = false; // 切换拍照模式后留言关闭
				lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), MESSAGE_SWITCH_MODULE);

				lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_OFF));
			}
		}
		else
		{
			door1_temp.record_mode = !door1_temp.record_mode;
			str1 = door1_temp.record_mode ? text_str(STR_VIDEO) : text_str(STR_SNAP);

			if (door1_temp.record_mode == false)
			{
				door1_temp.message_sw = false; // 切换拍照模式后留言关闭
				lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), MESSAGE_SWITCH_MODULE);

				lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_OFF));
			}
		}
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), RECORD_MODE_MODULE);
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	}
	else
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(is_sdcard_insert() ? text_str(STR_IN_NOT_AT_HOME) : text_str(STR_PLEASE_INSERT_SD), NULL);
		}
	}
}

static void door_record_mode_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_record_mode_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_record_mode_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;

	char *str1 = NULL;

	if (set_door_flag)
	{
		str1 = door2_temp.record_mode ? text_str(STR_VIDEO) : text_str(STR_SNAP);
	}
	else
	{
		str1 = door1_temp.record_mode ? text_str(STR_VIDEO) : text_str(STR_SNAP);
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_RECORD_MODE), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, RECORD_MODE_MODULE);
}

#ifdef MOTION_DETECT_SWITCH

static void door_motion_detect_switch_set_btn_up(lv_obj_t *obj)
{
	if (user_data_get()->scene.digital_photo_frame_sw || user_data_get()->scene.bg_music_sw)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_CLOSE_SCENE_MODE), NULL);
		}
		return;
	}

	door_set_btn_syn_up(obj);

	char *str1 = NULL;
	if (set_door_flag)
	{
		door2_temp.motion_sw = !door2_temp.motion_sw;
		str1 = door2_temp.motion_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	else
	{
		door1_temp.motion_sw = !door1_temp.motion_sw;
		str1 = door1_temp.motion_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), DETECT_SWITCH_MODULE);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void door_motion_detect_switch_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_motion_detect_switch_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_motion_detect_switch_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;

	char *str1 = NULL;

	if (set_door_flag)
	{
		str1 = door2_temp.motion_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	else
	{
		str1 = door1_temp.motion_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_MOTION_DETECTION), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, DETECT_SWITCH_MODULE);
}
#endif

#ifdef MOTION_DETECT_SENSITIVITY

void motion_sensitivity_set_cmd(network_device ch)
{
	if (ch < DEVICE_OUTDOOR_1 || ch > DEVICE_OUTDOOR_2)
		return;

	network_cmd_data data;
	data.cmd = NET_COMON_CMD_MOTION_SENSITIVITY;
	data.arg1 = ch == DEVICE_OUTDOOR_1 ? door1_temp.motion_sensitivity : door2_temp.motion_sensitivity;
	data.arg2 = 0;
	data.device = ch;
	network_send_cmd_data(&data);
}

static void door_motion_record_sensitivity_sub_btn_up(lv_obj_t *obj)
{
	if (user_data_get()->scene.digital_photo_frame_sw || user_data_get()->scene.bg_music_sw)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_CLOSE_SCENE_MODE), NULL);
		}
		return;
	}

	door_set_btn_syn_up(obj);
	if (user_data_get()->other.network_device != DEVICE_INDOOR_ID1)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_ONLY_DEVICE_1_SET), NULL);
		}
		return;
	}
	else if (is_sdcard_insert() == false)
	{
#ifdef PUBLIC_VERSION
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_PLEASE_INSERT_SD), NULL);
		}
#endif
	}

	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), DETECT_SENSITIVITY_MODULE);

	char *str1 = NULL;
	if (set_door_flag)
	{
		door2_temp.motion_sensitivity = door2_temp.motion_sensitivity > 0 ? door2_temp.motion_sensitivity - 1 : 3;
		str1 = text_str(door2_temp.motion_sensitivity ? (STR_LOW + door2_temp.motion_sensitivity - 1) : STR_OFF);
	}
	else
	{
		door1_temp.motion_sensitivity = door1_temp.motion_sensitivity > 0 ? door1_temp.motion_sensitivity - 1 : 3;
		str1 = text_str(door1_temp.motion_sensitivity ? (STR_LOW + door1_temp.motion_sensitivity - 1) : STR_OFF);
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	motion_sensitivity_set_cmd(set_door_flag ? DEVICE_OUTDOOR_2 : DEVICE_OUTDOOR_1);
}

static void door_motion_record_sensitivity_add_btn_up(lv_obj_t *obj)
{
	if (user_data_get()->scene.digital_photo_frame_sw || user_data_get()->scene.bg_music_sw)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_CLOSE_SCENE_MODE), NULL);
		}
		return;
	}

	door_set_btn_syn_up(obj);
	if (user_data_get()->other.network_device != DEVICE_INDOOR_ID1)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_ONLY_DEVICE_1_SET), NULL);
		}
		return;
	}
	else if (is_sdcard_insert() == false)
	{
#ifdef PUBLIC_VERSION
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_PLEASE_INSERT_SD), NULL);
		}
#endif
	}

	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), DETECT_SENSITIVITY_MODULE);

	char *str1 = NULL;
	if (set_door_flag)
	{
		door2_temp.motion_sensitivity = door2_temp.motion_sensitivity < 3 ? door2_temp.motion_sensitivity + 1 : 0;
		str1 = text_str(door2_temp.motion_sensitivity ? (STR_LOW + door2_temp.motion_sensitivity - 1) : STR_OFF);
	}
	else
	{
		door1_temp.motion_sensitivity = door1_temp.motion_sensitivity < 3 ? door1_temp.motion_sensitivity + 1 : 0;
		str1 = text_str(door1_temp.motion_sensitivity ? (STR_LOW + door1_temp.motion_sensitivity - 1) : STR_OFF);
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	motion_sensitivity_set_cmd(set_door_flag ? DEVICE_OUTDOOR_2 : DEVICE_OUTDOOR_1);
}

static void door_motion_record_sensitivity_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_motion_record_sensitivity_sub_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_motion_record_sensitivity_add_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;

#ifndef PUBLIC_VERSION
	if (is_sdcard_insert() == false)
	{
		door1_temp.motion_sensitivity = door2_temp.motion_sensitivity = 0;
	}
#endif

	char *str1 = NULL;
	if (set_door_flag)
	{
		str1 = text_str(door2_temp.motion_sensitivity ? (STR_LOW + door2_temp.motion_sensitivity - 1) : STR_OFF);
	}
	else
	{
		str1 = text_str(door1_temp.motion_sensitivity ? (STR_LOW + door1_temp.motion_sensitivity - 1) : STR_OFF);
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_MOTION_DETECTION), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, DETECT_SENSITIVITY_MODULE);
}
#endif

static void door_motion_record_mode_set_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);
	if (user_data_get()->other.network_device != DEVICE_INDOOR_ID1)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_ONLY_DEVICE_1_SET), NULL);
		}
		return;
	}

	if (is_sdcard_insert())
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), MOTION_RECORD_MODE_MODULE);
		char *str1 = NULL;
		if (set_door_flag)
		{
			door2_temp.motion_record_mode = !door2_temp.motion_record_mode;
			str1 = door2_temp.motion_record_mode ? text_str(STR_VIDEO) : text_str(STR_SNAP);
		}
		else
		{
			door1_temp.motion_record_mode = !door1_temp.motion_record_mode;
			str1 = door1_temp.motion_record_mode ? text_str(STR_VIDEO) : text_str(STR_SNAP);
		}
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	}
	else
	{

		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_PLEASE_INSERT_SD), NULL);
		}
	}
}

static void door_motion_record_mode_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_motion_record_mode_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_motion_record_mode_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;

	char *str1 = NULL;
	if (set_door_flag)
	{
		str1 = door2_temp.motion_record_mode ? text_str(STR_VIDEO) : text_str(STR_SNAP);
	}
	else
	{
		str1 = door1_temp.motion_record_mode ? text_str(STR_VIDEO) : text_str(STR_SNAP);
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_MOTION_DETECT_RECORD_MODE), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, MOTION_RECORD_MODE_MODULE);
}

static char motion_detect_duration_str[32] = {0};

static void door_motion_detect_duration_right_set_btn_up(lv_obj_t *obj)
{
	if (user_data_get()->other.network_device != DEVICE_INDOOR_ID1)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_ONLY_DEVICE_1_SET), NULL);
		}
		return;
	}
	door_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), DETECT_DURATION_MODULE);
	if (set_door_flag)
	{
		if (door2_temp.motion_duration < 20)
		{
			door2_temp.motion_duration++;
		}
		else if (door2_temp.motion_duration >= 20)
		{
			door2_temp.motion_duration += 10;
		}
		if (door2_temp.motion_duration > 300)
		{
			door2_temp.motion_duration = 10;
		}
		sprintf(motion_detect_duration_str, "%d %s", door2_temp.motion_duration, text_str(STR_S));
	}
	else
	{
		if (door1_temp.motion_duration < 20)
		{
			door1_temp.motion_duration++;
		}
		else if (door1_temp.motion_duration >= 20)
		{
			door1_temp.motion_duration += 10;
		}
		if (door1_temp.motion_duration > 300)
		{
			door1_temp.motion_duration = 10;
		}
		sprintf(motion_detect_duration_str, "%d %s", door1_temp.motion_duration, text_str(STR_S));
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, motion_detect_duration_str);
}

static void door_motion_detect_duration_left_set_btn_up(lv_obj_t *obj)
{
	if (user_data_get()->other.network_device != DEVICE_INDOOR_ID1)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_ONLY_DEVICE_1_SET), NULL);
		}
		return;
	}
	door_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), DETECT_DURATION_MODULE);
	if (set_door_flag)
	{
		if (door2_temp.motion_duration <= 20)
		{
			door2_temp.motion_duration--;
		}
		else if (door2_temp.motion_duration > 20)
		{
			door2_temp.motion_duration -= 10;
		}
		if (door2_temp.motion_duration < 10)
		{
			door2_temp.motion_duration = 300;
		}
		sprintf(motion_detect_duration_str, "%d %s", door2_temp.motion_duration, text_str(STR_S));
	}
	else
	{
		if (door1_temp.motion_duration <= 20)
		{
			door1_temp.motion_duration--;
		}
		else if (door1_temp.motion_duration > 20)
		{
			door1_temp.motion_duration -= 10;
		}
		if (door1_temp.motion_duration < 10)
		{
			door1_temp.motion_duration = 300;
		}
		sprintf(motion_detect_duration_str, "%d %s", door1_temp.motion_duration, text_str(STR_S));
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, motion_detect_duration_str);
}

static void door_motion_detect_duration_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_motion_detect_duration_left_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_motion_detect_duration_right_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;

	if (set_door_flag)
	{
		sprintf(motion_detect_duration_str, "%d %s", door2_temp.motion_duration, text_str(STR_S));
	}
	else
	{
		sprintf(motion_detect_duration_str, "%d %s", door1_temp.motion_duration, text_str(STR_S));
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, motion_detect_duration_str, text_str(STR_MOTION_DETECTION_DURATION), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, DETECT_DURATION_MODULE);
}

static void door_message_switch_set_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);

	if (user_data_get()->other.model == NOT_AT_HOME_PATTERN)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(is_sdcard_insert() ? text_str(STR_IN_NOT_AT_HOME) : text_str(STR_PLEASE_INSERT_SD), NULL);
		}
		return;
	}

	if (is_sdcard_insert())
	{
		char *str1 = NULL;
		if (set_door_flag)
		{
			door2_temp.message_sw = !door2_temp.message_sw;
			str1 = door2_temp.message_sw ? text_str(STR_ON) : text_str(STR_OFF);

			if (door2_temp.message_sw)
			{
				door2_temp.record_mode = true; // 开启留言后自动切换为视频记录
				lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), RECORD_MODE_MODULE);
				lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_VIDEO));
			}
		}
		else
		{
			door1_temp.message_sw = !door1_temp.message_sw;
			str1 = door1_temp.message_sw ? text_str(STR_ON) : text_str(STR_OFF);

			if (door1_temp.message_sw)
			{
				door1_temp.record_mode = true; // 开启留言后自动切换为视频记录
				lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), RECORD_MODE_MODULE);
				lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_VIDEO));
			}
		}

		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), MESSAGE_SWITCH_MODULE);
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	}
	else
	{

		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_PLEASE_INSERT_SD), NULL);
		}
	}
}

static void door_message_switch_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_message_switch_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_message_switch_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;
	char *str1 = NULL;

	if (set_door_flag)
	{
		str1 = door2_temp.message_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	else
	{
		str1 = door1_temp.message_sw ? text_str(STR_ON) : text_str(STR_OFF);
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_MESSAGE), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, MESSAGE_SWITCH_MODULE);
}

static void door_message_time_left_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);
	static char str1[32] = {0};
	if (set_door_flag)
	{
		door2_temp.message_time /= 2;
		if (door2_temp.message_time < 30)
		{
			door2_temp.message_time = 120;
		}
		sprintf(str1, "%d %s", door2_temp.message_time, text_str(STR_S));
	}
	else
	{
		door1_temp.message_time /= 2;
		if (door1_temp.message_time < 30)
		{
			door1_temp.message_time = 120;
		}
		sprintf(str1, "%d %s", door1_temp.message_time, text_str(STR_S));
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), MESSAGE_TIME_MODULE);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void door_message_time_right_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);
	static char str1[32] = {0};
	if (set_door_flag)
	{
		door2_temp.message_time *= 2;
		if (door2_temp.message_time > 120)
		{
			door2_temp.message_time = 30;
		}
		sprintf(str1, "%d %s", door2_temp.message_time, text_str(STR_S));
	}
	else
	{
		door1_temp.message_time *= 2;
		if (door1_temp.message_time > 120)
		{
			door1_temp.message_time = 30;
		}
		sprintf(str1, "%d %s", door1_temp.message_time, text_str(STR_S));
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), MESSAGE_TIME_MODULE);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void door_message_time_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_message_time_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(door_set_btn_syn_down, door_message_time_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = door_set_btn_syn_event;
	static char str1[32] = {0};
	if (set_door_flag)
	{
		sprintf(str1, "%d %s", door2_temp.message_time, text_str(STR_S));
	}
	else
	{
		sprintf(str1, "%d %s", door1_temp.message_time, text_str(STR_S));
	}
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_MESSAGE_TIME), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, MESSAGE_TIME_MODULE);
}

static void door_ring_setting_set_btn_up(lv_obj_t *obj)
{
	door_set_btn_syn_up(obj);
	if (set_door_flag)
	{
		set_door_ring_flag = 1;
		goto_layout(pLAYOUT(setting_ring));
	}
	else
	{
		set_door_ring_flag = 0;
		goto_layout(pLAYOUT(setting_ring));
	}
}
static void door_ring_setting_set_btn_up_1(lv_obj_t *obj)
{
	if (set_door_flag)
	{
		set_door_ring_flag = 1;
		goto_layout(pLAYOUT(setting_ring));
	}
	else
	{
		set_door_ring_flag = 0;
		goto_layout(pLAYOUT(setting_ring));
	}
}

static void door_ring_setting_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data3 = btn_data_create(NULL, door_ring_setting_set_btn_up_1, NULL);
	static btn_data btn_data1 = btn_data_create(door_set_btn_syn_down, door_ring_setting_set_btn_up, NULL);
	btn_data1.OPS_ANYTHING = door_set_btn_syn_event;
	lv_obj_t *btn = sys_setting_btn_create(coordinate, text_str(STR_RING_SETTING), text_str(STR_RING_SETTING), &btn_data3, NULL, &btn_data1);
	lv_obj_set_id(btn, RING_SET_MODULE);
}

static void door_setting_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}

static void door_setting_display(void)
{
	Controls_location module_coordinate[] = DOOR_MODULE_COORDINATE_INIT;
	door_setting_btn_text_create();
	door_enable_sw_set_btn_create(module_coordinate[DOOR_ENABLE_MODULE]);
	door_unlock_delay_set_btn_create(module_coordinate[UNLOCK_DELAY_MODULE]);
	door_ungate1_delay_set_btn_create(module_coordinate[UNGATE1_DELAY_MODULE]);
	door_record_mode_set_btn_create(module_coordinate[RECORD_MODE_MODULE]);

#ifdef MOTION_DETECT_SWITCH
	door_motion_detect_switch_set_btn_create(module_coordinate[DETECT_SWITCH_MODULE]);
#endif

#ifdef MOTION_DETECT_SENSITIVITY
	door_motion_record_sensitivity_set_btn_create(module_coordinate[DETECT_SENSITIVITY_MODULE]);
#endif
	door_motion_record_mode_set_btn_create(module_coordinate[MOTION_RECORD_MODE_MODULE]);
	door_motion_detect_duration_set_btn_create(module_coordinate[DETECT_DURATION_MODULE]);
#ifdef AUTO_RECORD_TIME
	door_record_time_set_btn_create();
#endif
	door_message_switch_set_btn_create(module_coordinate[MESSAGE_SWITCH_MODULE]);
	door_message_time_set_btn_create(module_coordinate[MESSAGE_TIME_MODULE]);
	door_ring_setting_set_btn_create(module_coordinate[RING_SET_MODULE]);
	home_back_btn_create(door_setting_back_btn_up, NULL);
}

static lv_task_t *sdcard_insert_detect = NULL;
static void sdcard_insert_detect_task(lv_task_t *task_t)
{
	static bool prev_insert_state = true;
	bool curr_insert_state = is_sdcard_insert();
	if (curr_insert_state != prev_insert_state)
	{
		prev_insert_state = curr_insert_state;
		if (prev_insert_state == false)
		{
			if (user_data_get()->other.model != NOT_AT_HOME_PATTERN)
			{
				door1_temp.message_sw = door2_temp.message_sw = false;
				door1_temp.record_mode = door2_temp.record_mode = false;
			}
			door1_temp.motion_record_mode = door2_temp.motion_record_mode = false;

			lv_obj_t *message_btn = lv_obj_get_child_form_id(lv_scr_act(), MESSAGE_SWITCH_MODULE);
			lv_obj_set_style_local_value_str(message_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, (set_door_flag ? door1_temp.message_sw : door2_temp.message_sw) ? text_str(STR_ON) : text_str(STR_OFF));

			lv_obj_t *motion_record_btn = lv_obj_get_child_form_id(lv_scr_act(), MOTION_RECORD_MODE_MODULE);
			lv_obj_set_style_local_value_str(motion_record_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, (set_door_flag ? door1_temp.motion_record_mode : door2_temp.motion_record_mode) ? text_str(STR_VIDEO) : text_str(STR_SNAP));

#ifndef PUBLIC_VERSION
			door1_temp.motion_sensitivity = door2_temp.motion_sensitivity = 0;
			lv_obj_t *motion_sensitivity_btn = lv_obj_get_child_form_id(lv_scr_act(), DETECT_SENSITIVITY_MODULE);
			lv_obj_set_style_local_value_str(motion_sensitivity_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_OFF));
#endif

			lv_obj_t *record_btn = lv_obj_get_child_form_id(lv_scr_act(), RECORD_MODE_MODULE);
			lv_obj_set_style_local_value_str(record_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, (set_door_flag ? door1_temp.record_mode : door2_temp.record_mode) ? text_str(STR_VIDEO) : text_str(STR_SNAP));
		}
	}
}

static void LAYOUT_ENETER_FUNC(setting_door)
{
	Debug("1+++++++++++++++++++++++++++++++%d\n\r", user_data_get()->door1.enable_sw);
	Debug("2+++++++++++++++++++++++++++++++%d\n\r", user_data_get()->door2.enable_sw);
	set_door_flag = set_door_ring_flag;

	door1_temp = user_data_get()->door1;
	door2_temp = user_data_get()->door2;
	setting_bg_display();
	door_setting_display();
	if (sdcard_insert_detect == NULL)
	{
		sdcard_insert_detect = lv_task_create(sdcard_insert_detect_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
		lv_task_ready(sdcard_insert_detect);
		sdcard_insert_detect_task(NULL);
	}
}

static void LAYOUT_QUIT_FUNC(setting_door)
{
	if (prompt_window != NULL)
	{
		lv_obj_del(prompt_window);
		prompt_window = NULL;
	}

	if (sdcard_insert_detect != NULL)
	{
		lv_task_del(sdcard_insert_detect);
		sdcard_insert_detect = NULL;
	}

	user_data_get()->door1 = door1_temp;
	user_data_get()->door2 = door2_temp;
	extern void def_unlock_time_cmd(network_device device, door_info door);
	Debug("door1:%d,%d   door2:%d,%d\n", door1_temp.unlock_delay, door1_temp.ungate1_delay, door2_temp.unlock_delay, door2_temp.ungate1_delay);
	def_unlock_time_cmd(set_door_flag + DEVICE_OUTDOOR_1, set_door_flag ? door2_temp : door1_temp);

	user_data_save();
}

CREATE_LAYOUT(setting_door);
