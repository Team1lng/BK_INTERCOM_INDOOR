#include "layout_define.h"
#include "leo_api.h"

#define RING_TIME_MIN 10
#define RING_TIME_MAX 45
typedef enum ring_module_list
{
	RING_SCHEDULE_MODULE,
	RING_TIME_MODULE,
	RING_MODE_MODULE,
	RING_SELECT_MODULE,
	RING_VOLUME_MODULE,
	TOTAL_MODULE
} ring_module_list;

#define RING_MODULE_COORDINATE_INIT { \
	{199, 75, 700, 52},               \
	{199, 127, 700, 52},              \
	{199, 179, 700, 52},              \
	{199, 231, 700, 52},              \
	{199, 285, 700, 52},              \
};

typedef struct
{
	int timer_start;
	int timer_end;

	int ring_time;

	int ring_mode;

	int ring;

	int custom_ring;

	int ring_val;
} ring_info;
static ring_info ring_temp = {0};

static void ring_setting_display(void);
bool set_door_ring_flag = 0; // 0:door1   1:door2

static int set_ring_flag = 0; // 0:ring1   1:ring2   2:ring3
static int door_ring_num = -1;

static void ring_setting_display(void);

static int custom_ring_total = 0;
static media_type custom_ring_type = FILE_TYPE_SD_MUSIC;

static void door_ring_fine(void)
{
	if (door_ring_num != -1)
	{
		return;
	}

	char cmd[128] = {0};
	sprintf(cmd, "find %s -maxdepth 1 -type f -name \"[0-9]*.mp3\" | wc -l", RING_FILE_PATH);
	FILE *fp = popen(cmd, "r");
	if (fp == NULL)
	{
		printf("无法执行命令\n");
		return;
	}

	// 读取输出并计数
	if (fgets(cmd, sizeof(cmd), fp) != NULL)
	{
		door_ring_num = atoi(cmd);
		Debug("找到的文件数量：%d\n", door_ring_num);
	}
}

static void ring_temp_init(void)
{
	if (set_door_ring_flag)
	{
		if (set_ring_flag == 0)
		{
			ring_temp.timer_start = user_data_get()->ring1.door2.timer_start;
			ring_temp.timer_end = user_data_get()->ring1.door2.timer_end;
			ring_temp.ring_time = user_data_get()->ring1.door2.ring_time;
			ring_temp.ring_mode = user_data_get()->ring1.door2.ring_mode;
			ring_temp.ring_val = user_data_get()->ring1.door2.ring_val;
			ring_temp.ring = user_data_get()->ring1.door2.ring;
			ring_temp.custom_ring = user_data_get()->ring1.door2.custom_ring;

			// 	Debug("-------ring_temp.ring_time = %d-------->>>>\n",ring_temp.ring_time);
			// Debug("-------ring_temp.ring_val = %d-------->>>>\n",ring_temp.ring_val);
			// Debug("-------ring_temp.ring = %d-------->>>>\n\n\n\n\n\n\n",ring_temp.ring);
		}
		else if (set_ring_flag == 1)
		{
			ring_temp.timer_start = user_data_get()->ring2.door2.timer_start;
			ring_temp.timer_end = user_data_get()->ring2.door2.timer_end;
			ring_temp.ring_time = user_data_get()->ring2.door2.ring_time;
			ring_temp.ring_mode = user_data_get()->ring2.door2.ring_mode;
			ring_temp.ring_val = user_data_get()->ring2.door2.ring_val;
			ring_temp.ring = user_data_get()->ring2.door2.ring;
			ring_temp.custom_ring = user_data_get()->ring2.door2.custom_ring;

			// 	Debug("-------ring_temp.ring_time = %d-------->>>>\n",ring_temp.ring_time);
			// Debug("-------ring_temp.ring_val = %d-------->>>>\n",ring_temp.ring_val);
			// Debug("-------ring_temp.ring = %d-------->>>>\n\n\n\n\n\n\n",ring_temp.ring);
		}
		else if (set_ring_flag == 2)
		{
			ring_temp.timer_start = user_data_get()->ring3.door2.timer_start;
			ring_temp.timer_end = user_data_get()->ring3.door2.timer_end;
			ring_temp.ring_time = user_data_get()->ring3.door2.ring_time;
			ring_temp.ring_mode = user_data_get()->ring3.door2.ring_mode;
			ring_temp.ring_val = user_data_get()->ring3.door2.ring_val;
			ring_temp.ring = user_data_get()->ring3.door2.ring;
			ring_temp.custom_ring = user_data_get()->ring3.door2.custom_ring;
			// 	Debug("-------ring_temp.ring_time = %d-------->>>>\n",ring_temp.ring_time);
			// Debug("-------ring_temp.ring_val = %d-------->>>>\n",ring_temp.ring_val);
			// Debug("-------ring_temp.ring = %d-------->>>>\n\n\n\n\n\n\n",ring_temp.ring);
		}
	}
	else
	{
		if (set_ring_flag == 0)
		{
			ring_temp.timer_start = user_data_get()->ring1.door1.timer_start;
			ring_temp.timer_end = user_data_get()->ring1.door1.timer_end;
			ring_temp.ring_time = user_data_get()->ring1.door1.ring_time;
			ring_temp.ring_mode = user_data_get()->ring1.door1.ring_mode;
			ring_temp.ring_val = user_data_get()->ring1.door1.ring_val;
			ring_temp.ring = user_data_get()->ring1.door1.ring;
			ring_temp.custom_ring = user_data_get()->ring1.door1.custom_ring;
		}
		else if (set_ring_flag == 1)
		{
			ring_temp.timer_start = user_data_get()->ring2.door1.timer_start;
			ring_temp.timer_end = user_data_get()->ring2.door1.timer_end;
			ring_temp.ring_time = user_data_get()->ring2.door1.ring_time;
			ring_temp.ring_mode = user_data_get()->ring2.door1.ring_mode;
			ring_temp.ring_val = user_data_get()->ring2.door1.ring_val;
			ring_temp.ring = user_data_get()->ring2.door1.ring;
			ring_temp.custom_ring = user_data_get()->ring2.door1.custom_ring;
		}
		else if (set_ring_flag == 2)
		{
			ring_temp.timer_start = user_data_get()->ring3.door1.timer_start;
			ring_temp.timer_end = user_data_get()->ring3.door1.timer_end;
			ring_temp.ring_time = user_data_get()->ring3.door1.ring_time;
			ring_temp.ring_mode = user_data_get()->ring3.door1.ring_mode;
			ring_temp.ring_val = user_data_get()->ring3.door1.ring_val;
			ring_temp.ring = user_data_get()->ring3.door1.ring;
			ring_temp.custom_ring = user_data_get()->ring3.door1.custom_ring;
		}
	}
}

