#include "layout_define.h"

static unsigned char *picture_data = NULL;

static lv_task_t* digital_picture_ptask = NULL;

static bool photo_task_play_status = true;
int photo_index_get(void);
void photo_index_set(int index);
int photo_total_get(void);
media_type photo_file_type_get(void);
char *cur_photo_path_get(void);

static rom_bin_info info_next = rom_bin_info_get(ROM_RES_MEDIA_NEXT_UNFOCUS_PNG);
static rom_bin_info info1_next = rom_bin_info_get(ROM_RES_MEDIA_NEXT_FOCUS_PNG);
static rom_bin_info info_pause = rom_bin_info_get(ROM_RES_MEDIA_PLAY_UNFOCUS_PNG);
static rom_bin_info info1_pause = rom_bin_info_get(ROM_RES_MEDIA_PAUSE_FOCUS_PNG);
static rom_bin_info info_prev = rom_bin_info_get(ROM_RES_MEDIA_PREV_UNFOCUS_PNG);
static rom_bin_info info1_prev = rom_bin_info_get(ROM_RES_MEDIA_PREV_FOCUS_PNG);

typedef enum media_photo_module_list
{
	NEXT_MODULE,
	PAUSE_MODULE,
	PREV_MODULE,
	TOTAL_MODULE
}media_photo_module_list;

#define MEDIA_PHOTO_MODULE_COORDINATE_INIT  {\
			{564,521,35,35},\
			{494,521,35,35},\
			{424,521,35,35},\
	};


static void digital_picture_diplay(media_info info)
{

#if  1
	extern bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h);
	static rom_bin_info img = rom_bin_raw_get();
	if (picture_data == NULL)
	{
		picture_data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_GUI, 1024 * 600 * 4);
		rom_bin_raw_init(img, picture_data, 1024, 600);
	}
	char picture_name[64] = {0};
	bzero(picture_name,sizeof(picture_name));
	sprintf(picture_name,SD_PICTURE_PATH"%s",info.file_name);
	lv_jpg_decode_data(picture_name, &img, 1024, 600);
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_disp_set_bg_image(lv_disp_get_default(), &img);
#endif
}

static void digital_picture_task(lv_task_t * task)
{
	if(photo_task_play_status)
	{
		photo_index_set(photo_index_get() + 1);		
		media_info *info = media_info_get(photo_file_type_get(),photo_index_get());
		printf("digital_picture_diplay index :%d       name:%s\n\r",photo_index_get(),info->file_name);
		digital_picture_diplay(*info);
	}

}

static void media_photo_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(photo_list));
}

static void media_photo_prev_btn_up(lv_obj_t *obj)
{
	switch (obj->obj_id)
	{
	case NEXT_MODULE:
		if(photo_task_play_status == false){
			photo_index_set(photo_index_get() + 1);
			media_info *info = media_info_get(photo_file_type_get(),photo_index_get());
			// printf("NEXT_MODULE  custom_music_play index :%d       name:%s\n\r",photo_index_get(),info->file_name);
			digital_picture_diplay(*info);
		}
		break;
	case PAUSE_MODULE:
		if((photo_task_play_status = !photo_task_play_status))
		{
			lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&info_pause);
		}
		else
		{
			lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&info1_pause);
		}
		break;
	case PREV_MODULE:
		if(photo_task_play_status == false){
			photo_index_set(photo_index_get() - 1);
			media_info *info = media_info_get(photo_file_type_get(),photo_index_get());
			// printf("PREV_MODULE custom_music_play index :%d       name:%s\n\r",photo_index_get(),info->file_name);
			digital_picture_diplay(*info);
		}
		break;	
	default:
		break;
	}
}
static void media_photo_btn_create(Controls_location coordinate,media_photo_module_list module,rom_bin_info *src_info,rom_bin_info *src_info1)
{
	static btn_data btn_data = btn_data_create(NULL, media_photo_prev_btn_up, NULL);

	lv_obj_t * imgbtn1 = lv_imgbtn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(imgbtn1, coordinate.x, coordinate.y);
    lv_obj_set_size(imgbtn1, coordinate.width, coordinate.high);

	lv_imgbtn_set_src(imgbtn1,LV_BTN_STATE_RELEASED,src_info);
	lv_obj_set_id(imgbtn1, module);
	
	lv_imgbtn_set_src(imgbtn1,LV_BTN_STATE_PRESSED,src_info1);
	imgbtn1->user_data = &btn_data;
	imgbtn1->obj_id = module;
	lv_imgbtn_set_checkable(imgbtn1,true);
    btn_touch_event_listen(imgbtn1);
}

static int picture_play_parameter_init(void)
{
	if(photo_total_get() > 0){

		media_info *info = media_info_get(photo_file_type_get(),photo_index_get());
		// printf("custom_music_play index :%d       name:%s\n\r",photo_index_get(),info->file_name);
		digital_picture_diplay(*info);

		if(digital_picture_ptask == NULL)
    		digital_picture_ptask = lv_task_create(digital_picture_task, user_data_get()->scene.digital_photo_sw_time*1000, LV_TASK_PRIO_MID, &clock);  // 1秒任务
	}
	return photo_total_get();
}


static void LAYOUT_ENETER_FUNC(media_photo)
{
	Debug("==============LAYOUT_ENETER_FUNC====>>>>%d\n\n\n",user_data_get()->other.screen_saver);

	Controls_location module_coordinate[] =  MEDIA_PHOTO_MODULE_COORDINATE_INIT;
	standby_timer_close();
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	
	fb_video_mode_enable(false);

	lv_obj_invalidate(lv_scr_act());
	media_photo_btn_create(module_coordinate[NEXT_MODULE],NEXT_MODULE,&info_next,&info1_next);
	media_photo_btn_create(module_coordinate[PAUSE_MODULE],PAUSE_MODULE,&info_pause,&info_pause);
	media_photo_btn_create(module_coordinate[PREV_MODULE],PREV_MODULE,&info_prev,&info1_prev);
	home_back_btn_create(media_photo_back_btn_up,NULL);
	picture_play_parameter_init();
}

static void LAYOUT_QUIT_FUNC(media_photo)
{	
	Debug("================================\n\r");
	standby_timer_open(-1, NULL);
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	lv_obj_set_click(lv_scr_act(), false);
    if (digital_picture_ptask != NULL)
    {
        lv_task_del(digital_picture_ptask);
		digital_picture_ptask = NULL;
    }
	
	if(picture_data != NULL)
	{
		ak_mem_dma_free(picture_data);
		picture_data = NULL;
		lv_disp_set_bg_image(lv_disp_get_default(), NULL);
	}
	// ak_sleep_ms(1000);
	video_raw_clear();
	void screen_force_refresh(void);
	screen_force_refresh();
	home_bg_display();

}



CREATE_LAYOUT(media_photo);

