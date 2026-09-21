#include "layout_define.h"
static void interphone_call_event_extern_func(unsigned long arg1,unsigned long arg2);
static void interphone_call_event_inside_func(unsigned long arg1,unsigned long arg2);

static void interphone_out_page_create(void);
static void interphone_call_talk_page_create(void);


interphone_status_enum interphone_status = INTERPHONE_STATUS_IDLE;

/*
*通话标志位，0x00:空闲   
*			 0x01:选中，即将呼叫
*		     0x02:呼叫中，
*            0x03:正在通话
*/
static char interphone_device_btn_status_flag[3] = {0x00,0x00,0x00};


static void interphone_head_label_create(void)
{
	lv_obj_t* obj = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_pos(obj, 412, 25);
	lv_obj_set_size(obj, 200, 60);
	lv_obj_t* label = lv_label_create(obj, NULL);
	lv_obj_align(label, obj, LV_ALIGN_CENTER, -30, -10);
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	
	lv_obj_set_style_local_text_font(label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,FONT_SIZE(28));
	lv_label_set_text(label, "Interphone");
}

static void interphone_cancel_btn_up(lv_obj_t* obj)
{
	goto_layout(pLAYOUT(home));
}


static void interphone_cancel_btn_create(void)
{
	lv_obj_t* obj = lv_imgbtn_create(lv_scr_act(), NULL);
	lv_obj_set_style_local_bg_color(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(57,57,57));
	lv_obj_set_style_local_bg_color(obj,LV_IMGBTN_PART_MAIN,LV_STATE_PRESSED,LV_COLOR_MAKE(0x4d,0x7a,0xFF));
	
	lv_obj_set_style_local_bg_opa(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_70);
	lv_obj_set_style_local_bg_opa(obj,LV_IMGBTN_PART_MAIN,LV_STATE_PRESSED,LV_OPA_70);

	lv_obj_set_style_local_radius(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,45);
	lv_obj_set_style_local_radius(obj,LV_IMGBTN_PART_MAIN,LV_STATE_PRESSED,45);
	
	lv_obj_set_pos(obj,25,25);
	lv_obj_set_size(obj,60,60);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, &info);
	lv_imgbtn_set_src(obj, LV_BTN_STATE_PRESSED, &info);
	
	static btn_data btn_data = btn_data_up_create(interphone_cancel_btn_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
}


static network_device interphone_index_device_get(int index)
{
	if(index ==1)
	{
		return user_data_get()->other.network_device == DEVICE_INDOOR_ID1?DEVICE_INDOOR_ID2:DEVICE_INDOOR_ID1;
	}
	if(index == 2)
	{
		return ((user_data_get()->other.network_device == DEVICE_INDOOR_ID1)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID2))?DEVICE_INDOOR_ID3:DEVICE_INDOOR_ID2;
	}
	if(index == 3)
	{
		return ((user_data_get()->other.network_device == DEVICE_INDOOR_ID1)||(user_data_get()->other.network_device == DEVICE_INDOOR_ID2))?DEVICE_INDOOR_ID4:DEVICE_INDOOR_ID3;
	}
	return DEVICE_UNKONW;
}

