#include "layout_define.h"
#include "leo_api.h"

typedef enum camera_module_list
{
	SWITCH_MODULE,
	MODEL_MODULE,
	IP_MODULE,
	ACCOUNT_MODULE,
	PASSWORD_MODULE,
	TOTAL_MODULE
}camera_module_list;

#define CAMERA_MODULE_COORDINATE_INIT  {\
			{199,75,700,52},\
			{199,127,700,52},\
			{199,179,700,52},\
			{199,231,700,52},\
			{199,293,700,52},\
	};

static bool set_camera_flag = 0;
extern char input_data[16];
extern int get_pwd_str;

static void ccamera_setting_display(void);



static void set_camera1_flag_up(lv_obj_t * obj)
{
	if(set_camera_flag != 0){
		set_camera_flag = 0;
		lv_obj_clean(lv_scr_act());
		ccamera_setting_display();
	}
}
static void set_camera2_flag_up(lv_obj_t * obj)
{
	if(set_camera_flag != 1){
		set_camera_flag = 1;
		lv_obj_clean(lv_scr_act());
		ccamera_setting_display();
	
	}
}




static void camera_setting_btn_text_create(void){
	lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);
   	lv_obj_set_pos(btn, 58, 162);
    lv_obj_set_size(btn, 66, 66);
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_CAMERA1_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_CAMERA1_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,set_camera_flag?&info1:&info);


    lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,text_str(STR_CAMERA1));
	if(set_camera_flag){
		lv_obj_set_style_local_value_color(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT ,lv_color_make(255, 255,255));
		lv_obj_set_style_local_value_color(btn,LV_OBJ_PART_MAIN, LV_STATE_PRESSED,BTN_PRESS_COLOR);
	}else{
		lv_obj_set_style_local_value_color(btn,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT ,BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_color(btn,LV_OBJ_PART_MAIN, LV_STATE_PRESSED,BTN_PRESS_COLOR);
	}
	lv_obj_set_style_local_value_align(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,15);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	
	static btn_data btn_data1 = btn_data_create(NULL, set_camera1_flag_up, NULL);
    btn->user_data = &btn_data1;
    btn_touch_event_listen(btn);


	

	lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
   	lv_obj_set_pos(btn1, 58, 321);
    lv_obj_set_size(btn1, 66, 66);
    lv_obj_set_style_local_bg_opa(btn1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(btn1, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_CAMERA2_FOCUS_PNG);
	static rom_bin_info info3 = rom_bin_info_get(ROM_RES_SETTING_CAMERA2_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,set_camera_flag?&info2:&info3);

    lv_obj_set_style_local_value_str(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,text_str(STR_CAMERA2));
	if(!set_camera_flag){
		lv_obj_set_style_local_value_color(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT ,lv_color_make(255, 255,255));
		lv_obj_set_style_local_value_color(btn1,LV_OBJ_PART_MAIN, LV_STATE_PRESSED,BTN_PRESS_COLOR);
	}else{
		lv_obj_set_style_local_value_color(btn1,LV_OBJ_PART_MAIN, LV_STATE_DEFAULT ,BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_color(btn1,LV_OBJ_PART_MAIN, LV_STATE_PRESSED,BTN_PRESS_COLOR);
	}
	lv_obj_set_style_local_value_align(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,15);
	lv_obj_set_style_local_value_font(btn1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	
	static btn_data btn_data2 = btn_data_create(NULL, set_camera2_flag_up, NULL);
    btn1->user_data = &btn_data2;
    btn_touch_event_listen(btn1);

	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_pos(label, 470, 28);
    lv_obj_set_size(label, 124, 38);
	
	lv_label_set_text( label, set_camera_flag ? text_str(STR_CAMERA2) : text_str(STR_CAMERA1));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	
	


}

static void camera_set_btn_syn_up(lv_obj_t * obj)
{
		btn_data *pdata = (btn_data *) obj->user_data;
		lv_obj_t * btn = (lv_obj_t *) pdata->user_data;
		lv_obj_set_state(btn,LV_STATE_DEFAULT);
}
static void camera_set_btn_syn_down(lv_obj_t * obj)
{
		btn_data *pdata = (btn_data *) obj->user_data;
   		lv_obj_t * btn = (lv_obj_t *) pdata->user_data;
		lv_obj_set_state(btn,LV_STATE_PRESSED);
}

static void camera_set_btn_syn_event(lv_obj_t * obj,lv_event_t event)
{

	if(LV_EVENT_PRESS_LOST == event){
		camera_set_btn_syn_up(obj);
	}
		
}

static void camera_camera_switch_right_btn_up(lv_obj_t *obj)
{
    camera_set_btn_syn_up(obj);
	char* str1 = NULL;
	if(set_camera_flag){
		user_data_get()->camera2.enable = !user_data_get()->camera2.enable;
		str1 = user_data_get()->camera2.enable ? text_str(STR_ON) : text_str(STR_OFF);
	}else{
		user_data_get()->camera1.enable = !user_data_get()->camera1.enable;
		str1 = user_data_get()->camera1.enable ? text_str(STR_ON) : text_str(STR_OFF);
	}
	lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),1);
	lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,str1);
}


