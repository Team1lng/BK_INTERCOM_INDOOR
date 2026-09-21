#include "layout_define.h"
#include "leo_api.h"
#include "tcp_network_cmd.h"
#include "numeric_keypad.h"

#define LIST_DISPLAY_MAX 50																																		// 一个列表最多刷新50张卡
#define LIST_PAD_VER_SIZE 0																																		// 列表垂直填充大小
#define LIST_BTN_PAD_VER_SIZE 0																																	// 列表按鍵垂直填充大小
#define LIST_BTN_HEIGHT 52																																		// 列表按键高度
#define LIST_ONCE_DISPLAY_MAX 8																																	// 列表一次显示最多
#define LIST_HEIGHT (LIST_BTN_HEIGHT * LIST_ONCE_DISPLAY_MAX)																									// 列表高度
#define ROLL_LENGTH_MAX (LIST_PAD_VER_SIZE * 2 + LIST_BTN_HEIGHT * (LIST_DISPLAY_MAX - LIST_ONCE_DISPLAY_MAX) + (LIST_BTN_PAD_VER_SIZE * 2) * LIST_DISPLAY_MAX) // 最大滚动长度（滚到底部）

static bool is_data_list_move = false;
static lv_obj_t *data_list = NULL;
static lv_obj_t *lock_setting_error_msg_box = NULL;
static tcp_device set_door_id = TCP_DEVICE_OUTDOOR_1;
static int curr_list_num = 0;
static int select_index = 0;
static int list_num_max = 0;
static int roll_length = 0;
static int valid_info_index[DATA_NUM_MAX] = {0};
data_info_t data_info = {0};

static void data_list_display(bool tcp_creat);
static void data_list_display_init(void);
static lv_obj_t *lock_setting_error_msgbox_create(char *str);
static void tcp_network_data_handler_callback(unsigned long arg1, unsigned long arg2, unsigned long arg3);

extern lv_obj_t *list_window_btn_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src1, const void *img_src2, char *string);

tcp_device curr_tcp_dev_id_get(void)
{
	return set_door_id;
}

int select_index_get(void)
{
	return select_index;
}
void select_index_set(int index)
{
	select_index = index;
}
int data_total_get(void)
{
	return data_info.total;
}
void curr_list_num_set(int list_num)
{
	curr_list_num = list_num;
}
int curr_list_num_get(void)
{
	return curr_list_num;
}
int valid_index_get_from_id(int obj_id)
{
	return valid_info_index[(curr_list_num_get() - 1) * LIST_DISPLAY_MAX + obj_id];
}

bool lock1_state_get(int index)
{
	return (bool)(data_info.data[index].lock_state & 0x01);
}
void lock1_state_set(int index, bool state)
{
	if (state)
	{
		data_info.data[index].lock_state |= 0x01;
	}
	else
	{
		data_info.data[index].lock_state &= (~0x01);
	}
	tcp_network_cmd_set_lock_type_send(index, data_info.data[index].lock_state, set_door_id);
}

bool lock2_state_get(int index)
{
	return (bool)(data_info.data[index].lock_state & 0x02);
}
void lock2_state_set(int index, bool state)
{
	if (state)
	{
		data_info.data[index].lock_state |= 0x02;
	}
	else
	{
		data_info.data[index].lock_state &= (~0x02);
	}
	tcp_network_cmd_set_lock_type_send(index, data_info.data[index].lock_state, set_door_id);
}

/* 参数 type :
**0 => 默认位置显示
**1 => 从顶部开始显示
**2 => 从底部开始显示
*/
void list_btn_display_set(int type)
{
	if (type == 1)
	{
		lv_obj_set_y(lv_page_get_scrollable(data_list), 0);
	}
	else if (type == 2)
	{
		lv_obj_set_y(lv_page_get_scrollable(data_list), -ROLL_LENGTH_MAX);
	}
	else
	{
		lv_obj_set_y(lv_page_get_scrollable(data_list), roll_length);
		roll_length = 0;
	}
}