static void interphone_device_id_btn_display(int id,lv_obj_t* obj)
{
	id -=1;
	if(interphone_device_btn_status_flag[id] == 0)
	{
		lv_obj_set_style_local_bg_color(obj,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x26,0x26,0x26));
	}
	else if((interphone_device_btn_status_flag[id] == 1)||(interphone_device_btn_status_flag[id] == 2))
	{
		lv_obj_set_style_local_bg_color(obj,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xB8,0x8D,0x56));
	}
	else if(interphone_device_btn_status_flag[id] == 3)
	{
		lv_obj_set_style_local_bg_color(obj,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x21,0xAF,0x8D));
	}
}
static lv_obj_t* interphone_call_btn_create(int x,int y ,int w,int h ,btn_data* data,int id)
{
	lv_obj_t* parent = lv_cont_create(lv_scr_act(),NULL);
	lv_obj_set_style_local_bg_opa(parent,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);
	lv_obj_set_pos(parent,x,y);
	lv_obj_set_size(parent,w,h);
	lv_obj_set_click(parent, true);

	lv_obj_t* cont_obj1 = lv_cont_create(parent,NULL);
	lv_obj_set_pos(cont_obj1,30,30);
	lv_obj_set_size(cont_obj1,124,124);
	lv_obj_set_click(cont_obj1, false);
	lv_obj_set_style_local_bg_color(cont_obj1,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFF,0xFF,0xFF));
	lv_obj_set_style_local_radius(cont_obj1,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,360);
	lv_obj_set_style_local_bg_opa(cont_obj1,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_30);


	lv_obj_t* cont_obj2 = lv_cont_create(cont_obj1,NULL);
	lv_obj_set_pos(cont_obj2,39,16);
	lv_obj_set_size(cont_obj2,46,46);
	lv_obj_set_click(cont_obj2, false);
	lv_obj_set_style_local_bg_color(cont_obj2,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFF,0xFF,0xFF));
	lv_obj_set_style_local_radius(cont_obj2,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,360);
	lv_obj_set_style_local_bg_opa(cont_obj2,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_COVER);

	lv_obj_t* img = lv_img_create(cont_obj1,NULL);
	lv_obj_set_pos(img,25,67);
	rom_bin_info info = rom_bin_info_get(ROM_RES_INTERPHONE_HEAD_PNG);
	lv_img_set_src(img, &info);


	lv_obj_t* label = lv_label_create(parent,NULL);
	lv_obj_align(label, parent, LV_ALIGN_IN_BOTTOM_MID, 0, -40);
	lv_obj_set_style_local_text_font(label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,FONT_SIZE(28));
	network_device device = interphone_index_device_get(id);
	char buffer[12] = {0};
	sprintf(buffer,"ID%d",device == DEVICE_INDOOR_ID1?1:device == DEVICE_INDOOR_ID2?2:device == DEVICE_INDOOR_ID3?3:4);
	lv_label_set_text(label, buffer);

	if(data != NULL)
	{
		parent->user_data = data;
		btn_touch_event_listen(parent);
	}
	interphone_device_id_btn_display(id,parent);
	return parent;
}


static void interphone_call_out_btn_display(void);

static void interphone_device_id_1_btn_up(lv_obj_t* obj)
{
	interphone_device_btn_status_flag[0] = interphone_device_btn_status_flag[0]?0x00:0x01;
	interphone_device_id_btn_display(1,obj);

	interphone_call_out_btn_display();
}

static void interphone_device_id_2_btn_up(lv_obj_t* obj)
{
	interphone_device_btn_status_flag[1] = interphone_device_btn_status_flag[1]?0x00:0x01;
	interphone_device_id_btn_display(2,obj);

	interphone_call_out_btn_display();
}

static void interphone_device_id_3_btn_up(lv_obj_t* obj)
{
	interphone_device_btn_status_flag[2] = interphone_device_btn_status_flag[2]?0x00:0x01;
	interphone_device_id_btn_display(3,obj);

	interphone_call_out_btn_display();
}