static void ring_temp_save(void)
{
	if (set_door_ring_flag)
	{
		if (set_ring_flag == 0)
		{
			user_data_get()->ring1.door2.timer_start = ring_temp.timer_start;
			user_data_get()->ring1.door2.timer_end = ring_temp.timer_end;
			user_data_get()->ring1.door2.ring_time = ring_temp.ring_time;
			user_data_get()->ring1.door2.ring_mode = ring_temp.ring_mode;
			user_data_get()->ring1.door2.ring_val = ring_temp.ring_val;
			user_data_get()->ring1.door2.ring = ring_temp.ring;
			user_data_get()->ring1.door2.custom_ring = ring_temp.custom_ring;
			Debug("-------------->>>%d\n\n", user_data_get()->ring1.door2.ring_time);
			Debug("-------------->>>%d\n\n", user_data_get()->ring1.door2.ring_val);
			Debug("-------------->>>%d\n\n", user_data_get()->ring1.door2.ring);
		}
		else if (set_ring_flag == 1)
		{
			user_data_get()->ring2.door2.timer_start = ring_temp.timer_start;
			user_data_get()->ring2.door2.timer_end = ring_temp.timer_end;
			user_data_get()->ring2.door2.ring_time = ring_temp.ring_time;
			user_data_get()->ring2.door2.ring_mode = ring_temp.ring_mode;
			user_data_get()->ring2.door2.ring_val = ring_temp.ring_val;
			user_data_get()->ring2.door2.ring = ring_temp.ring;
			user_data_get()->ring2.door2.custom_ring = ring_temp.custom_ring;
			Debug("-------------->>>%d\n\n", user_data_get()->ring2.door2.ring_time);
			Debug("-------------->>>%d\n\n", user_data_get()->ring2.door2.ring_val);
			Debug("-------------->>>%d\n\n", user_data_get()->ring2.door2.ring);
		}
		else if (set_ring_flag == 2)
		{
			user_data_get()->ring3.door2.timer_start = ring_temp.timer_start;
			user_data_get()->ring3.door2.timer_end = ring_temp.timer_end;
			user_data_get()->ring3.door2.ring_time = ring_temp.ring_time;
			user_data_get()->ring3.door2.ring_mode = ring_temp.ring_mode;
			user_data_get()->ring3.door2.ring_val = ring_temp.ring_val;
			user_data_get()->ring3.door2.ring = ring_temp.ring;
			user_data_get()->ring3.door2.custom_ring = ring_temp.custom_ring;
			Debug("-------------->>>%d\n\n", user_data_get()->ring3.door2.ring_time);
			Debug("-------------->>>%d\n\n", user_data_get()->ring3.door2.ring_val);
			Debug("-------------->>>%d\n\n", user_data_get()->ring3.door2.ring);
		}
	}
	else
	{
		if (set_ring_flag == 0)
		{
			user_data_get()->ring1.door1.timer_start = ring_temp.timer_start;
			user_data_get()->ring1.door1.timer_end = ring_temp.timer_end;
			user_data_get()->ring1.door1.ring_time = ring_temp.ring_time;
			user_data_get()->ring1.door1.ring_mode = ring_temp.ring_mode;
			user_data_get()->ring1.door1.ring_val = ring_temp.ring_val;
			user_data_get()->ring1.door1.ring = ring_temp.ring;
			user_data_get()->ring1.door1.custom_ring = ring_temp.custom_ring;
		}
		else if (set_ring_flag == 1)
		{
			user_data_get()->ring2.door1.timer_start = ring_temp.timer_start;
			user_data_get()->ring2.door1.timer_end = ring_temp.timer_end;
			user_data_get()->ring2.door1.ring_time = ring_temp.ring_time;
			user_data_get()->ring2.door1.ring_mode = ring_temp.ring_mode;
			user_data_get()->ring2.door1.ring_val = ring_temp.ring_val;
			user_data_get()->ring2.door1.ring = ring_temp.ring;
			user_data_get()->ring2.door1.custom_ring = ring_temp.custom_ring;
		}
		else if (set_ring_flag == 2)
		{
			user_data_get()->ring3.door1.timer_start = ring_temp.timer_start;
			user_data_get()->ring3.door1.timer_end = ring_temp.timer_end;
			user_data_get()->ring3.door1.ring_time = ring_temp.ring_time;
			user_data_get()->ring3.door1.ring_mode = ring_temp.ring_mode;
			user_data_get()->ring3.door1.ring_val = ring_temp.ring_val;
			user_data_get()->ring3.door1.ring = ring_temp.ring;
			user_data_get()->ring3.door1.custom_ring = ring_temp.custom_ring;
		}
	}
}