static void set_door1_flag_up(lv_obj_t *obj)
{

	if (get_outdoor_finerger_status(DEVICE_OUTDOOR_1) == false && data_manage_type_get() == MANAGE_FINGER)
	{
		return;
	}

	if (set_door_id != TCP_DEVICE_OUTDOOR_1)
	{
		select_index_set(0);
		set_door_id = TCP_DEVICE_OUTDOOR_1;
		lv_obj_clean(lv_scr_act());
		data_list_display(true);
	}
}
static void set_door2_flag_up(lv_obj_t *obj)
{

	if (get_outdoor_finerger_status(DEVICE_OUTDOOR_2) == false && data_manage_type_get() == MANAGE_FINGER)
	{
		return;
	}

	if (set_door_id != TCP_DEVICE_OUTDOOR_2)
	{
		select_index_set(0);
		set_door_id = TCP_DEVICE_OUTDOOR_2;
		lv_obj_clean(lv_scr_act());
		data_list_display(true);
	}
}

static void data_list_door_setting_btn_text_create(void)
{
	bool full_support = true;
	Debug("data_manage_type_get:%d\n", data_manage_type_get());

	if ((data_manage_type_get() == MANAGE_FINGER) && (get_outdoor_finerger_status(DEVICE_OUTDOOR_1) == false || get_outdoor_finerger_status(DEVICE_OUTDOOR_2) == false))
	{
		full_support = false;
	}

	if ((data_manage_type_get() == MANAGE_FINGER) && get_outdoor_finerger_status(DEVICE_OUTDOOR_1) == false)
	{
	}
	else

	{
		lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
		if (full_support)
		{
			lv_obj_set_pos(btn, 58, 162);
		}
		else
		{
			lv_obj_set_pos(btn, 58, 240);
		}

		lv_obj_set_size(btn, 66, 66);
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_DOOR1_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_DOOR1_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, set_door_id == TCP_DEVICE_OUTDOOR_1 ? &info : &info1);

		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_DOOR1));
		if (set_door_id)
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
	}

	if ((data_manage_type_get() == MANAGE_FINGER) && get_outdoor_finerger_status(DEVICE_OUTDOOR_2) == false)
	{
	}
	else

	{
		lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
		if (full_support)
		{
			lv_obj_set_pos(btn1, 58, 321);
		}
		else
		{
			lv_obj_set_pos(btn1, 58, 240);
		}

		lv_obj_set_size(btn1, 66, 66);
		lv_obj_set_style_local_bg_opa(btn1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_opa(btn1, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

		static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_DOOR2_FOCUS_PNG);
		static rom_bin_info info3 = rom_bin_info_get(ROM_RES_SETTING_DOOR2_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, set_door_id == TCP_DEVICE_OUTDOOR_2 ? &info2 : &info3);

		lv_obj_set_style_local_value_str(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_DOOR2));
		if (!set_door_id)
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
	}
}

static void data_list_img_text_display(void)
{
	lv_obj_t *line1 = lv_line_create(lv_scr_act(), NULL);
	static lv_point_t point[2] = {{199, 128}, {886, 128}};
	lv_line_set_points(line1, point, 2);
	lv_obj_set_style_local_line_color(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LIST_BORDER_COLOR);
	lv_obj_set_style_local_line_opa(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_line_width(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 2);

	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_pos(label, 206, 29);
	if (data_manage_type_get() == MANAGE_CARD)
	{
		lv_label_set_text(label, set_door_id == TCP_DEVICE_OUTDOOR_1 ? text_str(STR_DOOR1_CARD_MANAGE) : text_str(STR_DOOR2_CARD_MANAGE));
	}
	else if (data_manage_type_get() == MANAGE_FINGER)
	{
		lv_label_set_text(label, set_door_id == TCP_DEVICE_OUTDOOR_1 ? text_str(STR_DOOR1_FINGER_MANAGE) : text_str(STR_DOOR2_FINGER_MANAGE));
	}
	else if (data_manage_type_get() == MANAGE_PASSW)
	{
		lv_label_set_text(label, set_door_id == TCP_DEVICE_OUTDOOR_1 ? text_str(STR_DOOR1_PASSW_MANAGE) : text_str(STR_DOOR2_PASSW_MANAGE));
	}
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	label = lv_label_create(lv_scr_act(), label);
	lv_label_set_text(label, text_str(STR_NO_NUM));
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 250 - lv_obj_get_width(label), 91);

	label = lv_label_create(lv_scr_act(), label);
	if (data_manage_type_get() == MANAGE_CARD)
	{
		lv_label_set_text(label, text_str(STR_CARD_NUMBER));
	}
	else if (data_manage_type_get() == MANAGE_FINGER)
	{
		lv_label_set_text(label, text_str(STR_FINGER_NUMBER));
	}
	else if (data_manage_type_get() == MANAGE_PASSW)
	{
		lv_label_set_text(label, text_str(STR_PASSWORD_NUMBER));
	}
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 450 - lv_obj_get_width(label), 91);

	label = lv_label_create(lv_scr_act(), label);
	lv_label_set_text(label, text_str(STR_LOCK));
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 755 - lv_obj_get_width(label), 91);

	label = lv_label_create(lv_scr_act(), label);
	lv_label_set_text(label, text_str(STR_GATE1));
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 855 - lv_obj_get_width(label), 91);
}