static lv_task_t* interphone_publish_timer_task = NULL;
static void interphone_publish_timer_task_func(lv_task_t* task_t)
{
	if(interphone_status == INTERPHONE_STATUS_PUBLISH)
	{
		printf("publish status to idle \n");
		interphone_status = INTERPHONE_STATUS_IDLE;
	}

	lv_task_del(task_t);
	interphone_publish_timer_task = NULL;
}
static void interphone_call_out_btn_up(lv_obj_t* obj)
{
	if(interphone_status == INTERPHONE_STATUS_IDLE)
	{
		network_cmd_data data;
		data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
		data.arg1 = 1;
		data.arg2 = user_data_get()->other.network_device;
		for(int i = 0 ; i < 3 ; i++)
		{
			if(interphone_device_btn_status_flag[i] == 0x01)
			{
				
				data.device = interphone_index_device_get(i+1);
				network_send_cmd_data(&data);
				interphone_status = INTERPHONE_STATUS_PUBLISH;
			}
		}
		if((interphone_status == INTERPHONE_STATUS_PUBLISH)&&(interphone_publish_timer_task == NULL))
		{
			interphone_publish_timer_task = lv_task_create(interphone_publish_timer_task_func, 500, LV_TASK_PRIO_MID, NULL);
		}
	}
	else if(interphone_status == INTERPHONE_STATUS_OUT)
	{
		goto_layout(pLAYOUT(home));
	}
	else if(interphone_status == INTERPHONE_STATUS_TALK)
	{
		goto_layout(pLAYOUT(home));
	}
}
static void interphone_call_out_btn_display(void)
{
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(), 0);
	if(obj == NULL)
	{
		return ;
	}

	if((interphone_status == INTERPHONE_STATUS_IDLE)||(interphone_status == INTERPHONE_STATUS_PUBLISH))
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_ON_PNG);
		lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&info);
		lv_imgbtn_set_src(obj,LV_BTN_STATE_PRESSED,&info);
		if((interphone_device_btn_status_flag[0])||(interphone_device_btn_status_flag[1])||interphone_device_btn_status_flag[2])
		{
			lv_obj_set_style_local_bg_color(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x32,0xD7,0x4B));
			
			lv_obj_set_click(obj, true);
		}
		else
		{
			lv_obj_set_style_local_bg_color(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x39,0x39,0x39));
			lv_obj_set_click(obj, false);
		}
	}
	else
	{
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_OFF_PNG);
		lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&info);
		lv_imgbtn_set_src(obj,LV_BTN_STATE_PRESSED,&info);
		lv_obj_set_style_local_bg_color(obj,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFF,0x00,0x00));
		lv_obj_set_click(obj, true);
	}
}
static void interphone_call_out_btn_create(int y)
{
	lv_obj_t* imgbtn = lv_imgbtn_create(lv_scr_act(),NULL);
	lv_obj_set_id(imgbtn, 0);
	lv_obj_set_pos(imgbtn,466,y);
	lv_obj_set_size(imgbtn,100,100);
	lv_obj_set_style_local_bg_opa(imgbtn, LV_IMGBTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,360);
	

	interphone_call_out_btn_display();

	static btn_data btn_data = btn_data_up_create(interphone_call_out_btn_up);
	imgbtn->user_data = &btn_data;
	btn_touch_event_listen(imgbtn);
}


static void interphone_out_page_create(void)
{
	lv_obj_clean(lv_scr_act());
	
	int id_buf[3] = {0};
	int j = 0;
	int call_num = 0;
	for(int i = 0 ; i < 3 ; i++)
	{
		if((interphone_device_btn_status_flag[i] == 0x01)||(interphone_device_btn_status_flag[i] == 0x02))
		{	
			id_buf[j++] = i+1;
			call_num++;
		}
	}

	if(call_num == 1)
	{
		interphone_call_btn_create(421,106,183,246,NULL,id_buf[0]);
	}
	else if(call_num == 2)
	{
		interphone_call_btn_create(320,106,183,246,NULL,id_buf[0]);
		interphone_call_btn_create(522,106,183,246,NULL,id_buf[1]);
	}
	else if(call_num == 3)
	{
		interphone_call_btn_create(219,106,183,246,NULL,id_buf[0]);
		interphone_call_btn_create(421,106,183,246,NULL,id_buf[1]);
		interphone_call_btn_create(623,106,183,246,NULL,id_buf[2]);
	}

	
	interphone_call_out_btn_create(426);
}
static void interphone_idle_page_create(void)
{
	lv_obj_clean(lv_scr_act());
	
	interphone_head_label_create();
	interphone_cancel_btn_create();

	
	static btn_data btn1_data = btn_data_up_create(interphone_device_id_1_btn_up);
	interphone_call_btn_create(217,177,183,246,&btn1_data,1);

	static btn_data btn2_data = btn_data_up_create(interphone_device_id_2_btn_up);
	interphone_call_btn_create(420,177,183,246,&btn2_data,2);

	static btn_data btn3_data = btn_data_up_create(interphone_device_id_3_btn_up);
	interphone_call_btn_create(624,177,183,246,&btn3_data,3);

	interphone_call_out_btn_create(462);
}


