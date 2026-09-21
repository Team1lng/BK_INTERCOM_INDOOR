#include "layout_define.h"

extern int record_index_get(void);
extern void record_index_set(int index);
extern media_type record_file_type_get(void);
extern const char* cur_record_path_get(void);
extern int record_total_get(void);
extern void record_total_set(int index);


static lv_task_t* layout_video_task = NULL;

static void media_video_info_label_display(void);
static void media_video_progress_bar_display(int cur,int total);

static void media_video_lock_img_display(bool en);

static void video_back_btn_up(lv_obj_t *obj);
static void video_click_up(lv_obj_t* obj);
static bool video_icon_is_hidden = 0;
static bool media_video_display(void)
{
	int select = record_index_get();
	int total = record_total_get();
	media_info* info = media_info_get(record_file_type_get(), total - select - 1);
	char file_path[64] = {0};
	
	strcpy(file_path,cur_record_path_get());
	strcat(file_path,info->file_name);

    // Media_bad_path_check(file_path,total - select - 1);
	
	printf("media_video_display file_path:%s\n\r",file_path);
	if(media_thumb_load(0,0,1024,600,file_path) == false)
	{
		// media_file_delete(record_file_type_get(),total - select - 1);
		prompt_window_create(text_str(STR_FILE_CORRUPTED),video_back_btn_up);
		return false;
	}

	media_file_new_clear(info->type, total - select - 1);

	media_video_info_label_display();

	media_video_lock_img_display(info->is_lock);

	if(access(file_path,F_OK) != 0)
	{
		// media_file_delete(record_file_type_get(),total - select - 1);
		prompt_window_create(text_str(STR_FILE_CORRUPTED),video_back_btn_up);
		return false;
	}
	return true;
}


static void media_video_btn_state_set(lv_obj_t* obj,lv_state_t state)
{
	btn_data* pdata = (btn_data*)obj->user_data;
	lv_obj_t* children = (lv_obj_t*)pdata->user_data;
	lv_obj_set_state( children, state);
}

static void media_video_btn_img_transform_set(lv_obj_t* obj)
{
	lv_obj_set_style_local_transform_zoom(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,256);
	lv_obj_set_style_local_transform_zoom(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,300);
	
	lv_obj_set_style_local_transition_prop_1(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_STYLE_TRANSFORM_ZOOM);
	lv_obj_set_style_local_transition_prop_2(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_STYLE_TRANSFORM_ZOOM);

	lv_obj_set_style_local_transition_time(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,200);
	lv_obj_set_style_local_transition_time(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,200);

	static lv_anim_path_t path ;
	path.cb = lv_anim_path_overshoot,
	path.user_data = NULL;
	lv_obj_set_style_local_transition_path(obj,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,&path);
	lv_obj_set_style_local_transition_path(obj,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,&path);
}

static lv_obj_t* media_video_btn_create(int x,int y,int w,int h,btn_data* btn_pdata,const void* img_src,bool bg_color)
{
	lv_obj_t* btn = lv_btn_create(lv_scr_act(),NULL);
	lv_obj_set_pos(btn, x,y);
	lv_obj_set_size(btn,w,h);

	if(bg_color == true)
	{
		lv_obj_set_style_local_bg_color(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
		lv_obj_set_style_local_bg_color(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_COLOR_MAKE(0x4d,0x7a,0xFF));
		
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_70);
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_OPA_70);

		lv_obj_set_style_local_radius(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,45);
		lv_obj_set_style_local_radius(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,45);
	}
	else
	{
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);
		lv_obj_set_style_local_bg_opa(btn,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,LV_OPA_TRANSP);
	}
	
	lv_obj_t* img = lv_img_create(btn ,NULL);
	lv_img_set_src(img, img_src);


	media_video_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_pdata->user_data = img;
	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	
	return btn;
}

















static void media_video_left_btn_down(lv_obj_t* obj)
{
	media_video_btn_state_set(obj,LV_STATE_PRESSED);
}


static void media_video_left_btn_up(lv_obj_t* obj)
{
	media_video_btn_state_set(obj,LV_STATE_DEFAULT);
	
	video_play_stop();

	int select_index = record_index_get();
	int media_total = record_total_get();
	select_index -= 1;
	if(select_index  <0)
	{
		select_index = media_total -1;
	}
	record_index_set(select_index);
	media_info* info = media_info_get(record_file_type_get(), media_total - select_index - 1);
	if(info->file_type == VIDEO_TYPE)
	{
		//media_video_info_label_display();
		// media_video_display();
		goto_layout(pLAYOUT(video));
	}
	else if(info->file_type == PHOTO_TYPE)
	{
		goto_layout(pLAYOUT(photo));
	}	

}