static void window_delete_file_btn_up(lv_obj_t *obj)
{
	printf("=================<>>>> delete data index : [%d]\n", valid_info_index[select_index_get()]);
	tcp_network_cmd_del_data_send(valid_info_index[select_index_get()], set_door_id);
	int list_index = select_index_get();
	int valid_index = valid_info_index[list_index];
	data_info.data[valid_index].lock_state = 0;
	memset(&(data_info.data[valid_index].number), 0, sizeof(data_info.data[valid_index].number));
	data_info.total--;

	roll_length = lv_obj_get_y(lv_page_get_scrollable(data_list));

	if (list_index == data_total_get())
	{
		if (list_index != 0 && list_index % LIST_DISPLAY_MAX == 0)
			roll_length = -ROLL_LENGTH_MAX;
		select_index_set(list_index - 1);
	}
	lv_obj_del(obj->parent->parent);
	lv_list_clean(data_list);
	data_list_display_init();
}
static void window_delete_file_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_delete_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);

	list_window_btn_create(parent, 43, 25, 217, 48, &btn_data, &info1, &info, text_str(STR_DELETE));
}
static void window_delete_all_file_btn_up(lv_obj_t *obj)
{
	tcp_network_cmd_del_data_send(200, set_door_id);
	for (int i = 0; i < DATA_NUM_MAX; i++)
	{
		data_info.data[i].lock_state = 0;
	}
	data_info.total = 0;
	select_index_set(0);
	lv_obj_del(obj->parent->parent);
	lv_list_clean(data_list);
	data_list_display_init();
}

static void window_delete_all_file_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_delete_all_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);

	list_window_btn_create(parent, 43, 97, 217, 48, &btn_data, &info1, &info, text_str(STR_ALL_DELETE));
}
static void window_close_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window = lv_obj_get_child_form_id(lv_scr_act(), 8888);
	if (window != NULL)
	{
		lv_obj_del(window);
	}
}

static void window_close_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_close_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	list_window_btn_create(parent, 43, 169, 217, 48, &btn_data, &info1, &info, text_str(STR_CLOSE));
}

static void data_list_window_create(void)
{
	lv_obj_t *window = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_style_local_bg_opa(window, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window, 0, 0);
	lv_obj_set_size(window, 1024, 600);
	lv_obj_set_id(window, 8888);

	lv_obj_t *window_cont = lv_cont_create(window, NULL);
	lv_obj_set_style_local_bg_color(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00131D));
	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
	lv_obj_set_style_local_radius(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);
	set_location(window_cont, 391, 163, 304, 242);
	// lv_cont_set_layout(window_cont, LV_FIT_NONE);
	lv_obj_set_id(window_cont, 888);

	window_delete_file_btn_create(window_cont);
	window_delete_all_file_btn_create(window_cont);
	window_close_btn_create(window_cont);
}

static void data_list_btn_down(lv_obj_t *obj)
{
	lv_obj_t *btn = lv_obj_get_child(obj, NULL);
	lv_obj_set_state(btn, LV_STATE_PRESSED);
}

static void data_list_btn_up(lv_obj_t *obj)
{
	lv_obj_t *btn = lv_obj_get_child(obj, NULL);
	lv_obj_set_state(btn, LV_STATE_DEFAULT);

	if (is_data_list_move == true)
	{
		return;
	}

	select_index_set((curr_list_num_get() - 1) * LIST_DISPLAY_MAX + obj->obj_id);
	data_list_window_create();
}

static void data_list_btn_anything(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_PRESS_LOST == event)
	{
		lv_obj_t *btn = lv_obj_get_child(obj, NULL);
		lv_obj_set_state(btn, LV_STATE_DEFAULT);
	}
}

static void data_list_open_lock_state_enable(lv_obj_t *obj, bool en)
{

	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ENABLE_LOCK_PNG);
	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_DISABLE_LOCK_PNG);
	if (en)
	{
		lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
	}
	else
	{
		lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
	}
}

