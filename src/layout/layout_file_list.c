#include "layout_define.h"
#include "leo_api.h"
#include "tcp_network_cmd.h"

static media_type rec_file_type = FILE_TYPE_SD_CALL;
static char *cur_record_path = SD_CALL_PATH;
static enum delete_flag record_delete_type = DELETE_ALL_CALL;
static media_info *record_info = NULL;
static int record_total;
static int select_index = 1;
static bool is_data_list_move = false;
static lv_obj_t *data_list = NULL;
static int roll_length = 0;

#define LIST_DISPLAY_MAX 50																																		// 一个列表最多刷新50张卡
#define LIST_PAD_VER_SIZE 0																																		// 列表垂直填充大小
#define LIST_BTN_PAD_VER_SIZE 0																																	// 列表按鍵垂直填充大小
#define LIST_BTN_HEIGHT 52																																		// 列表按键高度
#define LIST_ONCE_DISPLAY_MAX 10																																// 列表一次显示最多
#define LIST_HEIGHT (LIST_BTN_HEIGHT * LIST_ONCE_DISPLAY_MAX)																									// 列表高度
#define ROLL_LENGTH_MAX (LIST_PAD_VER_SIZE * 2 + LIST_BTN_HEIGHT * (LIST_DISPLAY_MAX - LIST_ONCE_DISPLAY_MAX) + (LIST_BTN_PAD_VER_SIZE * 2) * LIST_DISPLAY_MAX) // 最大滚动长度（滚到底部）
#define LIST_CURR_PAGE (select_index / LIST_DISPLAY_MAX)
#define LIST_NEXT_PAGE (LIST_CURR_PAGE + 1)										  // 下一页
#define LIST_PREV_PAGE (LIST_CURR_PAGE > 0 ? LIST_CURR_PAGE - 1 : LIST_CURR_PAGE) // 上一页

static void file_background_display(void);
static void data_list_display_init(void);

extern lv_obj_t *list_window_btn_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src1, const void *img_src2, char *string);

void record_info_set(media_info *info)
{
	record_info = info;
}

media_info *record_info_get(void)
{
	return record_info;
}

int record_index_get(void)
{
	return select_index;
}
void record_index_set(int index)
{
	select_index = index;
}

int record_total_get(void)
{
	return record_total;
}

media_type record_file_type_get(void)
{
	return rec_file_type;
}

char *cur_record_path_get(void)
{
	return cur_record_path;
}

void carr_record_way_set(media_type way)
{
	rec_file_type = way;
	switch (rec_file_type)
	{

	case FILE_TYPE_SD_MUSIC:
		cur_record_path = SD_MUSIC_PATH;
		record_delete_type = DELETE_ALL_MUSIC;
		break;
	case FILE_TYPE_SD_PICTURE:
		cur_record_path = SD_PICTURE_PATH;
		record_delete_type = DELETE_ALL_PICTURE;
		break;
	case FILE_TYPE_SD_CALL:
		cur_record_path = SD_CALL_PATH;
		record_delete_type = DELETE_ALL_CALL;
		break;
	case FILE_TYPE_SD_MSG:
		cur_record_path = SD_MSG_PATH;
		record_delete_type = DELETE_ALL_MSG;
		break;
	case FILE_TYPE_SD_MOTION:
		cur_record_path = SD_MOTION_PATH;
		record_delete_type = DELETE_ALL_MOTION;
		break;
	case FILE_TYPE_SD_ALARM:
		cur_record_path = SD_ALARM_PATH;
		record_delete_type = DELETE_ALL_ALARM;
		break;
	default:
		break;
	}
	record_index_set(0);
}

/* 参数 type :
**0 => 默认位置显示
**1 => 从顶部开始显示
**2 => 从底部开始显示
*/
static void list_btn_display_set(int type)
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