static void interphone_call_in_handup_btn_up(lv_obj_t* obj)
{
	goto_layout(pLAYOUT(home));
}
static void interphone_call_in_handup_btn_create(void)
{
	lv_obj_t* imgbtn = lv_imgbtn_create(lv_scr_act(),NULL);
	lv_obj_set_id(imgbtn, 0);
	lv_obj_set_pos(imgbtn,404,426);
	lv_obj_set_size(imgbtn,100,100);
	lv_obj_set_style_local_bg_opa(imgbtn, LV_IMGBTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,360);
	

	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_OFF_PNG);
	lv_imgbtn_set_src(imgbtn,LV_BTN_STATE_RELEASED,&info);
	lv_imgbtn_set_src(imgbtn,LV_BTN_STATE_PRESSED,&info);
	lv_obj_set_style_local_bg_color(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFF,0x00,0x00));
	lv_obj_set_click(imgbtn, true);

	static btn_data btn_data = btn_data_up_create(interphone_call_in_handup_btn_up);
	imgbtn->user_data = &btn_data;
	btn_touch_event_listen(imgbtn);
}

static network_device interphone_call_mastar_device;
static void interphone_call_in_talk_btn_up(lv_obj_t* obj)
{
	lv_obj_set_hidden(obj,true);

	lv_obj_t* obj_handup = lv_obj_get_child_form_id(lv_scr_act(),0);
	if(obj_handup != NULL)
	{
		lv_obj_set_x(obj_handup, 466);

		network_cmd_data data;
		data.device = interphone_call_mastar_device;
		data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
		data.arg1 = 3;
		data.arg2 = (char)user_data_get()->other.network_device;
		network_send_cmd_data(&data);

		for(int i = 0 ; i < 3 ; i++)
		{
			if(interphone_call_mastar_device == interphone_index_device_get(i+1))
			{
				interphone_status = INTERPHONE_STATUS_TALK;
				interphone_device_btn_status_flag[i] = 0x03;
				interphone_call_talk_page_create();

				audio_play_stop_set();
				audio_talk_ctrl ctrl = {{interphone_call_mastar_device},(OPERATION_OPTION(AUDIO_SEND_EN)|
																																	OPERATION_OPTION(AUDIO_RECEIVE_EN)|
																																	OPERATION_OPTION(AUDIO_OUT_EN)|
																																	OPERATION_OPTION(AUDIO_IN_EN)),AI_AO_O,true,true,70};
				audio_talk_open(ctrl);
			}
		}
		
	}
	
}

static void interphone_call_in_talk_btn_create(void)
{
	lv_obj_t* imgbtn = lv_imgbtn_create(lv_scr_act(),NULL);
	lv_obj_set_id(imgbtn, 0);
	lv_obj_set_pos(imgbtn,522,426);
	lv_obj_set_size(imgbtn,100,100);
	lv_obj_set_style_local_bg_opa(imgbtn, LV_IMGBTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,360);
	

	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_ON_PNG);
	lv_imgbtn_set_src(imgbtn,LV_BTN_STATE_RELEASED,&info);
	lv_imgbtn_set_src(imgbtn,LV_BTN_STATE_PRESSED,&info);
	lv_obj_set_style_local_bg_color(imgbtn,LV_IMGBTN_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_MAKE(0x32,0xD7,0x4B));
	lv_obj_set_click(imgbtn, true);

	static btn_data btn_data = btn_data_up_create(interphone_call_in_talk_btn_up);
	imgbtn->user_data = &btn_data;
	btn_touch_event_listen(imgbtn);
}