static void data_list_open_lock1_btn_up(lv_obj_t *obj)
{
	lv_obj_t *btn = obj->parent;
	unsigned int id = btn->obj_id;
	int index = valid_index_get_from_id(id);
	bool state = !lock1_state_get(index);
	if (state == false && lock2_state_get(index) == false)
	{
		if (lock_setting_error_msg_box == NULL)
		{
			lock_setting_error_msg_box = lock_setting_error_msgbox_create(text_str(STR_ILLEGAL_OPERATION));
		}
		return;
	}
	data_list_open_lock_state_enable(obj, state);
	lock1_state_set(index, state);
}

static void data_list_open_lock2_btn_up(lv_obj_t *obj)
{
	lv_obj_t *btn = obj->parent;
	unsigned int id = btn->obj_id;
	int index = valid_index_get_from_id(id);
	bool state = !lock2_state_get(index);
	if (state == false && lock1_state_get(index) == false)
	{
		if (lock_setting_error_msg_box == NULL)
		{
			lock_setting_error_msg_box = lock_setting_error_msgbox_create(text_str(STR_ILLEGAL_OPERATION));
		}
		return;
	}
	data_list_open_lock_state_enable(obj, state);
	lock2_state_set(index, state);
}

static lv_obj_t *data_list_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int i)
{
	lv_obj_t *list_btn = lv_list_add_btn(parent, NULL, NULL);
	if (i >= 0)
		lv_obj_set_id(list_btn, i);
	lv_obj_set_style_local_pad_ver(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LIST_BTN_PAD_VER_SIZE);
	lv_obj_set_style_local_border_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_border_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
	lv_obj_set_style_local_border_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	lv_obj_t *cont = lv_cont_create(list_btn, NULL);
	if (i >= 0)
		lv_obj_set_id(cont, i);
	lv_obj_set_click(cont, false);
	lv_obj_set_size(cont, w, h);

	if (i < 0)
		return list_btn;
	int index = valid_index_get_from_id(i);

	lv_obj_set_style_local_bg_opa(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x80, 0x0, 0x10));

	lv_obj_set_style_local_border_side(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LIST_BORDER_COLOR);
	lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LIST_BORDER_COLOR);
	lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LIST_BORDER_COLOR);

	lv_obj_t *serial_num_label = lv_label_create(cont, NULL);
	lv_label_set_text_fmt(serial_num_label, "%d", index);
	lv_label_set_align(serial_num_label, LV_LABEL_ALIGN_RIGHT);
	lv_obj_align(serial_num_label, cont, LV_ALIGN_IN_LEFT_MID, 17, 0);

	lv_obj_t *number_label = lv_label_create(cont, NULL);
	if (data_manage_type_get() == MANAGE_PASSW)
	{
		lv_label_set_text_fmt(number_label, "%d%d%d%d", data_info.data[index].number[0],
							  data_info.data[index].number[1],
							  data_info.data[index].number[2],
							  data_info.data[index].number[3]);
	}
	else
	{
		lv_label_set_text_fmt(number_label, "%02x%02x%02x%02x%02x", data_info.data[index].number[0],
							  data_info.data[index].number[1],
							  data_info.data[index].number[2],
							  data_info.data[index].number[3],
							  data_info.data[index].number[4]);
	}

	lv_obj_align(number_label, cont, LV_ALIGN_IN_LEFT_MID, 149, 0);

	lv_obj_t *lock1_icon_obj = lv_obj_create(cont, NULL);
	lv_obj_set_size(lock1_icon_obj, 46, 40);
	data_list_open_lock_state_enable(lock1_icon_obj, lock1_state_get(index));
	lv_obj_align(lock1_icon_obj, NULL, LV_ALIGN_IN_LEFT_MID, 480, 0);
	static btn_data lock1_btn_data = btn_data_up_create(data_list_open_lock1_btn_up);
	lock1_icon_obj->user_data = &lock1_btn_data;
	btn_touch_event_listen(lock1_icon_obj);

	lv_obj_t *lock2_icon_obj = lv_obj_create(cont, lock1_icon_obj);
	data_list_open_lock_state_enable(lock2_icon_obj, lock2_state_get(index));
	lv_obj_align(lock2_icon_obj, NULL, LV_ALIGN_IN_LEFT_MID, 580, 0);
	static btn_data lock2_btn_data = btn_data_up_create(data_list_open_lock2_btn_up);
	lock2_icon_obj->user_data = &lock2_btn_data;
	btn_touch_event_listen(lock2_icon_obj);

	// 单独的回调函数
	static btn_data btn_data = btn_data_create(data_list_btn_down, data_list_btn_up, NULL);
	btn_data.OPS_ANYTHING = data_list_btn_anything;
	list_btn->user_data = &btn_data;
	btn_touch_event_listen(list_btn);

	return list_btn;
}