static void record_list_img_text_display(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_MOVIE_UNFOCUS_PNG);
	enum btn_string_id str_id = STR_VIDEO;
	switch (rec_file_type)
	{
	case FILE_TYPE_FLASH_PHOTO:
	case FILE_TYPE_SD_CALL:
		info.offset = ROM_RES_EVENT_CALL_UNFOCUS_PNG;
		info.size = ROM_RES_EVENT_CALL_UNFOCUS_PNG_SIZE;
		str_id = STR_CALL_RECORD;
		break;
	case FILE_TYPE_SD_MSG:
		info.offset = ROM_RES_EVENT_MESSAGE_UNFOCUS_PNG;
		info.size = ROM_RES_EVENT_MESSAGE_UNFOCUS_PNG_SIZE;
		str_id = STR_MESSAGE_RECORD;
		break;
	case FILE_TYPE_SD_MOTION:
		info.offset = ROM_RES_EVENT_MOTION_UNFOCUS_PNG;
		info.size = ROM_RES_EVENT_MOTION_FOCUS_PNG_SIZE;
		str_id = STR_MOTION_RECORD;
		break;
	case FILE_TYPE_SD_ALARM:
		info.offset = ROM_RES_EVENT_ALARM_UNFOCUS_PNG;
		info.size = ROM_RES_EVENT_ALARM_FOCUS_PNG_SIZE;
		str_id = STR_ALARM_RECORD;
		break;
	default:
		break;
	}
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_value_str(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(str_id));
	lv_obj_set_style_local_value_color(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_align(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_t *line1 = lv_line_create(lv_scr_act(), NULL);
	static lv_point_t point[2] = {{202, 76}, {887, 76}};
	lv_line_set_points(line1, point, 2);
	lv_obj_set_style_local_line_color(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	lv_obj_set_style_local_line_opa(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_line_width(line1, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 2);

	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_pos(label, 200, 38);
	lv_obj_set_size(label, 105, 30);

	lv_label_set_text(label, text_str(str_id));
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

}

extern lv_obj_t *list_window_btn_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src1, const void *img_src2, char *string);

static void window_open_file_btn_up(lv_obj_t *obj)
{
	if (record_info_get()->file_type == VIDEO_TYPE)
	{
		goto_layout(pLAYOUT(video));
	}
	else if (record_info_get()->file_type == PHOTO_TYPE)
		goto_layout(pLAYOUT(photo));
}
static void window_open_file_btn_create(lv_obj_t *parent, int btn_index)
{
	static btn_data btn_data = btn_data_create(NULL, window_open_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	lv_obj_t *btn = list_window_btn_create(parent, 43, 25 + btn_index * (48 + 25), 217, 48, &btn_data, &info1, &info, text_str(STR_OPEN));
	lv_obj_set_state(btn, LV_STATE_PRESSED);
}

static void window_delete_file_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window = lv_obj_get_child_form_id(lv_scr_act(), 8888);
	if (window != NULL)
	{
		lv_obj_del(window);
	}
	int select_index = record_index_get();
	int total = record_total_get();
	const media_type type = record_file_type_get();
	media_file_delete(type, total - select_index - 1);
	total = media_file_total_get(type, false);
	goto_layout(pLAYOUT(file_list));
}
static void window_delete_file_btn_create(lv_obj_t *parent, int btn_index)
{
	static btn_data btn_data = btn_data_create(NULL, window_delete_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);

	list_window_btn_create(parent, 43, 25 + btn_index * (48 + 25), 217, 48, &btn_data, &info1, &info, text_str(STR_DELETE));
}
static void window_delete_all_file_btn_up(lv_obj_t *obj)
{

	start_delete_media(record_delete_type);
	while (delete_media_status())
	{
		ak_sleep_ms(10);
	}
	Debug("====================>>>%d\n", delete_media_status());
	goto_layout(pLAYOUT(file_list));
}

static void window_delete_all_file_btn_create(lv_obj_t *parent, int btn_index)
{
	static btn_data btn_data = btn_data_create(NULL, window_delete_all_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);

	list_window_btn_create(parent, 43, 25 + btn_index * (48 + 25), 217, 48, &btn_data, &info1, &info, text_str(STR_ALL_DELETE));
}
static void window_close_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window = lv_obj_get_child_form_id(lv_scr_act(), 8888);
	if (window != NULL)
	{
		lv_obj_del(window);
	}
}

static void window_close_btn_create(lv_obj_t *parent, int btn_index)
{
	static btn_data btn_data = btn_data_create(NULL, window_close_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	list_window_btn_create(parent, 43, 25 + btn_index * (48 + 25), 217, 48, &btn_data, &info1, &info, text_str(STR_CLOSE));
}

static void record_list_window_create(void)
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
	if (rec_file_type == FILE_TYPE_SD_ALARM)
		set_location(window_cont, 391, 163, 304, 242);
	else
		set_location(window_cont, 391, 163, 304, 313);

	int sub_index = 0;
	if (rec_file_type != FILE_TYPE_SD_ALARM)
		window_open_file_btn_create(window_cont, sub_index++);

	window_delete_file_btn_create(window_cont, sub_index++);

	window_delete_all_file_btn_create(window_cont, sub_index++);
	window_close_btn_create(window_cont, sub_index++);
}

static void file_list_section_flush(void)
{
	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), 999);
	if (label)
	{
		char buf[16];
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%03d/%03d", select_index + 1, record_total);
		lv_label_set_text(label, buf);
	}
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
	int index = LIST_CURR_PAGE * LIST_DISPLAY_MAX + obj->obj_id;

	lv_obj_t *cont = lv_obj_get_child_form_id(obj, obj->obj_id);
	Debug("%p\n", cont);
	lv_obj_t *label = lv_obj_get_child_form_id(cont, obj->obj_id);
	Debug("%p\n", label);
	record_info = media_info_get(rec_file_type, record_total - index - 1);
	if (record_info == NULL)
	{
		Debug("===========>>>mdeia_info_get fail   index:%d\n\n", index);
		return;
	}
	record_index_set(index);
	file_list_section_flush();
	record_list_window_create();

	if (rec_file_type == FILE_TYPE_SD_ALARM)
	{
		media_file_new_clear(rec_file_type, record_total - index - 1);
		if (label != NULL)
		{
			Debug("%p,%s,%d\n", label, record_info->file_name, index);
			lv_obj_set_style_local_text_color(label, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
		}
	}
}