static void interphone_call_in_page_create(void)
{
	lv_obj_clean(lv_scr_act());

	for(int i = 1 ; i < 4 ; i++)
	{
		if(interphone_call_mastar_device == interphone_index_device_get(i))
		{
			interphone_device_btn_status_flag[i-1] = 0x02;
			interphone_call_btn_create(420,106,183,246,NULL,i);
			break;
		}
	}
	interphone_call_in_handup_btn_create();
	interphone_call_in_talk_btn_create();
}


static void interphone_call_talk_page_create(void)
{
	lv_obj_clean(lv_scr_act());
	
	for(int i = 1 ; i < 4 ; i++)
	{
		if(interphone_device_btn_status_flag[i-1] == 0x03)
		{
			interphone_call_btn_create(420,106,183,246,NULL,i);
			break;
		}
	}
	interphone_call_out_btn_create(426);
}
static void LAYOUT_ENETER_FUNC(interphone)
{
	Debug("=========================>>\n\r");
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	system_bg_enable_set(false);

	memset(interphone_device_btn_status_flag,0,sizeof(interphone_device_btn_status_flag));
	if(interphone_status == INTERPHONE_STATUS_IDLE)
	{
		interphone_idle_page_create();
	}
	else if(interphone_status == INTERPHONE_STATUS_IN)
	{
		interphone_call_in_page_create();
	}

	interphone_call_event_register(interphone_call_event_inside_func);


}

static void LAYOUT_QUIT_FUNC(interphone)
{	
	if(interphone_publish_timer_task != NULL)
	{
		lv_task_del(interphone_publish_timer_task);
		interphone_publish_timer_task =NULL;
	}
	
	audio_play_stop_set();
	audio_talk_close(true);

	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);

	network_cmd_data data;
	data.device = DEVICE_ALL;
	data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
	data.arg1 = 4;
	data.arg2 = user_data_get()->other.network_device;
	network_send_cmd_data(&data);

	
	memset(interphone_device_btn_status_flag,0,sizeof(interphone_device_btn_status_flag));
	interphone_status = INTERPHONE_STATUS_IDLE;

	interphone_call_event_register(interphone_call_event_extern_func);

}




static void interphone_ring_play_finsih_callback(void)
{
	if((interphone_status == INTERPHONE_STATUS_OUT)||(interphone_status == INTERPHONE_STATUS_IN))
	{
		interphone_ring_play(6,NULL,false,interphone_ring_play_finsih_callback);
	}
}

/*
*	arg1 = 1:呼叫
*   arg1 = 2:有设备应答呼叫
*	arg1 = 3:有设备接受通话
*   arg1 = 4:设备挂断正在通话
*	arg1 = 5:设备正忙
*   arg2 为对方设备号
*/
static void interphone_call_event_extern_func(unsigned long arg1,unsigned long arg2)
{
	if(arg1 == 1)
	{
		if(interphone_status == INTERPHONE_STATUS_IDLE)
		{
			interphone_call_mastar_device = (network_device)arg2;
			network_cmd_data data;
			data.device = interphone_call_mastar_device;
			data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			data.arg1 = 2;
			data.arg2 = (char)user_data_get()->other.network_device;
			network_send_cmd_data(&data);
		
			interphone_status = INTERPHONE_STATUS_IN;
			goto_layout(pLAYOUT(interphone));

			interphone_ring_play(6,NULL,false,interphone_ring_play_finsih_callback);
		}
	}
}