static void data_list_flush(void)
{
	lv_list_clean(data_list);

	int i = 0;
	for (; i < LIST_DISPLAY_MAX; i++)
	{
		if (valid_index_get_from_id(i) == -1)
		{
			break;
		}
		data_list_btn_create(data_list, 0, (i)*LIST_BTN_HEIGHT, 630, LIST_BTN_HEIGHT, i);
	}
	if (i < LIST_ONCE_DISPLAY_MAX)
	{
		data_list_btn_create(data_list, 0, (i)*LIST_BTN_HEIGHT, 630, LIST_BTN_HEIGHT * (LIST_ONCE_DISPLAY_MAX - i), -1);
		data_list_btn_create(data_list, 0, (i)*LIST_BTN_HEIGHT, 630, 10, -1);
	}
}

static void data_list_prev_page_flush(void)
{
	int list_num = curr_list_num_get();
	if (list_num <= 1)
	{
		return;
	}
	list_num--;
	curr_list_num_set(list_num);
	data_list_flush();
	list_btn_display_set(2);
}

static void data_list_next_page_flush(void)
{
	int list_num = curr_list_num_get();
	if (list_num >= list_num_max)
	{
		return;
	}
	list_num++;
	curr_list_num_set(list_num);
	data_list_flush();
	list_btn_display_set(1);
}

static void data_list_display_init(void)
{
	int total = data_total_get();
	printf("DATA_LIST_DISPLAY_INIT:%d\n", total);
	if (total == 0)
	{
		return;
	}

	int count = 0;
	int check = 0;
	int invalid_error_num = 0;

	for (int i = 0; i < DATA_NUM_MAX; i++)
	{
		check = data_info.data[i].number[0] | data_info.data[i].number[1] | data_info.data[i].number[2] | data_info.data[i].number[3] | data_info.data[i].number[4];
		if (data_info.data[i].lock_state != 0)
		{

			// Debug("=================================>>>>check:%d   invalid_error_num:%d  count:%d\n",check,invalid_error_num,count);
			valid_info_index[count] = i;
			++count;
			if (count >= total)
			{
				valid_info_index[count] = -1;
				break;
			}
		}
		else if (check != 0)
		{
			++invalid_error_num;

			// Debug("=================================>>>>check:%d   invalid_error_num:%d  count:%d\n",check,invalid_error_num,count);
			if ((invalid_error_num + count) >= total)
			{
				valid_info_index[count] = -1;
				break;
			}
		}
		else
		{
			Debug("=================================>>>>check:%d   invalid_error_num:%d  count:%d\n", check, invalid_error_num, count);
		}

		// else if(i == count)
		// {
		// 	valid_info_index[count] = -1;
		// 	break;
		// }
	}

	int index = select_index_get() + 1;
	int list_num = ((index == 0) ? 1 : (index / LIST_DISPLAY_MAX + ((index % LIST_DISPLAY_MAX) ? 1 : 0)));
	curr_list_num_set(list_num);

	printf("curr_list_num_set:%d     valid_info_index[%d]:%d\n", list_num, count, valid_info_index[count]);
	list_num_max = (total == 0) ? 1 : (total / LIST_DISPLAY_MAX + ((total % LIST_DISPLAY_MAX) ? 1 : 0));

	printf("list_num_max:%d\n", list_num_max);
	data_list_flush();

	if (select_index_get() > 0)
	{
		list_btn_display_set(0);
	}
}

