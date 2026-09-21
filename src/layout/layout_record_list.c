#include "layout_define.h"
#include "leo_api.h"
#if 0
static int record_total;
static char *cur_record_path = SD_CALL_PATH;
static media_type rec_file_type = FILE_TYPE_SD_CALL;
static bool is_video_page_move = false;
static lv_obj_t *record_page = NULL;
static enum delete_flag  record_delete_type = DELETE_ALL_CALL;
static media_info *record_info = NULL;

static int record_index;

/* 
static void carr_record_way_set(media_type way)
{
	rec_file_type = way;
	switch (rec_file_type)
	{

	case FILE_TYPE_SD_MUSIC:
		cur_record_path =  SD_MUSIC_PATH;
		record_delete_type = DELETE_ALL_MUSIC;
		break;
	case FILE_TYPE_SD_PICTURE:
		cur_record_path =  SD_PICTURE_PATH;
		record_delete_type = DELETE_ALL_PICTURE;
		break;
	case FILE_TYPE_SD_CALL:
		cur_record_path =  SD_CALL_PATH;
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
}

static void record_info_set(media_info *info)
{
	record_info = info;
}
*/
static media_info * record_info_get(void)
{
	return record_info;
}

static int record_index_get(void)
{
	return record_index;
}
static void record_index_set(int index)
{
	record_index = index;
} 

static int record_total_get(void)
{
	return record_total;
}


static media_type record_file_type_get(void)
{
	return rec_file_type;
}
/* static char *cur_record_path_get(void)
{
	return cur_record_path;
} */


static void record_list_img_text_display(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_MOVIE_UNFOCUS_PNG);
	enum btn_string_id  str_id= STR_VIDEO;
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

	lv_obj_t *label1 = lv_label_create(lv_scr_act(), label);
	lv_obj_set_pos(label1, 798, 38);
	lv_label_set_align(label1, LV_LABEL_ALIGN_LEFT);
	static char buf[32] = {0};
	if (record_total == 0)
	{
		record_index = -1;
	}
	sprintf(buf, "%03d/%03d", record_index + 1, record_total);
	lv_label_set_text(label1, buf);
	lv_obj_set_id(label1, 999);
}

void record_play_parameter_init(void)
{
	record_total = media_file_total_get(rec_file_type, 0);
	Debug("rec_file_type===========>%d      rec_file_total===========>%d\n",rec_file_type,record_total);
	if (record_total > 0)
	{
		media_thumb_device_open(DECODE_WIDTH, DECODE_HIGHT);
	}
}
extern lv_obj_t *list_window_btn_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src1, const void *img_src2, char *string);

static void window_open_file_btn_up(lv_obj_t *obj)
{
	if(record_info_get()->file_type == VIDEO_TYPE)
	{
		goto_layout(pLAYOUT(video));
	}
	else if(record_info_get()->file_type == PHOTO_TYPE)
		goto_layout(pLAYOUT(photo));
}
static void window_open_file_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_open_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	lv_obj_t *btn = list_window_btn_create(parent, 43, 25, 217, 48, &btn_data, &info1, &info, text_str(STR_OPEN));
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
	goto_layout(pLAYOUT(record_list));
}
static void window_delete_file_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_delete_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	
	list_window_btn_create(parent, 43, 97, 217, 48, &btn_data, &info1, &info,text_str(STR_DELETE));
}
static void window_delete_all_file_btn_up(lv_obj_t *obj)
{

	start_delete_media(record_delete_type);
	while (delete_media_status())
	{
		ak_sleep_ms(10);
	}
	Debug("====================>>>%d\n",delete_media_status());
	goto_layout(pLAYOUT(record_list));
}

static void window_delete_all_file_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_delete_all_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	
	list_window_btn_create(parent, 43, 169, 217, 48, &btn_data, &info1, &info,text_str(STR_ALL_DELETE));
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
	list_window_btn_create(parent, 43, 241, 217, 48, &btn_data, &info1, &info, text_str(STR_CLOSE));
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
	set_location(window_cont, 391, 163, 304, 313);

	window_open_file_btn_create(window_cont);
	window_delete_file_btn_create(window_cont);

	window_delete_all_file_btn_create(window_cont);
	window_close_btn_create(window_cont);
}

static void record_list_btn_up(lv_obj_t *obj)
{

	if (is_video_page_move == true)
	{
		return;
	}
	record_index = obj->obj_id;
	record_info = media_info_get(rec_file_type, record_total - record_index - 1);
	if(record_info == NULL)
	{
		Debug("===========>>>mdeia_info_get fail   %d\n\n", __LINE__);
		return;
	}
	Debug("===========>>>%d\n\n", record_index);
	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), 999);
	static char buf[32] = {0};
	if (record_total == 0)
	{
		record_index = -1;
	}
	sprintf(buf, "%03d/%03d", record_index + 1, record_total);
	lv_label_set_text(label, buf);
	record_list_window_create();
}