static void interphone_call_event_inside_func(unsigned long arg1,unsigned long arg2)
{

	network_device device = (network_device)arg2;
	if(arg1 == 1)
	{
		if(interphone_status == INTERPHONE_STATUS_IDLE)
		{
			interphone_call_mastar_device = (network_device)arg2;
			
			network_cmd_data data;
			data.device = interphone_call_mastar_device;
			data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			data.arg1 = 2;
			data.arg2 = (char)user_data_get()->other.network_device;
			network_send_cmd_data(&data);
		
			interphone_status = INTERPHONE_STATUS_IN;
			interphone_call_in_page_create();


			interphone_ring_play(6,NULL,false,interphone_ring_play_finsih_callback);
		}
	}
	else if(arg1 == 2)
	{
		if(interphone_status != INTERPHONE_STATUS_PUBLISH)
		{
			return ;
		}
		
		for(int i = 0 ; i < 3; i++)
		{
			if((device == interphone_index_device_get(i+1))&&(interphone_device_btn_status_flag[i] == 0x01))
			{
				interphone_device_btn_status_flag[i] = 0x02;
				interphone_status = INTERPHONE_STATUS_OUT;
			}
		}
		if(interphone_status == INTERPHONE_STATUS_OUT)
		{
			interphone_out_page_create();
			
			interphone_ring_play(6,NULL,false,interphone_ring_play_finsih_callback);
		}
	}
	else if(arg1 == 3)
	{
		if(interphone_status == INTERPHONE_STATUS_OUT)
		{
			for(int i = 0 ; i < 3; i++)
			{
				if((device == interphone_index_device_get(i+1))&&(interphone_device_btn_status_flag[i] == 0x02))
				{
					interphone_device_btn_status_flag[i] = 0x03;
					interphone_status = INTERPHONE_STATUS_TALK;
				}
				else if((interphone_device_btn_status_flag[i] == 0x02)||(interphone_device_btn_status_flag[i] == 0x01))
				{
					network_cmd_data data;
					data.device = interphone_index_device_get(i+1);
					data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
					data.arg1 = 4;
					data.arg2 = (char)user_data_get()->other.network_device;
					network_send_cmd_data(&data);
					interphone_device_btn_status_flag[i] = 0x00;
				}
			}

			if(interphone_status == INTERPHONE_STATUS_TALK)
			{
				interphone_call_talk_page_create();

				audio_play_stop_set();
				audio_talk_ctrl ctrl = {{device},(OPERATION_OPTION(AUDIO_SEND_EN)|
																				OPERATION_OPTION(AUDIO_RECEIVE_EN)|
																				OPERATION_OPTION(AUDIO_OUT_EN)|
																				OPERATION_OPTION(AUDIO_IN_EN)),AI_AO_O,true,true,70};
				audio_talk_open(ctrl);
			}
		}
	}
	else if(arg1 == 4)
	{
		if(interphone_status == INTERPHONE_STATUS_IN)
		{
			if(device == interphone_call_mastar_device)
			{
				goto_layout(pLAYOUT(home));
			}
		}
		else if(interphone_status == INTERPHONE_STATUS_OUT)
		{
			int out_device_num = 0;
			for(int i = 0 ; i < 3; i++)
			{
				if(interphone_device_btn_status_flag[i] == 0x02)
				{
					out_device_num++;
				}
			}
			if(out_device_num < 2)
			{
				goto_layout(pLAYOUT(home));
			}
		}
		else if(interphone_status == INTERPHONE_STATUS_TALK)
		{
		
			for(int i = 0 ; i < 3; i++)
			{
				if((device == interphone_index_device_get(i+1))&&(interphone_device_btn_status_flag[i] == 0x03))
				{
					goto_layout(pLAYOUT(home));
				}
			}	
		}
	}
}

void layout_interphone_init(void)
{
	interphone_call_event_register(interphone_call_event_extern_func);
}





CREATE_LAYOUT(interphone)
