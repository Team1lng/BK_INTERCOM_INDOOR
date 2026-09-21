#include "layout_define.h"
#include "leo_api.h"

static int music_total;
static char *cur_music_path = SD_MUSIC_PATH;
static media_type music_file_type = FILE_TYPE_SD_MUSIC;
static bool is_music_page_move = false;
static lv_obj_t *music_page = NULL;
media_info *music_info = NULL;

static int music_index;

int music_index_get(void)
{
	return music_index;
}
void music_index_set(int index)
{

	if (index >= music_total)
	{
		music_index = 0;
	}
	else if (index < 0)
	{
		music_index = music_total - 1;
	}
	else
	{
		music_index = index;
	}
}

int music_total_get(void)
{
	return music_total;
}
void music_total_set(int index)
{
	music_index = index;
}

media_type music_file_type_get(void)
{
	return music_file_type;
}
char *cur_music_path_get(void)
{
	return cur_music_path;
}

static void music_list_img_text_display(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_MUSIC_UNFOCUS_PNG);
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_value_str(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_MUSIC));
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

	lv_label_set_text(label, text_str(STR_MUSIC));
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_t *label1 = lv_label_create(lv_scr_act(), label);
	lv_obj_set_pos(label1, 798, 38);
	lv_label_set_align(label1, LV_LABEL_ALIGN_LEFT);
	static char buf[32] = {0};
	if (music_total == 0)
	{
		music_index = -1;
	}
	sprintf(buf, "%03d/%03d", music_index + 1, music_total);
	lv_label_set_text(label1, buf);
	lv_obj_set_id(label1, 999);
}

void music_play_parameter_init(void)
{
	if (is_sdcard_insert() == true)
	{
		music_file_type = FILE_TYPE_SD_MUSIC;
		cur_music_path = SD_MUSIC_PATH;
		music_total = media_file_total_get(music_file_type, 0);
	}
	else
	{
		music_total = 0;
	}
	printf("FILE_TYPE_SD_MUSIC music_total :%d\n", music_total);
}

static lv_obj_t *list_window_btn_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src1, const void *img_src2, char *string)
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);

	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	if (img_src1 != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, img_src1);
	}
	if (img_src2 != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, img_src2);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, img_src2);
	}

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, string);
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	return btn;
}

static void window_open_file_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(media_music));
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
	int select_index = music_index_get();
	// int total = music_total_get();
	// printf("%s =========>%d-%d\n",__func__,select_index,total);
	const media_type type = music_file_type_get();
	media_file_delete(type, select_index);
	goto_layout(pLAYOUT(music_list));
}
static void window_delete_file_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_delete_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);

	list_window_btn_create(parent, 43, 97, 217, 48, &btn_data, &info1, &info, text_str(STR_DELETE));
}
static void window_delete_all_file_btn_up(lv_obj_t *obj)
{

	start_delete_media(DELETE_ALL_MUSIC);
	while (delete_media_status())
	{
		ak_sleep_ms(10);
	}
	goto_layout(pLAYOUT(music_list));
}

static void window_delete_all_file_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_delete_all_file_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);

	list_window_btn_create(parent, 43, 169, 217, 48, &btn_data, &info1, &info, text_str(STR_ALL_DELETE));
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

static void music_list_window_create(void)
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
	// lv_cont_set_layout(window_cont, LV_FIT_NONE);
	lv_obj_set_id(window_cont, 888);

	window_open_file_btn_create(window_cont);
	window_delete_file_btn_create(window_cont);

	window_delete_all_file_btn_create(window_cont);
	window_close_btn_create(window_cont);
}

static void music_list_btn_up(lv_obj_t *obj)
{

	if (is_music_page_move == true)
	{
		return;
	}
	music_index = obj->obj_id;
	music_info = media_info_get(music_file_type, music_index);
	Debug("===========>>>%d ===========>%s\n\n\n\n\n", music_index, music_info->file_name);

	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), 999);
	static char buf[32] = {0};
	if (music_total == 0)
	{
		music_index = -1;
	}
	sprintf(buf, "%03d/%03d", music_index + 1, music_total);
	lv_label_set_text(label, buf);
	music_list_window_create();
}

static lv_obj_t *music_list_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int i)
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
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LIST_BORDER_COLOR);
	lv_obj_set_style_local_border_color(btn, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LIST_BORDER_COLOR);

	media_info *info = NULL;
	info = media_info_get(music_file_type, i);

	lv_obj_t *img = lv_img_create(btn, NULL);
	lv_obj_set_pos(img, 16, 13);
	lv_obj_set_size(img, 34, 27);
	// if (info->mode == REC_MODE_MANUAL)
	// {
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_7_PNG);
	lv_img_set_src(img, &info1);
	// }
	// else if (info->mode == REC_MODE_AUTO)
	// {
	// 	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_3_PNG);
	// 	lv_img_set_src(img, &info1);
	// }
	// else if (info->mode == REC_MODE_MOTION)
	// {
	// 	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_1_PNG);
	// 	lv_img_set_src(img, &info1);
	// }
	// else if (info->mode == REC_MODE_ALARM)
	// {
	// 	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_4_PNG);
	// 	lv_img_set_src(img, &info1);
	// }

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

	// 单独的回调函数
	static btn_data btn_data = btn_data_up_create(music_list_btn_up);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	return btn;
}

static void music_page_touch_anything(lv_obj_t *obj, lv_event_t event)
{
	printf("event：%d \n", event);
	if (LV_EVENT_DRAG_BEGIN == event)
	{
		is_music_page_move = true;
	}
	else if (LV_EVENT_DRAG_END == event)
	{
		is_music_page_move = false;
	}
}

static void music_list_page_display(void)
{
	// 创建页面
	music_page = lv_page_create(lv_scr_act(), NULL); // 在当前活跃的屏幕上创建页面

	static btn_data page_data = btn_data_anything_create(music_page_touch_anything);
	music_page->user_data = &page_data;
	btn_touch_event_listen(music_page);

	lv_obj_t *cont = lv_obj_get_child_back(music_page, NULL);
	if (cont != NULL)
	{
		lv_cont_set_fit4(cont, LV_FIT_PARENT, LV_FIT_PARENT, LV_FIT_TIGHT, LV_FIT_MAX);
		lv_cont_set_layout(cont, LV_LAYOUT_OFF);
	}
	set_location(music_page, 199, 80, 687, 520);
	lv_page_set_edge_flash(music_page, 1);
	lv_page_set_scrollbar_mode(music_page, LV_SCROLLBAR_MODE_DRAG);
	// lv_obj_set_style_local_bg_color(music_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x0, 0x0, 0x60));
	lv_obj_set_style_local_bg_opa(music_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_OPA_30);
	// lv_obj_set_style_local_bg_color(music_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, lv_color_make(0x0, 0x20, 0x60));
	// lv_obj_set_style_local_bg_opa(music_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	for (int i = 0; i < music_total; i++)
	{
		if (music_list_btn_create(music_page, 0, ((i) * 52 + 10), 687, 52, i) == NULL)
		{
			i--;
		}
	}
}

static void music_list_back_btn_up(lv_obj_t *obj)
{

	goto_layout(pLAYOUT(media));
}

static void LAYOUT_ENETER_FUNC(music_list)
{
	music_index = 0;
	setting_bg_display();
	music_play_parameter_init();
	music_list_img_text_display();
	music_list_page_display();
	home_back_btn_create(music_list_back_btn_up, NULL);
}

static void LAYOUT_QUIT_FUNC(music_list)
{
	if (target_layout != &layout_photo)
	{
	}
}

CREATE_LAYOUT(music_list);