static lv_obj_t *record_list_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int i)
{

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_id(btn, i);

	lv_obj_set_drag_parent(btn, true);

	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x80, 0x0, 0x10));

	lv_obj_set_style_local_border_side(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
	lv_obj_set_style_local_border_width(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LIST_BORDER_COLOR);

	media_info *info = NULL;
	info = media_info_get(rec_file_type, record_total - i - 1);
	if(info == NULL)
	{
		Debug("===========>>>mdeia_info_get fail   %d\n\n", __LINE__);
		// return NULL;
	}
	lv_obj_t *img = lv_img_create(btn, NULL);
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

	lv_obj_t *label = lv_label_create(btn, NULL);
	if (info->is_new)
	{
		lv_obj_set_style_local_text_color(label, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xff1111));
	}
	else
	{
		lv_obj_set_style_local_text_color(label, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	}
	lv_label_set_text(label, info->file_name);

	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 80, 0);

	lv_obj_t *label1 = lv_label_create(btn, NULL);
	if (info->ch == MON_CH_DOOR_1)
	{
		lv_label_set_text(label1,text_str(STR_DOOR1));
	}
	else if (info->ch == MON_CH_DOOR_2)
	{
		lv_label_set_text(label1,text_str(STR_DOOR2));
	}
	else if (info->ch == MON_CH_CCTV_1)
	{
		lv_label_set_text(label1,text_str(STR_CAMERA1));
	}
	else if (info->ch == MON_CH_CCTV_2)
	{
		lv_label_set_text(label1,text_str(STR_CAMERA2));
	}
	lv_label_set_align(label1, LV_LABEL_ALIGN_RIGHT);
	lv_obj_align(label1, btn, LV_ALIGN_IN_RIGHT_MID, -60, 0);
	//单独的回调函数
	static btn_data btn_data = btn_data_create(NULL, record_list_btn_up, NULL);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	return btn;
}

static void video_page_touch_anything(lv_obj_t *obj, lv_event_t event)
{
	printf("event：%d \n", event);
	if (LV_EVENT_DRAG_BEGIN == event)
	{
		is_video_page_move = true;
	}
	else if (LV_EVENT_DRAG_END == event)
	{
		is_video_page_move = false;
	}
}

static void record_list_page_display(void)
{
	//创建页面
	record_page = lv_page_create(lv_scr_act(), NULL); //在当前活跃的屏幕上创建页面

	static btn_data page_data = btn_data_anything_create(video_page_touch_anything);
	record_page->user_data = &page_data;
	btn_touch_event_listen(record_page);

	lv_obj_t *cont = lv_obj_get_child_back(record_page, NULL);
	if (cont != NULL)
	{
		lv_cont_set_fit4(cont, LV_FIT_PARENT, LV_FIT_PARENT, LV_FIT_TIGHT, LV_FIT_MAX);
		lv_cont_set_layout(cont, LV_LAYOUT_OFF);
	}
	set_location(record_page, 199, 80, 687, 520);
	lv_page_set_edge_flash(record_page, 1);
	lv_page_set_scrollbar_mode(record_page, LV_SCROLLBAR_MODE_DRAG);
	lv_obj_set_style_local_bg_color(record_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x0, 0x0, 0x60));
	lv_obj_set_style_local_bg_opa(record_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(record_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, lv_color_make(0x0, 0x20, 0x60));
	lv_obj_set_style_local_bg_opa(record_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	for (int i = 0; i < record_total; i++)
	{
		if (record_list_btn_create(record_page, 0, ((i)*52 + 10), 687, 52, i) == NULL)
		{
			i--;
		}
	}
}

static void record_list_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(event));
}

static lv_task_t *sdcard_insert_detect = NULL;
static bool prev_insert_state = true;
static void sdcard_insert_detect_task(lv_task_t *task_t)
{

	bool curr_insert_state = is_sdcard_insert();
	// printf("%s ==========================>>%d\n\r",__func__,curr_insert_state);
	if (curr_insert_state != prev_insert_state)
	{
		prev_insert_state = curr_insert_state;
		goto_layout(pLAYOUT(record_list));
	}
}

static void setting_sdcard_callback(unsigned long  arg1,unsigned long  arg2)
{
		if(arg2 == 0x00)
		{
			if (sdcard_insert_msg_box == NULL)
			{
				sdcard_insert_msg_box = sdcard_insert_msgbox_create((bool)arg1?text_str(STR_INSET_SD_SUCCEE):text_str(STR_NO_SD_CARD));
			}
			else
			{
				lv_label_set_text((lv_obj_t *)sdcard_insert_msg_box->user_data, (bool)arg1?text_str(STR_INSET_SD_SUCCEE):text_str(STR_NO_SD_CARD));
			}
		}
}

static void LAYOUT_ENETER_FUNC(record_list)
{
	Debug("=========\n\r");

	extern bool video_decode_wait_thread_quit(void);
	video_decode_wait_thread_quit();

	setting_bg_display();
	record_play_parameter_init();
	record_list_img_text_display();
	record_list_page_display();
	home_back_btn_create(record_list_back_btn_up, NULL);
	sdcard_event_register(setting_sdcard_callback);
	prev_insert_state = is_sdcard_insert();
	if (sdcard_insert_detect == NULL)
	{
		sdcard_insert_detect = lv_task_create(sdcard_insert_detect_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
		lv_task_ready(sdcard_insert_detect);
		sdcard_insert_detect_task(NULL);
	}
	
}

static void LAYOUT_QUIT_FUNC(record_list)
{
	Debug("=========\n\r");
	if((layout *)target_layout != &layout_video && (layout *)target_layout != &layout_photo)
	{
		video_play_stop();
		extern bool video_play_wait_thread_quit(void);
		video_play_wait_thread_quit();
		media_thumb_device_close();
		extern bool media_thumb_wait_thread_quit(void);
		media_thumb_wait_thread_quit();
	}
	Debug("=========\n\r");
	if (sdcard_insert_detect != NULL)
	{
		lv_task_del(sdcard_insert_detect);
		sdcard_insert_detect = NULL;
	}
	Debug("=========\n\r");
}

CREATE_LAYOUT(record_list);
#endif