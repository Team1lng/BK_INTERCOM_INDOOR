#include "layout_define.h"

extern int record_index_get(void);
extern void record_index_set(int index);
extern media_type record_file_type_get(void);
extern const char *cur_record_path_get(void);
extern int record_total_get(void);

static void photo_back_btn_up(lv_obj_t *obj);
static void media_photo_info_label_display(void);
// static void media_photo_lock_img_display(bool en);

static void media_photo_display(void)
{
	int select = record_index_get();
	int total = record_total_get();
	media_info *info = media_info_get(record_file_type_get(), total - select - 1);
	char file_path[64] = {0};

	strcpy(file_path, cur_record_path_get());
	strcat(file_path, info->file_name);
	Debug("==>>>%s\n\n\n\n\n", file_path);
	if (media_thumb_load(0, 0, 1024, 600, file_path) == false)
	{
		// media_file_delete(record_file_type_get(),total - select - 1);
		prompt_window_create(text_str(STR_FILE_CORRUPTED), photo_back_btn_up);
		return;
	}

	media_file_new_clear(info->type, total - select - 1);

	// media_photo_lock_img_display(info->is_lock);
}

static void media_photo_btn_state_set(lv_obj_t *obj, lv_state_t state)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *children = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(children, state);
}

static void media_photo_btn_img_transform_set(lv_obj_t *obj)
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

static lv_obj_t *media_photo_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color)
{
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(btn, x, y);
	lv_obj_set_size(btn, w, h);

	if (bg_color == true)
	{
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));

		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);

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

	media_photo_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);

	return btn;
}

/*

static void media_photo_delete_btn_down(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_PRESSED);
}




static void media_photo_msgbox_btn_up(lv_obj_t* obj)
{
	unsigned int btn_id = lv_msgbox_get_active_btn(obj);
	if(btn_id  == 1)
	{
		int select_index = record_index_get();
		int total = record_total_get();
		const media_type type = record_file_type_get();
		media_file_delete(type, total - select_index - 1);
		total = media_file_total_get(type, false);
		if(total < 1)
		{
			goto_layout(pLAYOUT(media));
		}
		else
		{
			photo_total_set(total);
			if((select_index) > (total - 1))
			{
				record_index_set(total - 1);
			}
			media_info* info = media_info_get(type, total - select_index - 1);
			if(info->type != FILE_TYPE_SD_MIXED_VIDEO)
			{

				media_photo_info_label_display();
				media_photo_display();
				lv_obj_del(obj);
			}
			else
			{
				goto_layout(pLAYOUT(video));
			}
		}
	}
	else if(btn_id == 0)
	{
		lv_obj_del(obj);
	}
}


static void media_photo_delete_msgbox_create(void)
{
	lv_obj_t* msg_box = lv_msgbox_create(lv_scr_act(), NULL);
	lv_obj_set_id(msg_box, 200);
	lv_obj_set_style_local_bg_color(msg_box,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
	lv_obj_set_style_local_bg_opa(msg_box,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);

	static const char * btns[] ={"Close", "Apple", ""};
	lv_msgbox_set_text(msg_box, "\n\nDelete ?\n\n");
	lv_msgbox_add_btns(msg_box, btns);
	lv_obj_set_size(msg_box, 470,300);

	static btn_data btn_data = btn_data_up_create(media_photo_msgbox_btn_up);
	msg_box->user_data = &btn_data;
	btn_touch_event_listen(msg_box);
	lv_obj_align(msg_box, NULL, LV_ALIGN_CENTER, 0, 0); //Align to the corner

	lv_obj_t* btnmatri_btn = lv_msgbox_get_btnmatrix(msg_box);

	lv_obj_set_style_local_bg_color(btnmatri_btn,LV_BTNMATRIX_PART_BTN,LV_STATE_PRESSED,LV_COLOR_MAKE(0xFF,0,0));
	lv_obj_set_style_local_radius(btnmatri_btn,LV_BTNMATRIX_PART_BTN,LV_STATE_DEFAULT,45);
}



static void media_photo_delete_btn_up(lv_obj_t* obj)
{
	media_photo_btn_state_set(obj,LV_STATE_DEFAULT);

	lv_obj_t* msgbox = lv_obj_get_child_form_id(lv_scr_act(), 200);
	if(msgbox != NULL)
	{
		lv_obj_del(msgbox);
	}
	else
	{
		media_photo_delete_msgbox_create();
	}
}

static void media_photo_delete_btn_create(void)
{
	static btn_data btn_data  = btn_data_create(media_photo_delete_btn_down, media_photo_delete_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_DELETE_PNG);
	media_photo_btn_create(917,25,60,60,&btn_data,&info,true);
}

*/

static void media_photo_left_btn_down(lv_obj_t *obj)
{
	media_photo_btn_state_set(obj, LV_STATE_PRESSED);
}

static void media_photo_left_btn_up(lv_obj_t *obj)
{
	media_photo_btn_state_set(obj, LV_STATE_DEFAULT);

	int select_index = record_index_get();
	int media_total = record_total_get();
	select_index -= 1;
	if (select_index < 0)
	{
		select_index = media_total - 1;
	}
	record_index_set(select_index);
	media_info *info = media_info_get(record_file_type_get(), media_total - select_index - 1);
	if (info->file_type == PHOTO_TYPE)
	{
		media_photo_info_label_display();
		media_photo_display();
	}
	else if (info->file_type == VIDEO_TYPE)
	{
		goto_layout(pLAYOUT(video));
	}
}