static void data_list_btn_anything(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_PRESS_LOST == event)
	{
		lv_obj_t *btn = lv_obj_get_child(obj, NULL);
		lv_obj_set_state(btn, LV_STATE_DEFAULT);
	}
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

	media_info *info = NULL;
	int index = record_total - LIST_CURR_PAGE * LIST_DISPLAY_MAX - i - 1;
	info = media_info_get(rec_file_type, index);
	if (info == NULL)
	{
		Debug("==========>>>mdeia_info_get fail   index:%d\n\n", index);
		return NULL;
	}

	lv_obj_t *img = lv_img_create(cont, NULL);
	lv_obj_set_pos(img, 16, 13);
	lv_obj_set_size(img, 34, 27);
	if (info->mode == REC_MODE_MANUAL)
	{
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_2_PNG);
		lv_img_set_src(img, &info1);
	}
	else if (info->mode == REC_MODE_AUTO)
	{
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_3_PNG);
		lv_img_set_src(img, &info1);
	}
	else if (info->mode == REC_MODE_MOTION)
	{
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_1_PNG);
		lv_img_set_src(img, &info1);
	}
	else if (info->mode == REC_MODE_ALARM)
	{
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_4_PNG);
		lv_img_set_src(img, &info1);
	}
	else if (info->mode == REC_MODE_MESSAGE)
	{
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_5_PNG);
		lv_img_set_src(img, &info1);
	}

	lv_obj_set_style_local_bg_opa(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x80, 0x0, 0x10));

	lv_obj_set_style_local_border_side(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 2);

	lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LIST_BORDER_COLOR);
	lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LIST_BORDER_COLOR);
	lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LIST_BORDER_COLOR);

	lv_obj_t *file_name_label = lv_label_create(cont, NULL);
	if (info->is_new)
	{
		lv_obj_set_style_local_text_color(file_name_label, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xff1111));
	}
	else
	{
		lv_obj_set_style_local_text_color(file_name_label, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	}
	lv_label_set_text(file_name_label, info->file_name);
	lv_label_set_align(file_name_label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(file_name_label, cont, LV_ALIGN_IN_LEFT_MID, 80, 0);
	lv_obj_set_id(file_name_label, i);

	lv_obj_t *channel_label = lv_label_create(cont, NULL);

	if (rec_file_type == FILE_TYPE_SD_ALARM)
	{
		if (info->ch == 1)
		{
			lv_label_set_text(channel_label, text_str(STR_ALARM_1));
		}
		else if (info->ch == 2)
		{
			lv_label_set_text(channel_label, text_str(STR_ALARM_2));
		}
	}
	else
	{
		if (info->ch == MON_CH_DOOR_1)
		{
			lv_label_set_text(channel_label, text_str(STR_DOOR1));
		}
		else if (info->ch == MON_CH_DOOR_2)
		{
			lv_label_set_text(channel_label, text_str(STR_DOOR2));
		}
		else if (info->ch == MON_CH_CCTV_1)
		{
			lv_label_set_text(channel_label, text_str(STR_CAMERA1));
		}
		else if (info->ch == MON_CH_CCTV_2)
		{
			lv_label_set_text(channel_label, text_str(STR_CAMERA2));
		}
	}

	lv_label_set_align(channel_label, LV_LABEL_ALIGN_RIGHT);
	lv_obj_align(channel_label, cont, LV_ALIGN_IN_RIGHT_MID, -60, 0);

	// 单独的回调函数
	//  Debug("%p,%s,%d\n",list_btn, info->file_name,i);
	static btn_data btn_data = btn_data_create(data_list_btn_down, data_list_btn_up, NULL);
	btn_data.OPS_ANYTHING = data_list_btn_anything;
	list_btn->user_data = &btn_data;
	btn_touch_event_listen(list_btn);

	return list_btn;
}

static void file_list_flush(void)
{
	lv_list_clean(data_list);

	int i = 0;
	int j = record_total - (LIST_CURR_PAGE * LIST_DISPLAY_MAX);
	// Debug("LIST_CURR_PAGE:%d j:%d\n",LIST_CURR_PAGE,j);
	for (; i < LIST_DISPLAY_MAX; i++, j--)
	{
		if (j == 0)
		{
			break;
		}

		// Debug("LIST_CURR_PAGE:%d j:%d\n",LIST_CURR_PAGE,j);
		if (data_list_btn_create(data_list, 0, (i)*LIST_BTN_HEIGHT, 630, LIST_BTN_HEIGHT, i) == NULL)
		{
			i--;
			record_total -= 1;
		}
	}
	// Debug("LIST_CURR_PAGE:%d i:%d\n",LIST_CURR_PAGE,i);

	/* 创建10个空表格，使屏幕哪怕列表未满10个也能做上下拉操作 */
	if (i < LIST_ONCE_DISPLAY_MAX)
	{
		data_list_btn_create(data_list, 0, (i)*LIST_BTN_HEIGHT, 630, LIST_BTN_HEIGHT * (LIST_ONCE_DISPLAY_MAX - i), -1);
		data_list_btn_create(data_list, 0, (i)*LIST_BTN_HEIGHT, 630, 10, -1);
	}
}

static void data_list_prev_page_flush(void)
{
	select_index = LIST_PREV_PAGE * LIST_DISPLAY_MAX;
	Debug("LIST_CURR_PAGE:%d select_index:%d\n", LIST_CURR_PAGE, select_index);
	file_list_flush();
	list_btn_display_set(2);

	file_list_section_flush();
}

static void data_list_next_page_flush(void)
{
	if (LIST_NEXT_PAGE * LIST_DISPLAY_MAX >= record_total)
	{
		return;
	}
	select_index = LIST_NEXT_PAGE * LIST_DISPLAY_MAX;
	Debug("LIST_CURR_PAGE:%d select_index:%d\n", LIST_CURR_PAGE, select_index);
	file_list_flush();
	list_btn_display_set(1);

	file_list_section_flush();
}

static void data_list_display_init(void)
{
	if (LIST_NEXT_PAGE * LIST_DISPLAY_MAX >= record_total)
	{
		select_index = 0;
	}
	printf("FILE_LIST_TOATL:%d LIST_CURR_PAGE:%d\n", record_total, LIST_CURR_PAGE);
	if (record_total == 0)
		return;

	file_list_flush();

	if (record_index_get() > 0)
	{
		list_btn_display_set(0);
	}
	
	lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_pos(label1, 798, 38);
	lv_label_set_align(label1, LV_LABEL_ALIGN_LEFT);
	static char buf[32] = {0};
	if (record_total == 0)
	{
		select_index = -1;
	}
	sprintf(buf, "%03d/%03d", select_index + 1, record_total);
	lv_label_set_text(label1, buf);
	lv_obj_set_id(label1, 999);
}

static void data_list_touch_anything(lv_obj_t *obj, lv_event_t event)
{
	static int flag = 0;
	// printf("%s event：%d flag:%d\n", __func__,event,flag);
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
		else if (flag == 1 && lv_page_on_edge(obj, LV_PAGE_EDGE_TOP) && LIST_CURR_PAGE)
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
	set_location(data_list, 199 /* 175 */, 76, 687 /* 740 */, LIST_HEIGHT);
	lv_obj_set_style_local_pad_ver(data_list, LV_LIST_PART_SCROLLABLE, LV_STATE_DEFAULT, LIST_PAD_VER_SIZE);
	lv_obj_set_style_local_pad_hor(data_list, LV_LIST_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_bg_opa(data_list, LV_LIST_PART_BG, LV_STATE_DEFAULT, LV_OPA_30);

	static btn_data page_data = btn_data_anything_create(data_list_touch_anything);
	data_list->user_data = &page_data;
	btn_touch_event_listen(data_list);
}

static void file_list_back_btn_up(lv_obj_t *obj)
{
	lv_obj_set_state(obj, LV_STATE_DEFAULT);
	goto_layout(pLAYOUT(event));
}

static void file_background_display(void)
{
	backlight_open(true, true, 4);

	data_list_page_create();
	record_list_img_text_display();
	home_back_btn_create(file_list_back_btn_up, NULL);
}

static lv_task_t *media_thumb_open_t = NULL;
extern bool video_decode_wait_thread_status(void);
static void media_thumb_open_task(lv_task_t *task_t)
{

	if (video_decode_wait_thread_status() == false && !is_video_recording())
	{
		forced_scan_media_file(rec_file_type);
		record_total = media_file_total_get(rec_file_type, 0);
		data_list_display_init();
		Debug("rec_file_type===========>%d      rec_file_total===========>%d\n", rec_file_type, record_total);
		media_thumb_device_open(DECODE_WIDTH, DECODE_HIGHT);
		if (media_thumb_open_t)
		{
			lv_task_del(media_thumb_open_t);
			media_thumb_open_t = NULL;
		}

		if (msg_loading_t)
			lv_obj_del_reload(&(msg_loading_t)); /* !!! 一定要删除loading 弹窗的父对象 */
	}
}

static void record_play_parameter_init(void)
{

	if (video_decode_wait_thread_status() == false && !is_video_recording())
	{
		forced_scan_media_file(rec_file_type);
		record_total = media_file_total_get(rec_file_type, 0);
		data_list_display_init();
		media_thumb_device_open(DECODE_WIDTH, DECODE_HIGHT);
		Debug("rec_file_type===========>%d      rec_file_total===========>%d\n", rec_file_type, record_total);
	}
	else
	{
		if (media_thumb_open_t == NULL)
		{
			media_thumb_open_t = lv_task_create(media_thumb_open_task, 50, LV_TASK_PRIO_HIGH, NULL);
		}

		if (msg_loading_t == NULL && video_decode_wait_thread_status())
			msg_loading_t = msg_window_create(text_str(STR_LOAD), true);
	}
}

static void alarm_event_callback(unsigned long arg1, unsigned long arg2)
{
	if (rec_file_type == FILE_TYPE_SD_ALARM)
	{
		record_total = media_file_total_get(rec_file_type, 0);
		file_list_flush();
	}
}

void file_list_sdcard_callback(unsigned long arg1, unsigned long arg2)
{
	// printf("%s ==========================>>%d\n\r",__func__,curr_insert_state);
	goto_layout(pLAYOUT(file_list));
	setting_sdcard_callback(arg1, arg2);
}

static void LAYOUT_ENETER_FUNC(file_list)
{
	Debug("\n\n");

	file_background_display();

	record_play_parameter_init();

	setting_bg_display();

	sdcard_event_register(file_list_sdcard_callback);

	alarm_event_register(alarm_event_callback);
}

static void LAYOUT_QUIT_FUNC(file_list)
{
	if ((layout *)target_layout != &layout_video && (layout *)target_layout != &layout_photo && (layout *)target_layout != &layout_file_list)
	{
		video_play_stop();
		extern bool video_play_wait_thread_quit(void);
		video_play_wait_thread_quit();
		media_thumb_device_close();
		extern bool media_thumb_wait_thread_quit(void);
		media_thumb_wait_thread_quit();
	}

	if (media_thumb_open_t)
	{
		lv_task_del(media_thumb_open_t);
		media_thumb_open_t = NULL;
	}

	if (msg_loading_t)
		lv_obj_del_reload(&(msg_loading_t)); /* !!! 一定要删除loading 弹窗的父对象 */

	alarm_event_register(NULL);
}

CREATE_LAYOUT(file_list);
