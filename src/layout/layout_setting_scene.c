#include "layout_define.h"
#include "leo_api.h"

typedef enum scene_module_list
{
	PHOTO_SWITCH_MODULE,
	PHOTO_TIME_MODULE,
	MUSIC_SWITCH_MODULE,
	MUSIC_VOLUME_MODULE,
	TOTAL_MODULE
}scene_module_list;

#define SCENE_MODULE_COORDINATE_INIT  {\
			{199,75,700,52},\
			{199,127,700,52},\
			{199,179,700,52},\
			{199,231,700,52},\
	};

// lv_obj_t * prompt_window_create(char *str);
lv_obj_t *prompt_window;

static void scene_setting_display(void);

static void scene_setting_img_text_display(void){
	lv_obj_t * img = lv_img_create(lv_scr_act(), NULL);

	lv_obj_set_pos(img, 44, 218);
    lv_obj_set_size(img, 102, 102);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_SCENE_UNFOCUS_PNG);
	lv_img_set_src(img, &info);
	
	lv_obj_set_style_local_value_str(img,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,text_str(STR_SCENE_SET));
	lv_obj_set_style_local_value_color(img,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_make(255, 255,255));
	lv_obj_set_style_local_value_align(img,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(img,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,15);
	lv_obj_set_style_local_value_font(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

}


static void scene_set_btn_syn_up(lv_obj_t * obj)
{
		btn_data *pdata = (btn_data *) obj->user_data;
		lv_obj_t * btn = (lv_obj_t *) pdata->user_data;
		lv_obj_set_state(btn,LV_STATE_DEFAULT);
}
static void scene_set_btn_syn_down(lv_obj_t * obj)
{
		btn_data *pdata = (btn_data *) obj->user_data;
   		lv_obj_t * btn = (lv_obj_t *) pdata->user_data;
		lv_obj_set_state(btn,LV_STATE_PRESSED);
}

static void scene_set_btn_syn_event(lv_obj_t * obj,lv_event_t event)
{

	if(LV_EVENT_PRESS_LOST == event){
		scene_set_btn_syn_up(obj);
	}
		
}



static void scene_digital_photo_frame_sw_right_btn_up(lv_obj_t *obj)
{
    scene_set_btn_syn_up(obj);

	if(user_data_get()->door1.motion_sensitivity || user_data_get()->door2.motion_sensitivity)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_MOTION_DETECTION_CLOSE),NULL);
		}
		return;
	}

	char* str1 = NULL;
	user_data_get()->scene.digital_photo_frame_sw = !user_data_get()->scene.digital_photo_frame_sw;
	str1 = user_data_get()->scene.digital_photo_frame_sw?text_str(STR_ON):text_str(STR_OFF);
	
	lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),1);
	lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,str1);
}

static void scene_digital_photo_frame_sw_set_btn_create(Controls_location coordinate){
	static btn_data btn_data1 = btn_data_create(scene_set_btn_syn_down, scene_digital_photo_frame_sw_right_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(scene_set_btn_syn_down, scene_digital_photo_frame_sw_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = scene_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = scene_set_btn_syn_event;
	char* str1 = NULL;
	str1 = user_data_get()->scene.digital_photo_frame_sw?text_str(STR_ON):text_str(STR_OFF);
	
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_DIGITAL_PHOTO),&btn_data3,&btn_data1,&btn_data2);
	lv_obj_set_id(btn, 1);

}

static void scene_digital_photo_sw_time_set_left_btn_up(lv_obj_t *obj)
{
    scene_set_btn_syn_up(obj);
	lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),2);
	static char str1[32] = {0};
	memset(str1,0,sizeof(str1));
	if(--user_data_get()->scene.digital_photo_sw_time < 1){
		user_data_get()->scene.digital_photo_sw_time = 30;
	}
	sprintf(str1, "%d %s",user_data_get()->scene.digital_photo_sw_time,text_str(STR_S));
	
	lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,str1);
}
static void scene_digital_photo_sw_time_set_right_btn_up(lv_obj_t *obj)
{
	scene_set_btn_syn_up(obj);
	lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),2);
	static char str1[32] = {0};
	memset(str1,0,sizeof(str1));
	if(++user_data_get()->scene.digital_photo_sw_time > 30){
		user_data_get()->scene.digital_photo_sw_time = 1;
	}
	sprintf(str1, "%d %s",user_data_get()->scene.digital_photo_sw_time,text_str(STR_S));
	
	lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,str1);
}