static void set_ring1_flag_up(lv_obj_t *obj)
{
	if (set_ring_flag != 0)
	{
		ring_temp_save();
		ring_init();
		set_ring_flag = 0;
		lv_obj_clean(lv_scr_act());
		ring_temp_init();
		ring_setting_display();
	}
}
static void set_ring2_flag_up(lv_obj_t *obj)
{
	if (set_ring_flag != 1)
	{
		ring_temp_save();
		ring_init();
		set_ring_flag = 1;
		lv_obj_clean(lv_scr_act());
		ring_temp_init();
		ring_setting_display();
	}
}
static void set_ring3_flag_up(lv_obj_t *obj)
{
	if (set_ring_flag != 2)
	{
		ring_temp_save();
		ring_init();
		set_ring_flag = 2;
		lv_obj_clean(lv_scr_act());
		ring_temp_init();
		ring_setting_display();
	}
}

static void ring_setting_btn_text_create(void)
{
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(btn, 56, 90);
	lv_obj_set_size(btn, 66, 66);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_RING1_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_RING1_UNFOCUS_PNG);
	if (set_ring_flag == 0)
	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	}
	else
	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
	}

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_RING1));
	if (set_ring_flag != 0)
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
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	static btn_data btn_data1 = btn_data_create(NULL, set_ring1_flag_up, NULL);
	btn->user_data = &btn_data1;
	btn_touch_event_listen(btn);

	lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(btn1, 56, 249);
	lv_obj_set_size(btn1, 66, 66);
	lv_obj_set_style_local_bg_opa(btn1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn1, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_RING2_FOCUS_PNG);
	static rom_bin_info info3 = rom_bin_info_get(ROM_RES_SETTING_RING2_UNFOCUS_PNG);
	if (set_ring_flag == 1)
	{
		lv_obj_set_style_local_pattern_image(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
	}
	else
	{
		lv_obj_set_style_local_pattern_image(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info3);
	}

	lv_obj_set_style_local_value_str(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_RING2));
	if (set_ring_flag != 1)
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
	lv_obj_set_style_local_value_font(btn1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	static btn_data btn_data2 = btn_data_create(NULL, set_ring2_flag_up, NULL);
	btn1->user_data = &btn_data2;
	btn_touch_event_listen(btn1);

	lv_obj_t *btn3 = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(btn3, 56, 409);
	lv_obj_set_size(btn3, 66, 66);
	lv_obj_set_style_local_bg_opa(btn3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn3, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info4 = rom_bin_info_get(ROM_RES_SETTING_RING3_FOCUS_PNG);
	static rom_bin_info info5 = rom_bin_info_get(ROM_RES_SETTING_RING3_UNFOCUS_PNG);
	if (set_ring_flag == 2)
	{
		lv_obj_set_style_local_pattern_image(btn3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info4);
	}
	else
	{
		lv_obj_set_style_local_pattern_image(btn3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info5);
	}

	lv_obj_set_style_local_value_str(btn3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_RING3));
	if (set_ring_flag != 2)
	{
		lv_obj_set_style_local_value_color(btn3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
		lv_obj_set_style_local_value_color(btn3, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	}
	else
	{
		lv_obj_set_style_local_value_color(btn3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_color(btn3, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	}
	lv_obj_set_style_local_value_align(btn3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(btn3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	static btn_data btn_data3 = btn_data_create(NULL, set_ring3_flag_up, NULL);
	btn3->user_data = &btn_data3;
	btn_touch_event_listen(btn3);

	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_pos(label, 470, 28);
	lv_obj_set_size(label, 124, 32);
	lv_label_set_text(label, text_str(STR_RING1_DOOR1 + (set_door_ring_flag * 3) + set_ring_flag));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
}

static void ring_set_btn_syn_up(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_DEFAULT);
}

static void ring_set_btn_syn_down(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_PRESSED);
}

static void ring_set_btn_syn_event(lv_obj_t *obj, lv_event_t event)
{

	if (LV_EVENT_PRESS_LOST == event)
	{
		ring_set_btn_syn_up(obj);
	}
}

static int ring_start_time_temp, ring_end_time_temp;

static void ring_time_roller_event(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t *parent = lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(), 888), 666);
		if (parent == NULL)
		{
			return;
		}

		lv_obj_t *obj_roller = obj_roller = lv_obj_get_child_form_id(parent, 11);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			ring_start_time_temp = ring_start_time_temp % 100;
			ring_start_time_temp += id * 100;
			return;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 22);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			ring_start_time_temp = ring_start_time_temp / 100;
			ring_start_time_temp = ring_start_time_temp * 100;
			ring_start_time_temp += id;
			return;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 33);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			ring_end_time_temp = ring_end_time_temp % 100;
			ring_end_time_temp += id * 100;
			return;
		}

		obj_roller = lv_obj_get_child_form_id(parent, 44);
		if (obj_roller == obj)
		{
			int id = lv_roller_get_selected(obj_roller);
			ring_end_time_temp = ring_end_time_temp / 100;
			ring_end_time_temp = ring_end_time_temp * 100;
			ring_end_time_temp += id;
			return;
		}
	}
}