static void media_video_left_btn_create(void)
{
	int total = record_total_get();
	if(total < 2)
	{
		return ;
	}

	static btn_data btn_data  = btn_data_create(media_video_left_btn_down, media_video_left_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAGE_LEFT_PNG);
	lv_obj_t * btn =  media_video_btn_create(0,260,80,80,&btn_data,&info,false);
	
	lv_obj_set_id(btn,22);
}



static void media_video_right_btn_down(lv_obj_t* obj)
{
	media_video_btn_state_set(obj,LV_STATE_PRESSED);
}


static void media_video_right_btn_up(lv_obj_t* obj)
{
	media_video_btn_state_set(obj,LV_STATE_DEFAULT);

	video_play_stop();

	int select_index = record_index_get();
	int media_total = record_total_get();
	select_index += 1;
	if(select_index  == (media_total))
	{
		select_index = 0;
	}
	record_index_set(select_index);
	media_info* info = media_info_get(record_file_type_get(), media_total - select_index - 1);
	if(info->file_type == VIDEO_TYPE)
	{
		//media_video_info_label_display();
		// media_video_display();
		goto_layout(pLAYOUT(video));
		// extern	bool media_thumb_decode_refresh(int handle);
		// extern int media_h264_decode_handle_id_get(void);
		// media_thumb_decode_refresh(media_h264_decode_handle_id_get());
	}
	else if(info->file_type == PHOTO_TYPE)
	{
		goto_layout(pLAYOUT(photo));
	}
}

static void media_video_right_btn_create(void)
{
	int total = record_total_get();
	if(total < 2)
	{
		return ;
	}

	static btn_data btn_data  = btn_data_create(media_video_right_btn_down, media_video_right_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAGE_RIGHT_PNG);
	lv_obj_t* btn = media_video_btn_create(944,260,80,80,&btn_data,&info,false);
	lv_obj_set_id(btn,11);
}



static void media_video_info_label_display(void)
{
	lv_obj_t* parent = lv_obj_get_child_form_id(lv_scr_act(), 0);
	if(parent != NULL)
	{		
		int select = record_index_get();
		int total = record_total_get();
		media_info* info = media_info_get(record_file_type_get(), total - select - 1);

	
		lv_obj_t* label_channel = lv_obj_get_child_form_id(parent, 0);
		if(label_channel != NULL)
		{
			lv_label_set_text(label_channel, info->ch == MON_CH_DOOR_1?"Door 1":info->ch == MON_CH_DOOR_2?"Door 2":info->ch == MON_CH_CCTV_1?"CAM 1":"CAM 2");
		}

		lv_obj_t* label_time = lv_obj_get_child_form_id(parent, 1);
		if(label_time != NULL)
		{
			char str[128] = {"0"};
			strncpy(&str[0],&info->file_name[0],4);
			str[4] = '-';
			strncat(&str[5],&info->file_name[4],2);
			str[7] = '-';
			strncat(&str[8],&info->file_name[6],2);
			str[10] = ' ';
			str[11] = ' ';
			
			strncat(&str[12],&info->file_name[9],2);
			str[14] = ':';
			strncat(&str[15],&info->file_name[11],2);
			str[17] = ':';
			strncat(&str[18],&info->file_name[13],2);
			lv_label_set_text(label_time, str);
		}
		
		lv_obj_align(label_channel, parent, LV_ALIGN_IN_TOP_LEFT, 0, 0);
		lv_obj_align(label_time, parent, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
	}
}

static void media_video_info_label_create(void)
{
	lv_obj_t* obj = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, 0);
	lv_obj_set_pos(obj,38,500);
	lv_obj_set_size(obj, 400, 70);

	lv_obj_t* label_channel = lv_label_create(obj, NULL);
	lv_obj_set_id(label_channel,0);
	lv_obj_t* label_time = lv_label_create(obj, NULL);
	lv_obj_set_id(label_time,1);	
//	media_video_info_label_display();
	
}