static void scene_digital_photo_sw_time_set_btn_create(Controls_location coordinate){
	
	static btn_data btn_data1 = btn_data_create(scene_set_btn_syn_down, scene_digital_photo_sw_time_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(scene_set_btn_syn_down, scene_digital_photo_sw_time_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = scene_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = scene_set_btn_syn_event;
	static char str1[32] = {0};
	memset(str1,0,sizeof(str1));
	sprintf(str1, "%d %s",user_data_get()->scene.digital_photo_sw_time,text_str(STR_S));
	
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_DIGITAL_PHOTO_SWITCH_TIME),&btn_data3,&btn_data1,&btn_data2);
	lv_obj_set_base_dir(btn,LV_BIDI_DIR_LTR);
	lv_obj_set_id(btn, 2);
}

static void scene_bg_music_sw_set_btn_up(lv_obj_t *obj)
{
    scene_set_btn_syn_up(obj);
	if(user_data_get()->door1.motion_sensitivity || user_data_get()->door2.motion_sensitivity)
	{
		if (prompt_window == NULL)
		{
			prompt_window = prompt_window_create(text_str(STR_MOTION_DETECTION_CLOSE),NULL);
		}
		return;
	}

	char* str1 = NULL;
	user_data_get()->scene.bg_music_sw = !user_data_get()->scene.bg_music_sw;
	str1 = user_data_get()->scene.bg_music_sw?text_str(STR_ON):text_str(STR_OFF);
	
    lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),3);
	lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,str1);
}


static void scene_bg_music_sw_set_btn_create(Controls_location coordinate){
	static btn_data btn_data1 = btn_data_create(scene_set_btn_syn_down, scene_bg_music_sw_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(scene_set_btn_syn_down, scene_bg_music_sw_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = scene_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = scene_set_btn_syn_event;
	char* str1 = NULL;
	str1 = user_data_get()->scene.bg_music_sw?text_str(STR_ON):text_str(STR_OFF);
	
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_BACKGROUND_MUSIC),&btn_data3,&btn_data1,&btn_data2);
	lv_obj_set_id(btn, 3);
}

static void scene_bg_music_vol_set_left_btn_up(lv_obj_t *obj)
{
    scene_set_btn_syn_up(obj);
	lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),4);
	static char str1[8] = {0};
	memset(str1,0,sizeof(str1));
	if(--user_data_get()->scene.bg_music_vol < 1){
		user_data_get()->scene.bg_music_vol = 10;
	}
	sprintf(str1, "%d",user_data_get()->scene.bg_music_vol);
	
	lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,str1);
}
static void scene_bg_music_vol_set_right_btn_up(lv_obj_t *obj)
{
	scene_set_btn_syn_up(obj);
	lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),4);
	static char str1[8] = {0};
	memset(str1,0,sizeof(str1));
	if(++user_data_get()->scene.bg_music_vol > 10){
		user_data_get()->scene.bg_music_vol = 1;
	}
	sprintf(str1, "%d",user_data_get()->scene.bg_music_vol);
	
	lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,str1);
}

static void scene_bg_music_vol_set_btn_create(Controls_location coordinate){
	
	static btn_data btn_data1 = btn_data_create(scene_set_btn_syn_down, scene_bg_music_vol_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(scene_set_btn_syn_down, scene_bg_music_vol_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = scene_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = scene_set_btn_syn_event;
	static char str1[8] = {0};
	memset(str1,0,sizeof(str1));
	sprintf(str1, "%d",user_data_get()->scene.bg_music_vol);
	
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_BACKGROUND_MUSIC_VOLUME),&btn_data3,&btn_data1,&btn_data2);
	lv_obj_set_id(btn, 4);
}






static void scene_setting_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}

static void scene_setting_display(void)
{	
	Controls_location module_coordinate[] =  SCENE_MODULE_COORDINATE_INIT;
    scene_setting_img_text_display();
	scene_digital_photo_frame_sw_set_btn_create(module_coordinate[PHOTO_SWITCH_MODULE]);
	scene_digital_photo_sw_time_set_btn_create(module_coordinate[PHOTO_TIME_MODULE]);
	scene_bg_music_sw_set_btn_create(module_coordinate[MUSIC_SWITCH_MODULE]);
	scene_bg_music_vol_set_btn_create(module_coordinate[MUSIC_VOLUME_MODULE]);
	
	home_back_btn_create(scene_setting_back_btn_up,NULL);
}



static void LAYOUT_ENETER_FUNC(setting_scene)
{
	setting_bg_display();
	scene_setting_display();
	
}


static void LAYOUT_QUIT_FUNC(setting_scene)
{
	if(prompt_window != NULL){
		lv_obj_del(prompt_window);
		prompt_window = NULL;
	}
    user_data_save();
}


CREATE_LAYOUT(setting_scene);