static void ring_time_roller_btn_syn_prev(lv_obj_t *obj)
{
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

	lv_obj_t *obj_roller = lv_obj_get_child_form_id(parent, 11);

	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		ring_start_time_temp = ring_start_time_temp % 100;
		ring_start_time_temp += (lv_roller_get_selected(obj_roller)) * 100;
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 22);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		ring_start_time_temp = ring_start_time_temp / 100;
		ring_start_time_temp = ring_start_time_temp * 100;
		ring_start_time_temp += lv_roller_get_selected(obj_roller);
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 33);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		ring_end_time_temp = ring_end_time_temp % 100;
		ring_end_time_temp += lv_roller_get_selected(obj_roller) * 100;
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 44);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &up_char);
		ring_end_time_temp = ring_end_time_temp / 100;
		ring_end_time_temp = ring_end_time_temp * 100;
		ring_end_time_temp += lv_roller_get_selected(obj_roller);
		return;
	}
}
static void ring_time_roller_btn_syn_next(lv_obj_t *obj)
{
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

	lv_obj_t *obj_roller = lv_obj_get_child_form_id(parent, 11);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);
		ring_start_time_temp = ring_start_time_temp % 100;
		ring_start_time_temp += (lv_roller_get_selected(obj_roller)) * 100;
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 22);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);
		ring_start_time_temp = ring_start_time_temp / 100;
		ring_start_time_temp = ring_start_time_temp * 100;
		ring_start_time_temp += lv_roller_get_selected(obj_roller);
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 33);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);
		ring_end_time_temp = ring_end_time_temp % 100;
		ring_end_time_temp += lv_roller_get_selected(obj_roller) * 100;
		return;
	}

	obj_roller = lv_obj_get_child_form_id(parent, 44);
	if (obj_roller == rooler)
	{
		lv_signal_send(obj_roller, LV_SIGNAL_CONTROL, &down_char);
		ring_end_time_temp = ring_end_time_temp / 100;
		ring_end_time_temp = ring_end_time_temp * 100;
		ring_end_time_temp += lv_roller_get_selected(obj_roller);
		return;
	}
}