static void data_list_touch_anything(lv_obj_t *obj, lv_event_t event)
{
	static int flag = 0;
	// printf("event：%d \n", event);
	if (LV_EVENT_DRAG_BEGIN == event)
	{
		is_data_list_move = true;
		if (lv_page_on_edge(obj, LV_PAGE_EDGE_BOTTOM))
		{
			flag = 2;
		}
		else if (lv_page_on_edge(obj, LV_PAGE_EDGE_TOP))
		{
			flag = 1;
		}
		else
		{
			flag = 0;
		}
	}
	else if (LV_EVENT_DRAG_END == event)
	{
		is_data_list_move = false;

		if (flag == 2 && lv_page_on_edge(obj, LV_PAGE_EDGE_BOTTOM))
		{
			data_list_next_page_flush();
		}
		else if (flag == 1 && lv_page_on_edge(obj, LV_PAGE_EDGE_TOP))
		{
			data_list_prev_page_flush();
		}
		flag = 0;
	}
}

static void data_list_page_create(void)
{
	// 创建页面
	data_list = lv_list_create(lv_scr_act(), NULL); // 在当前活跃的屏幕上创建页面
	lv_list_set_edge_flash(data_list, true);
	set_location(data_list, 199 /* 175 */, 128, 687 /* 740 */, LIST_HEIGHT);
	lv_obj_set_style_local_pad_ver(data_list, LV_LIST_PART_SCROLLABLE, LV_STATE_DEFAULT, LIST_PAD_VER_SIZE);
	lv_obj_set_style_local_pad_hor(data_list, LV_LIST_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(data_list, LV_LIST_PART_BG, LV_STATE_DEFAULT, LV_OPA_30);

	static btn_data page_data = btn_data_anything_create(data_list_touch_anything);
	data_list->user_data = &page_data;
	btn_touch_event_listen(data_list);
}

void add_management_btn_create(void (*back_btn_up)(lv_obj_t *), void (*back_btn_down)(lv_obj_t *))
{
	static btn_data btn_data;
	btn_data.OPS_DOWN = back_btn_down;
	btn_data.OPS_UP = back_btn_up;
	btn_data.OPS_ANYTHING = NULL;
	btn_data.user_data = NULL;
	btn_data.obj_tone = true;
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MANAGEMENT_ADD_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MANAGEMENT_ADD_FOCUS_PNG);
	Controls_location coordinate = {929, 402, 67, 67};
	home_btn_create_2(coordinate, &btn_data, &info, &info1);
}

void keyboard_confirm_cb(int password, int len)
{
	if (len < KEYBOARD_PASSWORD_LEN)
	{
		msgbox_animat_create(text_str(STR_PASSW_ENOUGH_BITS), 2000);
	}
	else
	{
		tcp_network_cmd_add_data_send(set_door_id, password >> 8, password & 0xFF);
	}
}

static void data_list_back_btn_up(lv_obj_t *obj)
{
	lv_obj_set_state(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(setting_admin));
}

static void data_list_add_btn_up(lv_obj_t *obj)
{
	lv_obj_set_state(obj, LV_STATE_DEFAULT);

	if (data_manage_type_get() == MANAGE_PASSW)
	{
		if (data_total_get() < 10)
		{
			init_numeric_keyboard(keyboard_confirm_cb);
		}
		else
		{
			msgbox_animat_create(text_str(STR_NUM_PASS_FULL), 2000);
		}
	}
	else
		goto_layout(pLAYOUT(add_data));
}

static void data_list_display(bool tcp_creat)
{
	if (tcp_creat && management_create_state(set_door_id) == false) // 线程未创建
	{
		if (device_online_state_get(set_door_id ? DEVICE_OUTDOOR_2 : DEVICE_OUTDOOR_1))
		{
			tcp_management_init(set_door_id);
		}
	}
	else
	{
		tcp_network_cmd_get_data_info_send(set_door_id);
	}

	data_list_door_setting_btn_text_create();
	data_list_img_text_display();
	data_list_page_create();
	home_back_btn_create(data_list_back_btn_up, NULL);
	add_management_btn_create(data_list_add_btn_up, NULL);
}

static void LAYOUT_ENETER_FUNC(data_list)
{

	if (data_manage_type_get() == MANAGE_FINGER)
	{
		if (get_outdoor_finerger_status(DEVICE_OUTDOOR_1) == false)
		{
			set_door_id = TCP_DEVICE_OUTDOOR_2;
		}

		if (get_outdoor_finerger_status(DEVICE_OUTDOOR_2) == false)
		{
			set_door_id = TCP_DEVICE_OUTDOOR_1;
		}
	}

	Debug("%s =================>>%d\n", __func__, set_door_id);
	tcp_network_event_register(tcp_network_data_handler_callback);
	setting_bg_display();

	bool creation = (prev_layout_get() != pLAYOUT(add_data)) ? true : false;
	data_list_display(creation);

	if (!device_online_state_get(set_door_id + DEVICE_OUTDOOR_1) && lock_setting_error_msg_box == NULL)
	{
		lock_setting_error_msg_box = lock_setting_error_msgbox_create(text_str(STR_DEVICE_OFFLINE));
	}
}