static void media_video_lock_img_display(bool en)
{	
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(),2);
	if(obj != NULL)
	{
		lv_obj_set_hidden(obj, en?false:true);
	}
	
}
static void media_video_lock_img_create(void)
{
	lv_obj_t* obj = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj,2);
	lv_obj_set_pos(obj,499,30);
	lv_obj_set_size(obj, 27 , 32);

	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_LOCK_PNG);
	lv_img_set_src(obj,&info);
	lv_obj_set_hidden(obj, true);
}

static lv_task_t * video_play_icon_clear_task_t = NULL;
static void video_play_icon_clear_task(lv_task_t *task_t)
{
	if(video_play_get_status() == 1 && video_icon_is_hidden == 0){
		video_click_up(NULL);
	}
	
	lv_task_del(video_play_icon_clear_task_t);
	video_play_icon_clear_task_t = NULL;
	
}


static void media_video_play_btn_display(void)
{
	video_icon_is_hidden = 1;
	video_click_up(NULL);

	lv_obj_t* parent = lv_obj_get_child_form_id(lv_scr_act(), 1);
	if(parent != NULL)
	{
		lv_obj_t* obj = lv_obj_get_child_form_id(parent, 0);
		if(video_play_get_status() == 1)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PAUSE_PNG);
			lv_img_set_src(obj, &info);
			lv_obj_align(obj, parent, LV_ALIGN_CENTER, 0,0);
			if(video_play_icon_clear_task_t != NULL){
				lv_task_del(video_play_icon_clear_task_t);
				video_play_icon_clear_task_t = NULL;
			}
			video_play_icon_clear_task_t = lv_task_create(video_play_icon_clear_task, 2000, LV_TASK_PRIO_HIGH, NULL);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PLAY_PNG);
			lv_img_set_src(obj, &info);
			lv_obj_align(obj, parent, LV_ALIGN_CENTER, 5,0);
			if(video_play_icon_clear_task_t != NULL){
				lv_task_del(video_play_icon_clear_task_t);
				video_play_icon_clear_task_t = NULL;
			}
		}		
		
	}
}


static void media_video_play_btn_up(lv_obj_t* obj)
{
	if(video_play_get_status() == 0)
	{
		Debug("=================================\n");
		int select = record_index_get();
		int total = record_total_get();
		media_info* info = media_info_get(record_file_type_get(),total - select -1);
		char file_path[64] = {0};
		
		strcpy(file_path,cur_record_path_get());
		strcat(file_path,info->file_name);
		
		video_play_open(file_path);
		// fb_video_mode_enable(true);
	}
	else
	{
		Debug("=================================\n");
		video_play_pause();
	}
}
static void media_video_play_btn_create(void)
{
	lv_obj_t* obj = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj,1);
	// lv_obj_set_pos(obj,437,225);
	lv_obj_set_size(obj, 120, 120);
	lv_obj_align(obj,lv_scr_act(),LV_ALIGN_CENTER,0,0);
	lv_obj_set_style_local_bg_color(obj,LV_IMG_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x00,0,0));
	lv_obj_set_style_local_radius(obj,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,90);
	lv_obj_set_style_local_bg_opa(obj,LV_IMG_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_70);


	lv_obj_t* img = lv_img_create(obj, NULL);
	lv_obj_set_id(img,0);
	media_video_play_btn_display();	

	static btn_data btn_data = btn_data_up_create(media_video_play_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}





static void layout_video_timer_task(lv_task_t* task_t)
{
	char* play_state = (char*)task_t->user_data;
	char cur_state = video_play_get_status();
	if((*play_state) != cur_state)
	{
		* play_state = cur_state;
		media_video_play_btn_display();

		int cur,total;
		video_play_duration_get(&cur,&total);
		media_video_progress_bar_display(cur,total);

		Debug("cur :%d======================================total:%d\n\r",cur,total);
	}

	if(cur_state == 1)
	{
		int cur,total;
		video_play_duration_get(&cur,&total);
		media_video_progress_bar_display(cur,total);
	}
}

static void media_video_progress_bar_display(int cur,int total)
{
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(), 3);
	if(obj != NULL)
	{
		// Debug("cur :%d======================================total:%d\n\r",cur,total);
		lv_bar_set_range(obj, 0, 1000);
		lv_bar_set_anim_time(obj, 30);
		lv_bar_set_value(obj, cur*1000/total, LV_ANIM_OFF);
	}
}