static lv_obj_t *ring_time_roller_create(lv_obj_t *parent, int x, int y, int w, char *opt, btn_data *btn_data2, btn_data *btn_data3)
{
	lv_obj_t *rooler = lv_roller_create(parent, NULL);
	lv_obj_set_style_local_text_line_space(rooler, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 20);
	lv_obj_set_style_local_text_color(rooler, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x80, 0x80, 0x80));

	lv_obj_set_style_local_bg_opa(rooler, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_text_color(rooler, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, lv_color_make(0xF8, 0xCD, 0xA5));
	lv_obj_set_style_local_text_font(rooler, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_roller_set_options(rooler, opt, LV_ROLLER_MODE_INFINITE);
	lv_roller_set_visible_row_count(rooler, 3);
	lv_obj_set_pos(rooler, x, y);
	lv_obj_set_width(rooler, w);
	static btn_data btn_data = btn_data_anything_create(ring_time_roller_event);
	btn_data2->OPS_DOWN = ring_time_roller_btn_syn_prev;
	btn_data3->OPS_DOWN = ring_time_roller_btn_syn_next;

	rooler->user_data = &btn_data;
	btn_touch_event_listen(rooler);

	lv_obj_t *up_btn = lv_btn_create(parent, NULL);
	lv_obj_set_style_local_bg_opa(up_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(up_btn, x, y - 55);
	lv_obj_set_size(up_btn, w, 52);
	up_btn->user_data = btn_data2;
	btn_data2->user_data = rooler;
	btn_touch_event_listen(up_btn);

	lv_obj_t *down_btn = lv_btn_create(parent, NULL);
	lv_obj_set_style_local_bg_opa(down_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(down_btn, x, y + 135);
	lv_obj_set_size(down_btn, w, 52);
	down_btn->user_data = btn_data3;
	btn_data3->user_data = rooler;
	btn_touch_event_listen(down_btn);
	return rooler;
}

static void start_time_hour_roller_create(lv_obj_t *parent)
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
	lv_obj_t *obj = ring_time_roller_create(parent, 72, 140, 80, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 11);

	int id = ring_temp.timer_start / 100;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void start_time_min_roller_create(lv_obj_t *parent)
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
	lv_obj_t *obj = ring_time_roller_create(parent, 168, 140, 80, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 22);

	int id = ring_temp.timer_start % 100;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void end_time_hour_roller_create(lv_obj_t *parent)
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
	lv_obj_t *obj = ring_time_roller_create(parent, 417, 140, 80, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 33);

	int id = ring_temp.timer_end / 100;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void end_time_min_roller_create(lv_obj_t *parent)
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
	lv_obj_t *obj = ring_time_roller_create(parent, 513, 140, 80, opt, &btn_data2, &btn_data3);
	lv_obj_set_id(obj, 44);

	int id = ring_temp.timer_end % 100;
	lv_roller_set_selected(obj, id, LV_ANIM_OFF);
}

static void ring_time_set_ok_btn_up(lv_obj_t *obj)
{

	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
	ring_temp.timer_start = ring_start_time_temp;
	ring_temp.timer_end = ring_end_time_temp;
	static char str1[24] = {0};
	sprintf(str1, "%02d:%02d - %02d:%02d", ring_temp.timer_start / 100, ring_temp.timer_start % 100, ring_temp.timer_end / 100, ring_temp.timer_end % 100);

	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 1);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void ring_time_set_cancel_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
}

static void sys_time_setting_window_create(void)
{

	ring_start_time_temp = ring_temp.timer_start;
	ring_end_time_temp = ring_temp.timer_end;

	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	lv_obj_set_id(window_cont, 888);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_2_PNG);
	lv_obj_t *window_img = lv_img_create(window_cont, NULL);
	lv_obj_set_pos(window_img, 228, 103);
	lv_obj_set_size(window_img, 648, 441);
	lv_img_set_src(window_img, &info);
	lv_obj_set_id(window_img, 666);

	lv_obj_t *window_head_label = lv_label_create(window_img, NULL);
	lv_obj_set_pos(window_head_label, 300, 20);
	lv_obj_set_size(window_head_label, 48, 40);
	lv_label_set_text(window_head_label, text_str(STR_SCHEDULE));
	lv_label_set_align(window_head_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	start_time_hour_roller_create(window_img);
	start_time_min_roller_create(window_img);
	end_time_hour_roller_create(window_img);
	end_time_min_roller_create(window_img);

	lv_obj_t *window_ok_btn = lv_btn_create(window_img, NULL);
	lv_obj_set_pos(window_ok_btn, 0, 360);
	lv_obj_set_size(window_ok_btn, 320, 77);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static btn_data btn_data1 = btn_data_create(NULL, ring_time_set_ok_btn_up, NULL);

	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	lv_obj_t *window_cancel_btn = lv_btn_create(window_img, window_ok_btn);
	lv_obj_set_x(window_cancel_btn, 324);

	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CANCEL));
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CANCEL));
	static btn_data btn_data2 = btn_data_create(NULL, ring_time_set_cancel_btn_up, NULL);
	window_cancel_btn->user_data = &btn_data2;
	btn_touch_event_listen(window_cancel_btn);
}