static void media_photo_left_btn_create(void)
{
	int total = record_total_get();
	if (total < 2)
	{
		return;
	}

	static btn_data btn_data = btn_data_create(media_photo_left_btn_down, media_photo_left_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAGE_LEFT_PNG);
	media_photo_btn_create(0, 260, 80, 80, &btn_data, &info, false);
}

static void media_photo_right_btn_down(lv_obj_t *obj)
{
	media_photo_btn_state_set(obj, LV_STATE_PRESSED);
}

static void media_photo_right_btn_up(lv_obj_t *obj)
{
	media_photo_btn_state_set(obj, LV_STATE_DEFAULT);

	int select_index = record_index_get();
	int media_total = record_total_get();
	select_index += 1;
	if (select_index == (media_total))
	{
		select_index = 0;
	}
	record_index_set(select_index);
	media_info *info = media_info_get(record_file_type_get(), media_total - select_index - 1);
	if (info->file_type == PHOTO_TYPE)
	{
		media_photo_info_label_display();
		media_photo_display();
	}
	else if (info->file_type == VIDEO_TYPE)
	{
		goto_layout(pLAYOUT(video));
	}
}

static void media_photo_right_btn_create(void)
{
	int total = record_total_get();
	if (total < 2)
	{
		return;
	}

	static btn_data btn_data = btn_data_create(media_photo_right_btn_down, media_photo_right_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAGE_RIGHT_PNG);
	media_photo_btn_create(944, 260, 80, 80, &btn_data, &info, false);
}

static void media_photo_info_label_display(void)
{
	lv_obj_t *parent = lv_obj_get_child_form_id(lv_scr_act(), 0);
	if (parent != NULL)
	{
		int select = record_index_get();
		int total = record_total_get();
		media_info *info = media_info_get(record_file_type_get(), total - select - 1);

		lv_obj_t *label_channel = lv_obj_get_child_form_id(parent, 0);
		if (label_channel != NULL)
		{
			lv_label_set_text(label_channel, info->ch == MON_CH_DOOR_1 ? text_str(STR_DOOR1) : info->ch == MON_CH_DOOR_2 ? text_str(STR_DOOR2)
																						   : info->ch == MON_CH_CCTV_1	 ? text_str(STR_CAMERA1)
																														 : text_str(STR_CAMERA2));
		}

		lv_obj_t *label_time = lv_obj_get_child_form_id(parent, 1);
		if (label_time != NULL)
		{
			char str[128] = {"0"};
			strncpy(&str[0], &info->file_name[0], 4);
			str[4] = '-';
			strncat(&str[5], &info->file_name[4], 2);
			str[7] = '-';
			strncat(&str[8], &info->file_name[6], 2);
			str[10] = ' ';
			str[11] = ' ';

			strncat(&str[12], &info->file_name[9], 2);
			str[14] = ':';
			strncat(&str[15], &info->file_name[11], 2);
			str[17] = ':';
			strncat(&str[18], &info->file_name[13], 2);
			lv_label_set_text(label_time, str);
		}
	}
}

static void media_photo_info_label_create(void)
{
	lv_obj_t *obj = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, 0);
	lv_obj_set_pos(obj, 38, 500);
	lv_obj_set_size(obj, 400, 70);

	lv_obj_t *label_channel = lv_label_create(obj, NULL);
	lv_obj_set_id(label_channel, 0);
	lv_obj_t *label_time = lv_label_create(obj, NULL);
	lv_obj_set_id(label_time, 1);
	media_photo_info_label_display();

	lv_obj_align(label_channel, obj, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_align(label_time, obj, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
}

// static void media_photo_lock_img_display(bool en)
// {
// 	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(),1);
// 	if(obj != NULL)
// 	{
// 		lv_obj_set_hidden(obj, en?false:true);
// 	}

// }

static void media_photo_lock_img_create(void)
{
	lv_obj_t *obj = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, 1);
	lv_obj_set_pos(obj, 499, 30);
	lv_obj_set_size(obj, 27, 32);

	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_LOCK_PNG);
	lv_img_set_src(obj, &info);
	lv_obj_set_hidden(obj, true);
}

static void photo_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(file_list));
}

static void file_list_sdcard_callback(unsigned long arg1, unsigned long arg2)
{
	// printf("%s ==========================>>%d\n\r",__func__,curr_insert_state);
	goto_layout(pLAYOUT(file_list));
	setting_sdcard_callback(arg1, arg2);
}

static void LAYOUT_ENETER_FUNC(photo)
{

	Debug("++++++++++++++++++++++++++++++++++>\n");
	media_photo_lock_img_create();

	// media_photo_delete_btn_create();

	media_photo_left_btn_create();

	media_photo_right_btn_create();

	media_photo_info_label_create();

	media_photo_display();

	home_back_btn_create(photo_back_btn_up, NULL);

	sdcard_event_register(file_list_sdcard_callback);
}

static void LAYOUT_QUIT_FUNC(photo)
{
	if ((layout *)target_layout != &layout_file_list && (layout *)target_layout != &layout_video)
	{
		video_play_stop();
		extern bool video_play_wait_thread_quit(void);
		video_play_wait_thread_quit();
		media_thumb_device_close();
		extern bool media_thumb_wait_thread_quit(void);
		media_thumb_wait_thread_quit();
	}
}

CREATE_LAYOUT(photo);