static void media_video_progress_bar_create(void)
{
	lv_obj_t* obj = lv_bar_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj,3);

	lv_obj_set_pos(obj, 32, 580);
	lv_obj_set_size(obj, 960, 8);
	lv_obj_set_style_local_bg_color(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_make(0xC4, 0xC4, 0xC4));
    lv_obj_set_style_local_bg_color(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_make(0x43, 0x72, 0xEB));
}

static void video_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(file_list));
}
static void video_click_up(lv_obj_t* obj)
{
	lv_obj_t* obj1 = lv_obj_get_child_form_id(lv_scr_act(), 1);
	if(obj1 != NULL)
	{
		lv_obj_set_hidden(obj1,video_icon_is_hidden ?false:true);
	}
	lv_obj_t* obj2 = lv_obj_get_child_form_id(lv_scr_act(), 11);
	if(obj2 != NULL)
	{
		lv_obj_set_hidden(obj2,video_icon_is_hidden ?false:true);
	}
	lv_obj_t* obj3 = lv_obj_get_child_form_id(lv_scr_act(), 22);
	if(obj3 != NULL)
	{
		lv_obj_set_hidden(obj3,video_icon_is_hidden ?false:true);
	}
	if(video_icon_is_hidden){
		
		if(video_play_icon_clear_task_t != NULL){
			lv_task_del(video_play_icon_clear_task_t);
			video_play_icon_clear_task_t = NULL;
		}
		video_play_icon_clear_task_t = lv_task_create(video_play_icon_clear_task, 2000, LV_TASK_PRIO_HIGH, NULL);
	}
	video_icon_is_hidden = !video_icon_is_hidden;
}

static void file_list_sdcard_callback(unsigned long  arg1,unsigned long  arg2)
{
	// printf("%s ==========================>>%d\n\r",__func__,curr_insert_state);
	goto_layout(pLAYOUT(file_list));
	setting_sdcard_callback(arg1,arg2);
}

static void LAYOUT_ENETER_FUNC(video)
{
	lv_area_t area[] = 
	{
		{929,505, 929+67     ,505+67},
		{917, 25, 917+60     ,25+60},
		{0,  260, 0  +80     ,260+80},
		{944,260, 944+80     ,260+80},
		{38, 500, 38 +400    ,500+70},
		{260,140, 260+470+30 , 140+ 290+30},
		{499, 30, 499+27     ,30 +32},
		{32, 580, 32 +960    ,580 +18},
	};
	video_icon_is_hidden = 0;	
	lv_obj_t* obj = lv_scr_act();
	static btn_data btn_data = btn_data_up_create(video_click_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
	lv_obj_set_click(obj, true);

	
	gui_draw_area_set(area,sizeof(area)/sizeof(lv_area_t));
	
	sdcard_event_register(file_list_sdcard_callback);

	media_video_info_label_create();

	media_video_lock_img_create();

	if(media_video_display() == false)
		return;

	media_video_left_btn_create();

	media_video_right_btn_create();

	media_video_play_btn_create();

	media_video_progress_bar_create();
	
	home_back_btn_create(video_back_btn_up,NULL);

	static char play_state = 0;
	play_state = 0;
	layout_video_task = lv_task_create(layout_video_timer_task, 50, LV_TASK_PRIO_MID, &play_state);
	layout_video_timer_task(layout_video_task);
}


static void LAYOUT_QUIT_FUNC(video)
{
	// extern unsigned long long os_get_ms(void);
	// unsigned long long x = os_get_ms();
	sdcard_event_register(NULL);
	video_play_stop();
	if((layout *)target_layout != &layout_file_list && (layout *)target_layout != &layout_photo && (layout *)target_layout != &layout_video)
	{
	// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();
		extern bool video_play_wait_thread_quit(void);
		video_play_wait_thread_quit();
	// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();
		media_thumb_device_close();
	// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();
		extern bool media_thumb_wait_thread_quit(void);
		media_thumb_wait_thread_quit();
	// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();
	}


	fb_video_mode_enable(false);

	if(layout_video_task != NULL)
	{
		lv_task_del(layout_video_task);
		layout_video_task = NULL;
	}
	if(video_play_icon_clear_task_t != NULL){
		lv_task_del(video_play_icon_clear_task_t);
		video_play_icon_clear_task_t = NULL;
	}
	
	lv_obj_set_click(lv_scr_act(), false);
}

CREATE_LAYOUT(video);