static void ring_schedule_btn_up(lv_obj_t *obj)
{
	ring_set_btn_syn_up(obj);
	sys_time_setting_window_create();
}
static void ring_schedule_btn_up_1(lv_obj_t *obj)
{
	sys_time_setting_window_create();
}

static void ring_schedule_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data2 = btn_data_create(ring_set_btn_syn_down, ring_schedule_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, ring_schedule_btn_up_1, NULL);
	btn_data2.OPS_ANYTHING = ring_set_btn_syn_event;
	static char str1[24] = {0};
	sprintf(str1, "%02d:%02d - %02d:%02d", ring_temp.timer_start / 100, ring_temp.timer_start % 100, ring_temp.timer_end / 100, ring_temp.timer_end % 100);

	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_SCHEDULE), &btn_data3, NULL, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, 1);
}

static void ring_time_set_left_btn_up(lv_obj_t *obj)
{
	ring_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 2);
	static char str1[32] = {0};
	if (--ring_temp.ring_time < RING_TIME_MIN)
	{
		ring_temp.ring_time = RING_TIME_MAX;
	}
	sprintf(str1, "%d %s", ring_temp.ring_time, text_str(STR_S));

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}
static void ring_time_set_right_btn_up(lv_obj_t *obj)
{
	ring_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 2);
	static char str1[32] = {0};
	if (++ring_temp.ring_time > RING_TIME_MAX)
	{
		ring_temp.ring_time = RING_TIME_MIN;
	}
	sprintf(str1, "%d %s", ring_temp.ring_time, text_str(STR_S));

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void ring_time_set_btn_create(Controls_location coordinate)
{

	static btn_data btn_data1 = btn_data_create(ring_set_btn_syn_down, ring_time_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(ring_set_btn_syn_down, ring_time_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = ring_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = ring_set_btn_syn_event;
	static char str1[32] = {0};

	sprintf(str1, "%d %s", ring_temp.ring_time, text_str(STR_S));

	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_RING_TIME), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, 2);
}

static void ring_ring_mode_set_btn_switch(void)
{
	lv_obj_t *btn1 = lv_obj_get_child_form_id(lv_scr_act(), 4);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 3);
	static char str1[16] = {0};
	if ((ring_temp.ring_mode = !ring_temp.ring_mode))
	{
		custom_ring_total = media_file_total_get(custom_ring_type, 0);
		printf("custom_ring_total ====>>%d\n\r", custom_ring_total);
		if (custom_ring_total)
		{
			if (ring_temp.custom_ring > custom_ring_total)
			{
				ring_temp.custom_ring = 0;
			}
			media_info *info = media_info_get(custom_ring_type, ring_temp.custom_ring);
			sprintf(str1, "%s", info->file_name);
		}
		else
		{
			bzero(str1, sizeof(str1));
		}
	}
	else
	{
		bzero(str1, sizeof(str1));
		sprintf(str1, "%d", ring_temp.ring);
	}
	lv_obj_set_style_local_value_str(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, ring_temp.ring_mode ? text_str(STR_CUSTOMIZED) : text_str(STR_STANDARD));
}

static void ring_ring_mode_set_btn_up(lv_obj_t *obj)
{
	ring_set_btn_syn_up(obj);
	if (is_sdcard_insert())
	{
		ring_ring_mode_set_btn_switch();
	}
}