static void camera_camera_switch_set_btn_create(Controls_location coordinate){
	static btn_data btn_data1 = btn_data_create(camera_set_btn_syn_down, camera_camera_switch_right_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_switch_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = camera_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;
	
	static char* str1 = NULL;
	
	if(set_camera_flag){
		str1 = user_data_get()->camera2.enable ? text_str(STR_ON) : text_str(STR_OFF);
	}else{
		str1 = user_data_get()->camera1.enable ? text_str(STR_ON) : text_str(STR_OFF);
	}
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_STATE),&btn_data3,&btn_data1,&btn_data2);
	lv_obj_set_id(btn, 1);

}


static void camera_camera_model_right_btn_up(lv_obj_t *obj)
{
    camera_set_btn_syn_up(obj);
	char* str1 = NULL;
	if(set_camera_flag){
		user_data_get()->camera2.model = !user_data_get()->camera2.model;
		str1 = user_data_get()->camera2.model ? text_str(STR_HIKVISION) : text_str(STR_DAHUA);
	}else{
		user_data_get()->camera1.model = !user_data_get()->camera1.model;
		str1 = user_data_get()->camera1.model ? text_str(STR_HIKVISION) : text_str(STR_DAHUA);
	}
	lv_obj_t * btn = lv_obj_get_child_form_id(lv_scr_act(),2);
	lv_obj_set_style_local_value_str(btn,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,str1);
}


static void camera_camera_model_set_btn_create(Controls_location coordinate){
	static btn_data btn_data1 = btn_data_create(camera_set_btn_syn_down, camera_camera_model_right_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_model_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = camera_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;
	
	static char* str1 = NULL;
	
	if(set_camera_flag){
		str1 = user_data_get()->camera2.model ? text_str(STR_HIKVISION) : text_str(STR_DAHUA);
	}else{
		str1 = user_data_get()->camera1.model ? text_str(STR_HIKVISION) : text_str(STR_DAHUA);
	}
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_CAMERA_MODEL),&btn_data3,&btn_data1,&btn_data2);
	lv_obj_set_id(btn, 2);

}
static void camera_camera_ip_set_btn_up(lv_obj_t *obj)
{
    camera_set_btn_syn_up(obj);
	get_pwd_str = 2+set_camera_flag;
	goto_layout(pLAYOUT(password_input));
}
static void camera_camera_ip_set_btn_up_1(lv_obj_t *obj)
{
	get_pwd_str = 2+set_camera_flag;
	goto_layout(pLAYOUT(password_input));
}


static void camera_camera_ip_set_btn_create(Controls_location coordinate){
	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_ip_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, camera_camera_ip_set_btn_up_1, NULL);
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;
	
	static char* str1 = NULL;
	if(set_camera_flag){
		str1 = user_data_get()->camera2.ip;
	}else{
		str1 = user_data_get()->camera1.ip;
	}
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_CAMERA_IP_ADDRESS),&btn_data3,NULL,&btn_data2);
	lv_obj_set_id(btn, 8);
}




static void camera_camera_account_set_btn_up(lv_obj_t *obj)
{
	camera_set_btn_syn_up(obj);
	get_pwd_str = 4+set_camera_flag;
	goto_layout(pLAYOUT(password_input));
	
}
static void camera_camera_account_set_btn_up_1(lv_obj_t *obj)
{
	get_pwd_str = 4+set_camera_flag;
	goto_layout(pLAYOUT(password_input));
	
}