static void LAYOUT_QUIT_FUNC(data_list)
{
	if (target_layout != pLAYOUT(data_list))
		select_index_set(0);

	if (target_layout != pLAYOUT(data_list) && target_layout != pLAYOUT(add_data))
		tcp_management_close();

	if (lock_setting_error_msg_box != NULL)
	{
		lv_obj_del(lock_setting_error_msg_box);
		lock_setting_error_msg_box = NULL;
	}

	tcp_network_event_register(NULL);
}

CREATE_LAYOUT(data_list);

bool data_info_check(char *data)
{
	data_info_t *data_info = (data_info_t *)data;
	int total_check = 0;
	for (int i = 0; i < DATA_NUM_MAX; i++)
	{
		// printf("===============[%x][%x][%x][%x][%x][%x]================\n",
		// data_info->data[i].lock_state,
		// data_info->data[i].number[0],
		// data_info->data[i].number[1],
		// data_info->data[i].number[2],
		// data_info->data[i].number[3],
		// data_info->data[i].number[4]);
		if (total_check < data_info->total && data_info->data[i].lock_state != 0)
		{
			total_check++;
		}
	}
	printf("=====================total_check[%d] data_info->total[%d]======================\n", total_check, data_info->total);
	return total_check == data_info->total ? true : false;
}

void data_info_sync_handler(char *data)
{
	memset(&data_info, 0, sizeof(data_info));
	if (data_info_check(data))
	{
		memcpy(&data_info, data, sizeof(data_info_t));
		printf("=====================total:[%d]======================\n", data_info.total);
		// for (int i = 0; i < 8; i++)
		// {
		// 	printf("===============[%x][%x][%x][%x][%x][%x]================\n",
		// 	data_info.data[i].lock_state,
		// 	data_info.data[i].number[0],
		// 	data_info.data[i].number[1],
		// 	data_info.data[i].number[2],
		// 	data_info.data[i].number[3],
		// 	data_info.data[i].number[4]);
		// }
	}
	else
	{
		printf("=====================data info check error !!!!!!!!!!!!!!!!!!!!!!!!!===================\n");
	}
}

static void lock_setting_error_msg_btn_up(lv_obj_t *obj)
{
	if (lock_setting_error_msg_box != NULL)
	{
		lv_obj_del(lock_setting_error_msg_box);
		lock_setting_error_msg_box = NULL;
	}
}

static lv_obj_t *lock_setting_error_msgbox_create(char *str)
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
	btn_data1.OPS_UP = lock_setting_error_msg_btn_up;
	lv_obj_set_style_local_pattern_image(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
	// lv_obj_set_style_local_pattern_image(window_ok_btn,LV_OBJ_PART_MAIN,LV_STATE_FOCUSED,&info2);
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_align(window_ok_btn, msgbox_cont, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	return window_cont;
}

static void tcp_network_data_handler_callback(unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
	if (arg3 != set_door_id) // 返回信息与当前通讯设备不符即丢弃
	{
		Debug("return error dev:%lu message :0x%x\n", arg3, (unsigned int)arg1);
		return;
	}

	switch (arg1)
	{
	case NET_COMMON_CMD_DEL_FINGER:

		break;

	case NET_COMMON_CMD_GET_FINGER:

		data_info_sync_handler(data_packge_buffer_get());
		data_list_display_init();
		break;

	case NET_COMMON_CMD_DEL_CARD:
		break;

	case NET_COMMON_CMD_GET_CARD:

		data_info_sync_handler(data_packge_buffer_get());
		data_list_display_init();
		break;

	case NET_COMMON_CMD_GET_PASSW:
		data_info_sync_handler(data_packge_buffer_get());
		data_list_display_init();
		break;

	case NET_COMMON_CMD_ACCESS_DENIED:
		if (lock_setting_error_msg_box == NULL)
		{
			lock_setting_error_msg_box = lock_setting_error_msgbox_create(text_str(STR_EXISTING_EQUIPMENT_ONLINE));
		}
		break;

	default:
		break;
	}
}