static void ring_ring_mode_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data1 = btn_data_create(ring_set_btn_syn_down, ring_ring_mode_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(ring_set_btn_syn_down, ring_ring_mode_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = ring_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = ring_set_btn_syn_event;
	ring_temp.ring_mode = is_sdcard_insert() ? ring_temp.ring_mode : false;
	lv_obj_t *btn = sys_setting_btn_create(coordinate, ring_temp.ring_mode ? text_str(STR_CUSTOMIZED) : text_str(STR_STANDARD), text_str(STR_RING_MODE), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 3);
}

static void ring_ring_select_set_left_btn_up(lv_obj_t *obj)
{
	ring_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 4);
	static char str1[8] = {0};
	if (ring_temp.ring_mode)
	{
		custom_ring_total = media_file_total_get(custom_ring_type, 0);
		printf("custom_ring_total %d========>>>\n\r", custom_ring_total);
		if (custom_ring_total)
		{
			if (--ring_temp.custom_ring < 0)
			{
				ring_temp.custom_ring = custom_ring_total - 1;
			}
			media_info *info = media_info_get(custom_ring_type, ring_temp.custom_ring);
			sprintf(str1, "%s", info->file_name);
			printf("info->file_name %d========>>>%s\n\r", ring_temp.custom_ring, info->file_name);
			custom_music_play(info->file_name, ring_temp.ring_val, false, NULL, NULL);
		}
		else
			bzero(str1, sizeof(str1));
	}
	else
	{
		if (--ring_temp.ring < 1)
		{
			ring_temp.ring = door_ring_num;
		}
		sprintf(str1, "%d", ring_temp.ring);
		door_ring_play(ring_temp.ring, ring_temp.ring_val ? ring_temp.ring_val : 0, false, NULL, NULL);
	}
	// printf("AAAAAAAAAAAAAAAAAAAAAAAA:%s     %p\n",str1,btn);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void ring_ring_select_set_right_btn_up(lv_obj_t *obj)
{
	printf("%s==============\n\r", __func__);
	ring_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 4);
	static char str1[32] = {0};
	if (ring_temp.ring_mode)
	{
		custom_ring_total = media_file_total_get(custom_ring_type, 0);

		printf("custom_ring_total %d========>>>%d\n\r", custom_ring_total, ring_temp.custom_ring);
		if (custom_ring_total)
		{
			if (++ring_temp.custom_ring >= custom_ring_total)
			{
				ring_temp.custom_ring = 0;
			}
			media_info *info = media_info_get(custom_ring_type, ring_temp.custom_ring);
			bzero(str1, sizeof(str1));
			sprintf(str1, "%s", info->file_name);
			printf("info->file_name %d========>>>%s\n\r", ring_temp.custom_ring, info->file_name);
			custom_music_play(info->file_name, ring_temp.ring_val, false, NULL, NULL);
		}
		else
			bzero(str1, sizeof(str1));
	}
	else
	{
		if (++ring_temp.ring > door_ring_num)
		{
			ring_temp.ring = 1;
		}
		sprintf(str1, "%d", ring_temp.ring);
		door_ring_play(ring_temp.ring, ring_temp.ring_val ? ring_temp.ring_val : 0, false, NULL, NULL);
	}

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void ring_ring_select_set_btn_create(Controls_location coordinate)
{

	static btn_data btn_data1 = btn_data_create(ring_set_btn_syn_down, ring_ring_select_set_left_btn_up, NULL);
	btn_data1.obj_tone = false;
	static btn_data btn_data2 = btn_data_create(ring_set_btn_syn_down, ring_ring_select_set_right_btn_up, NULL);
	btn_data2.obj_tone = false;
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = ring_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = ring_set_btn_syn_event;
	static char str1[32] = {0};
	if (ring_temp.ring_mode)
	{
		custom_ring_total = media_file_total_get(custom_ring_type, 0);
		printf("custom_ring_total ====>>%d\n\r", custom_ring_total);
		if (custom_ring_total)
		{
			if (ring_temp.custom_ring > custom_ring_total)
			{
				ring_temp.custom_ring = 0;
			}
			media_info *info = media_info_get(custom_ring_type, ring_temp.custom_ring);
			sprintf(str1, "%s", info->file_name);
		}
		else
		{
			bzero(str1, sizeof(str1));
		}
	}
	else
		sprintf(str1, "%d", ring_temp.ring);

	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_RING_SELECT), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 4);
}