static void camera_camera_account_set_btn_create(Controls_location coordinate){
	
	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_account_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, camera_camera_account_set_btn_up_1, NULL);
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;
	
	static char* str1 = NULL;
	if(set_camera_flag){
		str1 = user_data_get()->camera2.account;
	}else{
		str1 = user_data_get()->camera1.account;
	}
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_ACCOUNT_NUMBER),&btn_data3,NULL,&btn_data2);
	lv_obj_set_id(btn, 3);
}


static void camera_camera_pwd_mode_set_btn_up(lv_obj_t *obj)
{
    camera_set_btn_syn_up(obj);
	get_pwd_str = 6+set_camera_flag;
	goto_layout(pLAYOUT(password_input));
}
static void camera_camera_pwd_mode_set_btn_up_1(lv_obj_t *obj)
{
	get_pwd_str = 6+set_camera_flag;
	goto_layout(pLAYOUT(password_input));
	
	
}


static void camera_camera_pwd_mode_set_btn_create(Controls_location coordinate){
	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_pwd_mode_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, camera_camera_pwd_mode_set_btn_up_1, NULL);
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;
	
	static char* str1 = NULL;
	if(set_camera_flag){
		str1 = user_data_get()->camera2.pwd;
	}else{
		str1 = user_data_get()->camera1.pwd;
	}
	lv_obj_t * btn = sys_setting_btn_create(coordinate,str1,text_str(STR_PASSWORD),&btn_data3,NULL,&btn_data2);
	lv_obj_set_id(btn, 4);
}



static void camera_setting_back_btn_up(lv_obj_t *obj)
{

	#ifndef DHCP_IPCAMERA
    	goto_layout(pLAYOUT(setting));
	#else
		goto_layout(pLAYOUT(setting_ipc));
	#endif
}


static void ccamera_setting_display(void)
{	
	Controls_location module_coordinate[] =  CAMERA_MODULE_COORDINATE_INIT;
    camera_setting_btn_text_create();
	camera_camera_switch_set_btn_create(module_coordinate[SWITCH_MODULE]);
	camera_camera_model_set_btn_create(module_coordinate[MODEL_MODULE]);
	camera_camera_ip_set_btn_create(module_coordinate[IP_MODULE]);
	camera_camera_account_set_btn_create(module_coordinate[ACCOUNT_MODULE]);
	camera_camera_pwd_mode_set_btn_create(module_coordinate[PASSWORD_MODULE]);
	home_back_btn_create(camera_setting_back_btn_up,NULL);

}


static void LAYOUT_ENETER_FUNC(setting_camera)
{
	Debug("======================\n");
	setting_bg_display();
	if(prev_layout_get() != &layout_password_input)
	{
		set_camera_flag = 0;
	}
	ccamera_setting_display();
	
}


static void LAYOUT_QUIT_FUNC(setting_camera)
{
   if(user_data_get()->camera1.ip[0] != 0 && user_data_get()->camera1.ip[0] != '0')
   {	
		if(user_data_get()->camera1.model)
		sprintf(user_data_get()->camera1.url, "rtsp://%s:%s@%s:554/Streaming/Channels/1", user_data_get()->camera1.account,user_data_get()->camera1.pwd, user_data_get()->camera1.ip);   
		else
		sprintf(user_data_get()->camera1.url, "rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=1", user_data_get()->camera1.account,user_data_get()->camera1.pwd, user_data_get()->camera1.ip);   
   }
   else
   {
		memset(user_data_get()->camera1.url,0,sizeof(user_data_get()->camera1.url));
   }
   if(user_data_get()->camera2.ip[0] != 0 && user_data_get()->camera2.ip[0] != '0')
   {
		if(user_data_get()->camera2.model)
		sprintf(user_data_get()->camera2.url, "rtsp://%s:%s@%s:554/Streaming/Channels/1", user_data_get()->camera2.account,user_data_get()->camera2.pwd, user_data_get()->camera2.ip);   
		else
		sprintf(user_data_get()->camera2.url, "rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=1", user_data_get()->camera2.account,user_data_get()->camera2.pwd, user_data_get()->camera2.ip);   
   }
   else
   {
		memset(user_data_get()->camera2.url,0,sizeof(user_data_get()->camera2.url));
   }
    user_data_save();
}


CREATE_LAYOUT(setting_camera);