static void ring_ring_volume_set_left_btn_up(lv_obj_t *obj)
{
	ring_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 5);
	static char str1[8] = {0};
	if (--ring_temp.ring_val < 1)
	{
		ring_temp.ring_val = 10;
	}
	sprintf(str1, "%d", ring_temp.ring_val);
	if (!is_audio_play_ing())
	{
		if (ring_temp.ring_mode)
		{
			custom_ring_total = media_file_total_get(custom_ring_type, 0);

			printf("custom_ring_total %d========>>>%d\n\r", custom_ring_total, ring_temp.custom_ring);
			if (custom_ring_total)
			{
				media_info *info = media_info_get(custom_ring_type, ring_temp.custom_ring);
				printf("info->file_name %d========>>>%s\n\r", ring_temp.custom_ring, info->file_name);
				custom_music_play(info->file_name, ring_temp.ring_val, false, NULL, NULL);
			}
		}
		else
		{
			printf("door_ring_play %d========>>>%d\n\r", door_ring_num, ring_temp.ring);
			door_ring_play(ring_temp.ring, ring_temp.ring_val ? ring_temp.ring_val : 0, false, NULL, NULL);
		}
	}
	else
	{
		audio_play_volume_set(ring_temp.ring_val);
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}
static void ring_ring_volume_set_right_btn_up(lv_obj_t *obj)
{
	ring_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 5);
	static char str1[8] = {0};
	if (++ring_temp.ring_val > 10)
	{
		ring_temp.ring_val = 1;
	}
	sprintf(str1, "%d", ring_temp.ring_val);
	if (!is_audio_play_ing())
	{
		if (ring_temp.ring_mode)
		{
			custom_ring_total = media_file_total_get(custom_ring_type, 0);

			printf("custom_ring_total %d========>>>%d\n\r", custom_ring_total, ring_temp.custom_ring);
			if (custom_ring_total)
			{
				media_info *info = media_info_get(custom_ring_type, ring_temp.custom_ring);
				printf("info->file_name %d========>>>%s\n\r", ring_temp.custom_ring, info->file_name);
				custom_music_play(info->file_name, ring_temp.ring_val, false, NULL, NULL);
			}
		}
		else
		{
			printf("door_ring_play %d========>>>%d\n\r", door_ring_num, ring_temp.ring);
			door_ring_play(ring_temp.ring, ring_temp.ring_val ? ring_temp.ring_val : 0, false, NULL, NULL);
		}
	}
	else
	{
		audio_play_volume_set(ring_temp.ring_val);
	}
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void ring_ring_volume_set_btn_create(Controls_location coordinate)
{

	static btn_data btn_data1 = btn_data_create(ring_set_btn_syn_down, ring_ring_volume_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(ring_set_btn_syn_down, ring_ring_volume_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.obj_tone = btn_data2.obj_tone = false;
	btn_data1.OPS_ANYTHING = ring_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = ring_set_btn_syn_event;

	static char str1[8] = {0};
	sprintf(str1, "%d", ring_temp.ring_val);
	lv_obj_t *btn = sys_setting_btn_create(coordinate, str1, text_str(STR_RING_VOLUME), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 5);
}

static void ring_setting_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_door));
}

static void ring_setting_display(void)
{
	Controls_location module_coordinate[] = RING_MODULE_COORDINATE_INIT;
	ring_temp_init();
	ring_schedule_set_btn_create(module_coordinate[RING_SCHEDULE_MODULE]);
	ring_time_set_btn_create(module_coordinate[RING_TIME_MODULE]);
	ring_ring_mode_set_btn_create(module_coordinate[RING_MODE_MODULE]);
	ring_ring_select_set_btn_create(module_coordinate[RING_SELECT_MODULE]);
	ring_ring_volume_set_btn_create(module_coordinate[RING_VOLUME_MODULE]);
	ring_setting_btn_text_create();
	home_back_btn_create(ring_setting_back_btn_up, NULL);
}

static void setting_ring_sdcard_callback(unsigned long arg1, unsigned long arg2)
{
	if (arg2 == 0x00)
	{
		if (sdcard_insert_msg_box == NULL)
		{
			sdcard_insert_msg_box = sdcard_insert_msgbox_create((bool)arg1 ? text_str(STR_INSET_SD_SUCCEE) : text_str(STR_NO_SD_CARD));
		}
		else
		{
			lv_label_set_text((lv_obj_t *)sdcard_insert_msg_box->user_data, (bool)arg1 ? text_str(STR_INSET_SD_SUCCEE) : text_str(STR_NO_SD_CARD));
			lv_obj_align(sdcard_insert_msg_box->user_data, ((lv_obj_t *)sdcard_insert_msg_box->user_data)->user_data, LV_ALIGN_CENTER, 0, -30);
		}

		if (ring_temp.ring_mode)
		{
			ring_ring_mode_set_btn_switch();
		}
	}
	else
	{
		if (sdcard_insert_msg_box == NULL)
		{
			sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_PLEASE_FORMAT_SD));
		}
		else
		{
			lv_label_set_text((lv_obj_t *)sdcard_insert_msg_box->user_data, text_str(STR_PLEASE_FORMAT_SD));
			lv_obj_align(sdcard_insert_msg_box->user_data, ((lv_obj_t *)sdcard_insert_msg_box->user_data)->user_data, LV_ALIGN_CENTER, 0, -30);
		}
	}
}

static void LAYOUT_ENETER_FUNC(setting_ring)
{
	set_ring_flag = 0;
	door_ring_fine();
	setting_bg_display();
	ring_setting_display();
	sdcard_event_register(setting_ring_sdcard_callback);
}

static void LAYOUT_QUIT_FUNC(setting_ring)
{
	audio_play_stop_set();
	if (sdcard_insert_msg_box != NULL)
	{
		lv_obj_del(sdcard_insert_msg_box);
		sdcard_insert_msg_box = NULL;
	}
	sdcard_event_register(NULL);
	ring_temp_save();
	user_data_save();
	ring_init();
	// Debug("+++++++++++++++++++++++++++++++\n\r");
}

CREATE_LAYOUT(setting_ring);
