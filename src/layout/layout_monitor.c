#include "layout_define.h"
#include "leo_api.h"

extern bool wifi_control;
static void monitor_Lock_1_btn_up(lv_obj_t *obj);
static MONITOR_CH montior_call_ch = MON_CH_NONE;
#define LV_MONITOR_UNLOCK_OBJ_ID 0X00
#define MONITOR_DURATION 30

typedef enum monitor_module_list
{
	SWITCH_MODULE,
	SANP_MODULE,
	RECORD_MODULE,
	LOCK1_MODULE,
	LOCK2_MODULE,
	SETTING_MODULE,
	TALK_MODULE,
	HAND_MODULE,
	TOTAL_MODULE
} monitor_module_list;

#define MONITOR_MODULE_COORDINATE_INIT { \
	{139, 483, 87, 87},                  \
	{236, 483, 87, 87},                  \
	{333, 483, 87, 87},                  \
	{430, 483, 87, 87},                  \
	{527, 483, 87, 87},                  \
	{624, 483, 87, 87},                  \
	{721, 483, 87, 87},                  \
	{818, 483, 87, 87},                  \
};

bool get_outdoor_talk_state(MONITOR_CH ch);
extern bool is_talking; // 正在接听
static bool is_tuya_talking = false;
extern void network_event_register(event_pro_callback handle);
static int is_tuya_enter;

static bool is_talked; // 接听过
static bool record_message_ing = false;
static lv_task_t *montior_talk_task_t = NULL;
static void monitor_enter_ui_different_processing(void);
static void monitor_record_label_display(bool is_record_video, bool hide);

bool is_light_status_on(void);

void light_status_set(bool is_on);
static void monitor_click_up(lv_obj_t *obj);

static void monitor_Lock_2_btn_up(lv_obj_t *obj);

static void monitor_light_control(bool open);

static void monitor_call_extern_func(unsigned long arg1, unsigned long arg2);
/*
static void network_event_extern_proc(unsigned long arg1, unsigned long arg2);
static void network_event_inside_proc(unsigned int arg1, unsigned int arg2);
static void tuya_event_extern_proc(unsigned int arg1, unsigned int arg2);
*/
static void monitor_call_inside_func(unsigned long ar1g, unsigned long arg2);

static void mechanical_key_extern_callback(unsigned long arg1, unsigned long arg2);
static void mechanical_key_inside_callback(unsigned long arg1, unsigned long arg2);

static void device_adc_key_callback(unsigned long arg1, unsigned long arg2);

static void monitor_channel_label_display(void);

extern bool is_video_recording(void);

static int monitor_timeout_val = MONITOR_DURATION;
static lv_task_t *monitor_timer_ptask = NULL;
static int monitor_ring_timeout_val;
static lv_task_t *monitor_ring_time_ptask = NULL;
static lv_task_t *monitor_chime_ptask = NULL;
// static bool is_tuya_app_busy = false;

bool monitor_message_ing(void)
{
	return record_message_ing;
}

static void monitor_timer_set(__int32_t time)
{
	monitor_timeout_val = time == 0 ? MONITOR_DURATION : time;
	Debug("monitor_timeout_val:%d\n", monitor_timeout_val);
}

bool is_monitor_talked(void)
{
	return is_talked;
}

static void monitor_video_mode_open(bool fb_video_enable)
{
	Debug("==============monitor_video_mode_open====>>>>%d\n\n\n", monitor_channel_get());
	monitor_switch(); // 打开指定通道的监控
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
	// system_bg_fill_color(0x00, 0, 0, 1024, 600); //填充颜色
	video_raw_clear();
	fb_video_mode_enable(fb_video_enable); // 打开
}

static void monitor_video_mode_close(void)
{
	Debug("==============monitor_video_mode_close====>>>>\n\n\n");
	fb_video_mode_enable(false);
	monitor_close();
	monitor_ring_timeout_val = 0;
	audio_play_stop_set();
}

static void monitor_record_jpeg_callback(unsigned long arg1, unsigned long arg2)
{
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 2);
	if (btn == NULL)
	{
		return;
	}
	Debug("%d\n", lv_obj_get_state(btn, LV_OBJ_PART_MAIN));
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SNAP_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_SNAP_FOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
	lv_obj_clear_state(btn, LV_STATE_FOCUSED);
	lv_obj_set_click(btn, true);

	Debug("===========================>\n");
	if (arg2 != REC_MODE_TUYA)
		monitor_record_label_display(false, true);
}

static void monitor_record_video_callback(unsigned long arg1, unsigned long arg2)
{
	Debug("=====================++++++>\n\r");
	if (is_video_recording())
		return;

	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 3);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORD_FOCUS_PNG);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORD_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
	lv_obj_clear_state(btn, LV_STATE_FOCUSED);

	monitor_record_label_display(true, true);

	if (arg2 == REC_MODE_MESSAGE)
		record_message_ing = false;
}

static lv_task_t *record_message_task_t = NULL;
static void record_message_task(lv_task_t *task_t)
{
	monitor_record_label_display(true, true);
	record_message_ing = true;
	if (record_video_start(REC_MODE_MESSAGE, true, monitor_channel_get()) == true)
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 3);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORDING_FOCUS_PNG);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORDING_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);

#ifdef AUTO_RECORD_TIME
		auto_rec_timer = 0;
		auto_record_colse_task_t = lv_task_create(auto_record_colse_task, 1000, LV_TASK_PRIO_HIGH, NULL);
#endif
	}

	lv_task_del(record_message_task_t);
	record_message_task_t = NULL;
}

static void record_message_action(door_info ch)
{
	record_message_ing = true;
	outdoor_order_set(NET_COMMON_PARAM_RECORD_MESSAGE | (user_data_get()->language.index << 2));
	if (record_message_task_t == NULL)
	{
		record_message_task_t = lv_task_create(record_message_task, 2000, LV_TASK_PRIO_HIGH, NULL);
		// lv_task_ready(record_message_task_t);
	}
}

static void monitor_timer_task(struct _lv_task_t *task_t)
{
	lv_obj_t *time_label = (lv_obj_t *)task_t->user_data;
	if (is_talking && monitor_channel_get() < MON_CH_CCTV_1)
	{
		int talk_vol = monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46;
		if (audio_output_volume_get() != talk_vol)
			audio_output_volume_set(talk_vol);
	}
	// Debug("monitor_enter_way_get():%d\n",monitor_enter_way_get());
	if (monitor_enter_way_get() == MONITOR_ENTER_TUYA)
	{
		lv_label_set_text(time_label, text_str(STR_APP_PREVIEW));
		outdoor_order_set(NET_COMMON_PARAM_CAMERA_TUYA);
	}
	else if (is_jpg_record_ing())
	{
		lv_label_set_text_fmt(time_label, "%s %02d", text_str(STR_SNAPSHOT), monitor_timeout_val);
	}
	else if (monitor_enter_way_get() == MONITOR_ENTER_CALL && montior_call_ch)
	{
		lv_label_set_text_fmt(time_label, "%s %02d", (montior_call_ch - 1) ? text_str(STR_DOOR2_CALL) : text_str(STR_DOOR1_CALL), monitor_timeout_val);
	}
	else if (record_message_ing && is_talked == false /*  && sd_free_space_insufficient() */)
	{
		lv_label_set_text_fmt(time_label, "%s %02d", text_str(STR_MESSAGE), monitor_timeout_val);
	}
	else if (is_video_recording())
	{
		lv_label_set_text_fmt(time_label, "%s %02d", text_str(STR_REC), monitor_timeout_val);
	}
	else
	{
		// Debug("record_message_ing :%d    is_talked:%d\n\r",record_message_ing,is_talked);
		lv_label_set_text_fmt(time_label, "%02d", monitor_timeout_val);
	}
	lv_obj_align(time_label, time_label->parent, LV_ALIGN_IN_TOP_RIGHT, -40, 20);
	static int sec = 0;

	if ((sec = !sec) && monitor_enter_way_get() != MONITOR_ENTER_TUYA)
	{
		if (is_audio_talk_open() == AI_AO_C)
		{
		}
		// monitor_default_camera_busy_display();

		if (monitor_timeout_val == 0)
		{
			record_video_stop(0x00); // 停止录像
			goto_layout(pLAYOUT(standby));
		}
		else
		{
#ifndef APP_ATS_OPEN
			monitor_timeout_val--;
#endif
		}
	}

	if (is_talking)
	{
		extern void talk_led_gpio_control(bool en);
		talk_led_gpio_control(true);
	}
}

static lv_obj_t *montior_busy_msg_box = NULL;
static void monitor_device_busy_msg_btn_up(lv_obj_t *obj)
{
	if (montior_busy_msg_box != NULL)
	{
		lv_obj_del(montior_busy_msg_box);
		montior_busy_msg_box = NULL;
	}
}

static lv_obj_t *monitor_busy_msgbox_create(char *str)
{

	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);

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
	lv_label_set_text(window_head_label, str);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_align(window_head_label, msgbox_cont, LV_ALIGN_CENTER, 0, -30);

	lv_obj_t *window_ok_btn = lv_btn_create(msgbox_cont, NULL);
	lv_obj_set_size(window_ok_btn, 160, 48);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_COMFIRM_PNG);

	static btn_data btn_data1 = {0};
	btn_data1.OPS_UP = monitor_device_busy_msg_btn_up;
	lv_obj_set_style_local_pattern_image(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
	// lv_obj_set_style_local_pattern_image(window_ok_btn,LV_OBJ_PART_MAIN,LV_STATE_FOCUSED,&info2);
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_align(window_ok_btn, msgbox_cont, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	return window_cont;
}
static void monitor_indoor_cmd_func(unsigned long arg1, unsigned long arg2)
{
	char cmd = (arg1 >> 8) & 0xFF;
	// char info = (arg1)&0xFF;
	// network_device in_device = (arg2>>8)&0xFF;
	network_device out_device = arg2 & 0xFF;

	MONITOR_CH ch = monitor_channel_get();
	MONITOR_CH out_ch = out_device == DEVICE_OUTDOOR_1 ? MON_CH_DOOR_1 : MON_CH_DOOR_2;

	printf("======net_common_outdoor_hand_func= %d ,%d======>>>\n\n", cmd, out_ch);
	fflush(stdout);
	switch (cmd)
	{
	case 1: /* in device 与 out_device 通话 */
		if (((is_talked == true) && (out_ch != ch)) || monitor_enter_way_get() == MONITOR_ENTER_TUYA)
		{
			/*不做任何处理*/
		}
		else
		{
			goto_layout(pLAYOUT(standby));
		}
		break;
	case 2: /*设备正忙，in device与 out device 通话*/
	{
		if (montior_talk_task_t != NULL)
		{
			/* 删除预通话任务*/
			lv_task_del(montior_talk_task_t);
			montior_talk_task_t = NULL;
		}
		if (montior_busy_msg_box == NULL)
		{
			montior_busy_msg_box = monitor_busy_msgbox_create(text_str(STR_DEVICE_BUSY));
		}
	}
	break;
	default:
		printf("Parameter error :%d\n\r", cmd);
		break;
	}
}

#include "tuya_ipc_p2p.h"
/* static lv_task_t *monitor_tuya_exit_task_t = NULL;
static void monitor_tuya_exit_task(lv_task_t *task_t)
{
		Debug("=================+>>>>>>\n");
		int on_line = tuya_client_num_get();
		if(on_line <1 && current_layout_get() == &layout_monitor && monitor_enter_way_get() == MONITOR_ENTER_TUYA)
		{
			tuya_event_state_set(TRANS_LIVE_VIDEO_STOP);
			send_monitor_hang_cmd();
			goto_layout(pLAYOUT(standby));
		}

		if(monitor_tuya_exit_task_t)
		{
			lv_task_del(monitor_tuya_exit_task_t);
			monitor_tuya_exit_task_t = NULL;
		}
		Debug("=================+>>>>>>\n");
} */
#if 0
static lv_task_t *unlock_task_t = NULL;
static void monitor_unlock_task(lv_task_t *task_t)
{
	unlock_gpio_set(0);
	lv_task_del(unlock_task_t);
	unlock_task_t = NULL;
}
#endif
void tuya_switch_camera(char channel)
{
	MONITOR_CH ch = monitor_channel_get(); // 获取当前通道
	printf("tuya switch ch %d -------------------------->channel %d   \n\r", ch, channel);
	if (ch != channel)
	{
		// send_monitor_talk_cmd(true);
		monitor_channel_set(channel);
		// audio_push_to_tuya_close();
		audio_talk_close(true);

		if (channel < MON_CH_CCTV_1)
		{
			request_send_I_frame_cmd(channel == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2);
			network_device devcie = channel + DEVICE_INDOOR_ID6;
			audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, channel == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
			audio_talk_open(ctrl);
			// if(is_tuya_talking)
			// 	send_monitor_talk_cmd(true);
		}
		// audio_pull_to_local_close();

		// tuya_ipc_ring_buffer_video_release_data();
		monitor_switch(); // monitor_open(true);//
	}
	tuya_switch_channel_upload_results(channel);
}

void tuya_event_inside_proc(unsigned long arg1, unsigned long arg2)
{
	tuya_event ev = (tuya_event)arg1;
	Debug("1111:%d\n\n\n", ev);
	switch (ev)
	{
	/*切换监控*/
	case TUYA_EVENT_MONITOR_SWAP:
	{
		Debug("TUYA_EVENT_MONITOR_SWAP ====================+>>>>\n\n\n");
		tuya_switch_camera(arg2);
		monitor_channel_label_display(); // 通道的文本显示
		break;
	}

	case TUYA_EVENT_LIGHT_ON:
	{
		Debug("TUYA_EVENT_LIGHT_ON ====================+>>>>\n\n\n");
		monitor_light_control(arg2);
		break;
	}
		/*开锁*/
	case TUYA_EVENT_OPEN_LOCK:
	{
		tuya_unlock_start();
		break;
	}

	/* 开锁2*/
	case TUYA_EVENT_OPEN_GATE1:
	{
#if defined(PUBLIC_VERSION)
		if (user_data_get()->tuya_info.lock_id == false)
		{
			tuya_ungate1_start();
		}
		else
		{
			tuya_ungate2_start();
		}
#else
		tuya_ungate1_start();
#endif
		break;
	}
	case TUYA_EVENT_OPEN_GATE2:
	{
		// Debug("=======>>CCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\n\n");
		tuya_ungate2_start();
		break;
	}
		/*通话*/
	case TUYA_EVENT_TALK:
		if (arg2 == true)
		{
			MONITOR_CH monitor_ch = monitor_channel_get();
			Debug("=======>>TUYA_EVENT_TALK CH:%d\n\n\n", monitor_ch);
			if (monitor_ch < MON_CH_CCTV_1)
			{
				network_device devcie = monitor_ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
				audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
				audio_talk_open(ctrl);
				send_monitor_talk_cmd(true);
				is_talking = false;
				is_tuya_talking = true;
			}
		}
		// else
		// {
		// 	send_monitor_talk_cmd(false);
		// 	is_talking = false;
		// 	is_tuya_talking = false;
		// }
		break;
		/*进入监控*/
	case TUYA_EVENT_MONITOR_ENTER:
		/*Don't do anything*/
		{
			Debug("TUYA_EVENT_MONITOR_ENTER ====================+>>>>\n\n\n");
			if (device_online_state_get(monitor_channel_get() + DEVICE_INDOOR_ID6) == false)
			{
				audio_talk_close(false);

				for (int ch = DEVICE_OUTDOOR_1; ch < DEVICE_END; ch++)
				{
					if (device_online_state_get(ch))
					{

						monitor_channel_set(MON_CH_DOOR_1 + ch - DEVICE_OUTDOOR_1);
						break;
					}
				}

				monitor_switch(); // monitor_open(true);//
								  // break;
			}

			monitor_enter_way_set(MONITOR_ENTER_TUYA);
			is_tuya_enter = 1;
			// send_monitor_talk_cmd(true);
			record_video_stop(0x00);
			tuya_channel_valid_report();
			monitor_enter_ui_different_processing();
			monitor_channel_label_display();

			MONITOR_CH monitor_ch = monitor_channel_get();
			network_device devcie = monitor_ch + DEVICE_INDOOR_ID6;
			if (monitor_ch != MON_CH_CCTV_1 && monitor_ch != MON_CH_CCTV_2)
			{
				audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
				audio_talk_open(ctrl);
			}

			if (monitor_ring_time_ptask != NULL)
			{
				lv_task_del(monitor_ring_time_ptask);
				monitor_ring_time_ptask = NULL;
			}
			monitor_ring_timeout_val = 0;
			audio_play_stop_set();
			while (video_decode_data_status() != true)
			{
				// Debug("video_decode_data_status\n");
				if (get_video_decode_state() == false)
					break;
			};
			video_raw_clear();
			fb_video_mode_enable(false);
		}
		break;
		/*退出监控*/
	case TUYA_EVENT_MONITOR_QUIT:
	{
		// if(monitor_tuya_exit_task_t == NULL)
		// 	monitor_tuya_exit_task_t = lv_task_create(monitor_tuya_exit_task, 500, LV_TASK_PRIO_HIGH, NULL);

		send_monitor_hang_cmd();
		monitor_channel_set(MON_CH_NONE);
		goto_layout(pLAYOUT(standby));
	}
	break;
	/* 截屏 */
	case TUYA_EVENT_SCREENSHOT:
	{
		extern int tuya_dp_236_response_screenshot(bool state);
		extern void screen_capture();
		screen_capture();
		tuya_dp_236_response_screenshot(false);
	}
	break;

	case TUYA_EVENT_MONITOR_ING:
	{
		if (current_layout_get() != &layout_standby && monitor_enter_way_get() != MONITOR_ENTER_TUYA)
		{
			goto_layout(pLAYOUT(standby));
		}
	}
	break;
	case TUYA_EVENT_MQTT_OFFLINE:
	{
		if (user_data_get()->wifi.wifi_connect_flag)
		{
			system("/app/app/hi3881_reload.sh");
		}
	}
	break;
	default:
		/*Don't do anything*/
		break;
	}
	return;
}

void tuya_event_extern_proc(unsigned long arg1, unsigned long arg2)
{
	tuya_event ev = (tuya_event)arg1;
	switch (ev)
	{
	/*切换监控*/
	case TUYA_EVENT_MONITOR_SWAP:
		// tuya_ipc_ring_buffer_video_release_data();
		// tuya_switch_camera(arg2);
		//  tuya_ipc_ring_buffer_video_release_data();
		break;
		/*开锁*/
	case TUYA_EVENT_OPEN_LOCK:
		// start_door_unlock();
		break;
	case TUYA_EVENT_WORK_MODE:
		user_data_get()->other.model = arg2 == 0 ? 2 : arg2 - 1;
		if (current_layout_get() == &layout_home)
		{
			extern void home_Model_btn_switch(void);
			home_Model_btn_switch();
		}
		Debug("TUYA_EVENT_WORK_MODE ====================+>>>>%d\n\n\n", user_data_get()->other.model);
		break;
		/*通话*/
	case TUYA_EVENT_TALK:
		send_monitor_talk_cmd(true);
		break;
		/*进入监控*/
	case TUYA_EVENT_MONITOR_ENTER:
	{
		is_tuya_enter = 1;
		monitor_channel_set(MON_CH_DOOR_1);
		for (int ch = DEVICE_OUTDOOR_1; ch < DEVICE_END; ch++)
		{
			if (device_online_state_get(ch))
			{
				monitor_channel_set(MON_CH_DOOR_1 + ch - DEVICE_OUTDOOR_1);
				break;
			}
		}
		Debug("TUYA_EVENT_MONITOR_ENTER ====================+>>>>%d\n\n\n", monitor_channel_get());
		tuya_channel_valid_report();
		monitor_enter_way_set(MONITOR_ENTER_TUYA);
		goto_layout(pLAYOUT(monitor));

		MONITOR_CH monitor_ch = monitor_channel_get();
		Debug("TUYA_EVENT_MONITOR_ENTER ====================+>>>>%d\n\n\n", monitor_ch);
		network_device devcie = monitor_ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
		audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
		audio_talk_open(ctrl);

		fb_video_mode_enable(false);
		// send_monitor_talk_cmd(true);
	}
	break;
		/*退出监控*/
	case TUYA_EVENT_MONITOR_QUIT:
		/*Don't do anything*/
		break;
	/* 截屏 */
	case TUYA_EVENT_SCREENSHOT:
	{
		extern int tuya_dp_236_response_screenshot(bool state);
		extern void screen_capture();
		screen_capture();
		tuya_dp_236_response_screenshot(false);
	}
	case TUYA_EVENT_MONITOR_ING:
	{
		if (current_layout_get() != &layout_standby && monitor_enter_way_get() != MONITOR_ENTER_TUYA)
		{
			goto_layout(pLAYOUT(standby));
		}
	}
	break;
	case TUYA_EVENT_MQTT_OFFLINE:
	{
		if (user_data_get()->wifi.wifi_connect_flag)
		{
			system("/app/app/hi3881_reload.sh");
		}
	}
	break;
	default:
		/*Don't do anything*/
		break;
	}
	return;
}

static int get_monitor_time(void)
{
	int time = MONITOR_DURATION;
	if (monitor_enter_way_get() != MONITOR_ENTER_CALL)
	{
		time = MONITOR_DURATION;
	}
	else if (monitor_channel_get() == MON_CH_DOOR_1)
	{
		if (user_data_get()->other.model == NOT_AT_HOME_PATTERN)
		{
			time = user_data_get()->door1.message_time;
		}
		else
		{
			if (user_data_get()->door1.message_sw)
			{
				time = ring_attr.door1.ring_time + user_data_get()->door1.message_time;
			}
			else
			{
				time = ring_attr.door1.ring_time;
			}
		}
	}
	else if (monitor_channel_get() == MON_CH_DOOR_2)
	{
		if (user_data_get()->other.model == NOT_AT_HOME_PATTERN)
		{
			time = user_data_get()->door2.message_time;
		}
		else
		{
			if (user_data_get()->door2.message_sw)
			{
				time = ring_attr.door2.ring_time + user_data_get()->door2.message_time;
			}
			else
			{
				time = ring_attr.door2.ring_time;
			}
		}
	}

	return time;
}

static int get_monitor_brightness(void)
{
	switch (monitor_channel_get())
	{
	case MON_CH_DOOR_1:

		return user_data_get()->door1.brightness;
	case MON_CH_DOOR_2:

		return user_data_get()->door2.brightness;
	case MON_CH_CCTV_1:

		return user_data_get()->camera1.brightness;
	case MON_CH_CCTV_2:

		return user_data_get()->camera2.brightness;

	default:
		return user_data_get()->other.brightness;
	}
}

static int set_monitor_brightness(int bri)
{
	switch (monitor_channel_get())
	{
	case MON_CH_DOOR_1:
		user_data_get()->door1.brightness = bri;
		break;
	case MON_CH_DOOR_2:

		user_data_get()->door2.brightness = bri;
		break;
	case MON_CH_CCTV_1:

		user_data_get()->camera1.brightness = bri;
		break;
	case MON_CH_CCTV_2:

		user_data_get()->camera2.brightness = bri;
		break;

	default:
		break;
	}
	return bri;
}
static void monitor_enter_parameter_init(void)
{
	if (is_sdcard_insert() == false)
	{
		user_data_get()->door1.message_sw = user_data_get()->door2.message_sw = false;
	}
	monitor_timer_set(get_monitor_time());
	record_jpeg_event_register(monitor_record_jpeg_callback);
	record_video_event_register(monitor_record_video_callback);
	outdoor_call_event_register(monitor_call_inside_func);
	indoor_cmd_event_register(monitor_indoor_cmd_func);
	tuya_event_register(tuya_event_inside_proc);
	mechanical_key_event_register(mechanical_key_inside_callback);
	device_adc_key_register(device_adc_key_callback);
	backlight_open(true, true, get_monitor_brightness());
}

static lv_obj_t *montior_channel_label = NULL;
static void monitor_channel_label_display(void)
{
	MONITOR_CH ch = monitor_channel_get();
	if (ch == MON_CH_DOOR_1)
	{
		lv_label_set_text(montior_channel_label, text_str(STR_DOOR1));
	}
	else if (ch == MON_CH_DOOR_2)
	{
		lv_label_set_text(montior_channel_label, text_str(STR_DOOR2));
	}
	else if (ch == MON_CH_CCTV_1)
	{
		lv_label_set_text(montior_channel_label, text_str(STR_CAMERA1));
	}
	else if (ch == MON_CH_CCTV_2)
	{
		lv_label_set_text(montior_channel_label, text_str(STR_CAMERA2));
	}
}

static void monitor_call_label_display(MONITOR_CH ch, bool hide)
{
	if (hide)
	{
		montior_call_ch = MON_CH_NONE;
	}
	else
	{
		montior_call_ch = ch;
	}

	return;
}

static lv_obj_t *montior_record_label = NULL;
static void monitor_record_label_display(bool is_record_video, bool hide)
{
	return;
	Debug("=====================++++++>%d\n\r", hide);
	if (hide)
	{
		lv_obj_set_hidden(montior_record_label, true);
		lv_obj_set_click(montior_record_label, true);
		return;
	}
	else
	{
		lv_obj_set_hidden(montior_record_label, false);
	}

	if (is_record_video)
	{
		lv_label_set_text(montior_record_label, text_str(STR_REC));
	}
	else
	{
		lv_label_set_text(montior_record_label, text_str(STR_SNAPSHOT));
	}
}

static lv_task_t *monitor_time_ptask = NULL;
static void monitor_time_task(struct _lv_task_t *task_t)
{
	lv_obj_t *time_label = (lv_obj_t *)task_t->user_data;
	struct ak_date date;
	static bool colon = false;
	ak_get_localdate(&date);
	lv_obj_set_style_local_text_font(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	if (colon)
	{
		colon = false;
		lv_label_set_text_fmt(time_label, "%02d/%02d  %02d:%02d", date.month + 1, date.day + 1, date.hour, date.minute);
	}
	else
	{
		colon = true;
		lv_label_set_text_fmt(time_label, "%02d/%02d  %02d %02d", date.month + 1, date.day + 1, date.hour, date.minute);
	}
	lv_obj_align(time_label, time_label->parent, LV_ALIGN_CENTER, -150, 0);
}

static void monitor_2_head_create(void)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_cont_set_layout(cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(cont, LV_FIT_NONE);
	lv_obj_set_pos(cont, 0, 0);
	lv_obj_set_size(cont, 1024, 60);
	lv_obj_set_style_local_bg_opa(cont, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
	lv_obj_set_style_local_bg_color(cont, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
	lv_obj_set_auto_realign(cont, true);

	lv_obj_t *time_label = lv_label_create(cont, NULL);
	lv_label_set_long_mode(time_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_align(time_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(time_label, cont, LV_ALIGN_CENTER, 0, 0);
	if (monitor_time_ptask != NULL)
	{
		lv_task_del(monitor_time_ptask);
	}
	monitor_time_ptask = lv_task_create(monitor_time_task, 1000, LV_TASK_PRIO_MID, time_label);
	lv_task_ready(monitor_time_ptask);
	// monitor_time_task(monitor_time_ptask);

	montior_channel_label = lv_label_create(cont, NULL);
	lv_obj_set_size(montior_channel_label, 120, 60);

	lv_label_set_align(montior_channel_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_font(montior_channel_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_align(montior_channel_label, cont, LV_ALIGN_IN_LEFT_MID, 30, 0);

	monitor_channel_label_display();

	montior_record_label = lv_label_create(cont, NULL);
	lv_obj_set_size(montior_record_label, 120, 60);

	lv_label_set_align(montior_record_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_color(montior_record_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xdd3d3d));
	lv_obj_set_style_local_text_font(montior_record_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_align(montior_record_label, cont, LV_ALIGN_IN_RIGHT_MID, -140, 0);
	lv_obj_set_hidden(montior_record_label, true);

	lv_obj_t *timer_label = lv_label_create(cont, NULL);
	lv_label_set_align(timer_label, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(timer_label, LV_LABEL_LONG_EXPAND);
	lv_obj_set_style_local_text_color(timer_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xdd3d3d));
	lv_obj_set_style_local_text_font(timer_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));

	if (monitor_timer_ptask != NULL)
	{
		lv_task_del(monitor_timer_ptask);
	}
	monitor_timer_ptask = lv_task_create(monitor_timer_task, 500, LV_TASK_PRIO_HIGHEST, timer_label);
	lv_task_ready(monitor_timer_ptask);

	monitor_timer_task(monitor_timer_ptask);
}

static MONITOR_CH monitor_next_ch(void)
{
	MONITOR_CH ch = monitor_channel_get();
	if (ch == MON_CH_DOOR_1)
	{
		ch = user_data_get()->camera1.enable && user_data_get()->camera1.url[0] == 'r' ? MON_CH_CCTV_1 : (user_data_get()->door2.enable_sw ? MON_CH_DOOR_2 : user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2
																																																									   : MON_CH_DOOR_1);
	}
	else if (ch == MON_CH_CCTV_1)
	{
		ch = user_data_get()->door2.enable_sw ? MON_CH_DOOR_2 : (user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2 : MON_CH_DOOR_1);
	}
	else if (ch == MON_CH_DOOR_2)
	{
		ch = user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2 : MON_CH_DOOR_1;
	}
	else if (ch == MON_CH_CCTV_2)
	{
		ch = MON_CH_DOOR_1;
	}
	return ch;
}
static void monitor_switch_btn_up(lv_obj_t *obj)
{
	record_video_stop(0x00);
	monitor_call_label_display(MON_CH_NONE, true);
	monitor_click_up(NULL);
	monitor_timer_set(MONITOR_DURATION);
	monitor_enter_way_set(MONITOR_ENTER_MANUAL);
	MONITOR_CH ch = monitor_channel_get();

#ifdef CAMERA_MODULE_ENABLE
	if (wifi_usb_module_enable())
	{
		if (ch == MON_CH_DOOR_1)
		{
			ch = user_data_get()->camera1.enable && user_data_get()->camera1.url[0] == 'r' ? MON_CH_CCTV_1 : (user_data_get()->door2.enable_sw ? MON_CH_DOOR_2 : user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2
																																																										   : MON_CH_DOOR_1);
		}
		else if (ch == MON_CH_CCTV_1)
		{
			ch = user_data_get()->door2.enable_sw ? MON_CH_DOOR_2 : (user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2 : MON_CH_DOOR_1);
		}
		else if (ch == MON_CH_DOOR_2)
		{
			ch = user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2 : MON_CH_DOOR_1;
		}
		else if (ch == MON_CH_CCTV_2)
		{
			ch = MON_CH_DOOR_1;
		}
	}
	else
	{
		ch = ch == MON_CH_DOOR_1 ? MON_CH_DOOR_2 : MON_CH_DOOR_1;
	}
#else
	ch = ch == MON_CH_DOOR_1 ? MON_CH_DOOR_2 : MON_CH_DOOR_1;
#endif

	audio_talk_close(false);
	if (is_talking)
	{
		audio_talk_ctrl ctrl = {{ch + DEVICE_INDOOR_ID6}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN) | OPERATION_OPTION(AUDIO_OUT_EN) | OPERATION_OPTION(AUDIO_IN_EN)), AI_AO_O, true, true, ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
		audio_talk_open(ctrl);
	}
	else
	{
		audio_talk_ctrl ctrl = {{ch + DEVICE_INDOOR_ID6}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
		audio_talk_open(ctrl);
	}

	record_message_ing = false;

	monitor_channel_set(ch);
	monitor_switch(); // monitor_open(true);//

	monitor_channel_label_display();
	monitor_enter_ui_different_processing();

	backlight_open(true, true, get_monitor_brightness());

	outdoor_order_set(NET_COMMON_CMD_NONE);
	Debug("is_talking :%d  ch:%d\n", is_talking, ch);
	if (is_talking && ch < MON_CH_CCTV_1)
	{
		send_monitor_talk_cmd(true);
	}
}

static void monitor_switch_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_switch_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_MONITOR_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_MONITOR_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 1);
}
extern bool get_video_data_display_state(void);

static void monitor_snap_photo_btn_up(lv_obj_t *obj)
{
	monitor_click_up(NULL);
	if (!get_video_data_display_state())
		return;

	if (is_sdcard_insert() == false)
	{
		if (sdcard_insert_msg_box == NULL)
		{
			sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_PLEASE_INSERT_SD));
		}
		return;
	}

	int free_space = sd_free_space_insufficient();
	if (free_space < 500)
	{
		if (sdcard_insert_msg_box == NULL)
		{
			sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
			record_message_ing = false;
		}
		// return;
	}

	if (free_space > 200 && record_pictrue_start(REC_MODE_MANUAL, monitor_channel_get()) == true) // 手动从当前通道拍照
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 2);
		Debug("%d\n", lv_obj_get_state(btn, LV_OBJ_PART_MAIN));
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SNAPING_UNFOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_SNAPING_FOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
		lv_obj_add_state(btn, LV_STATE_FOCUSED);
		lv_obj_set_click(btn, false);

		monitor_record_label_display(false, false);
	}
	else
	{
		if (free_space < 200)
		{
			extern void detect_sd_free_space(void);
			detect_sd_free_space();
		}
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 2);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SNAP_UNFOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_SNAP_FOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
		lv_obj_clear_state(btn, LV_STATE_FOCUSED);
		lv_obj_set_click(btn, true);
	}
}

static void monitor_snap_photo_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_snap_photo_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SNAP_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_SNAP_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 2);
}

static void monitor_record_video_btn_up(lv_obj_t *obj)
{
	monitor_click_up(NULL);
	if (!get_video_data_display_state())
		return;

	if (is_sdcard_insert() == false)
	{
		if (sdcard_insert_msg_box == NULL)
		{
			sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_PLEASE_INSERT_SD));
		}
		return;
	}

	int free_space = sd_free_space_insufficient();
	if (free_space < 500)
	{
		if (sdcard_insert_msg_box == NULL)
		{
			sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
			record_message_ing = false;
		}
		// return;
	}

	if (is_video_recording() == false && free_space > 200) // 如果已经未开始录像
	{
		if (record_video_start(REC_MODE_MANUAL, true, monitor_channel_get()) == true) // 那么开始录像
		{
			lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 3);
			static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORDING_FOCUS_PNG);
			static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORDING_UNFOCUS_PNG);
			lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
			lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
			lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
			lv_obj_add_state(btn, LV_STATE_FOCUSED);

			monitor_record_label_display(true, false);
		}
	}
	else
	{
		if (free_space < 200)
		{
			extern void detect_sd_free_space(void);
			detect_sd_free_space();
		}
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 3);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORD_UNFOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORD_FOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
		record_video_stop(0x00); // 停止录像
	}
}

static void monitor_record_video_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_record_video_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORD_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORD_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 3);
}

lv_obj_t *monitor_window_btn_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src1, const void *img_src2)
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
	}
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, img_src2);

	btn->user_data = btn_pdata;
	btn_touch_event_listen(btn);
	return btn;
}

static void window_volume_add_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (obj == lv_obj_get_child_form_id(window_cont, 53))
	{
		int *vol = monitor_channel_get() == MON_CH_DOOR_1 ? &user_data_get()->door1.out_talk_volume : &user_data_get()->door2.out_talk_volume;
		if (++(*vol) > 10)
		{
			*vol = 10;
		}
		lv_obj_t *cont = lv_obj_get_child_form_id(window_cont, 11);
		lv_obj_set_size(cont, 14 * (*vol), 12);

		lv_obj_t *label = lv_obj_get_child_form_id(window_cont, 100);
		static char buf[4] = {0};
		sprintf(buf, "%02d", *vol);
		lv_label_set_text(label, buf);
	}
	else if (obj == lv_obj_get_child_form_id(window_cont, 54))
	{
		int *vol = monitor_channel_get() == MON_CH_DOOR_1 ? &user_data_get()->door1.talk_volume : &user_data_get()->door2.talk_volume;
		if (++*vol > 10)
		{
			*vol = 10;
		}

		lv_obj_t *cont = lv_obj_get_child_form_id(window_cont, 22);
		lv_obj_set_size(cont, 14 * *vol, 12);

		lv_obj_t *label = lv_obj_get_child_form_id(window_cont, 101);
		static char buf[4] = {0};
		sprintf(buf, "%02d", *vol);
		lv_label_set_text(label, buf);
		audio_volume_set((*vol));
		// user_data_save();
	}
}
static void window_indoor_volume_add_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_volume_add_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_VOLUME_ADD_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_VOLUME_ADD_UNFOCUS_PNG);
	lv_obj_t *btn = monitor_window_btn_create(parent, 222, 94, 38, 38, &btn_data, &info1, &info);
	lv_obj_set_ext_click_area(btn, 20, 20, 20, 20);
	lv_obj_set_id(btn, 54);
	btn_data.obj_tone = false;
}

static void window_outdoor_volume_add_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_volume_add_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_OUT_VOLUME_ADD_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_OUT_VOLUME_ADD_UNFOCUS_PNG);
	lv_obj_t *btn = monitor_window_btn_create(parent, 222, 28, 38, 38, &btn_data, &info1, &info);
	lv_obj_set_ext_click_area(btn, 20, 20, 20, 20);
	lv_obj_set_id(btn, 53);
	btn_data.obj_tone = false;
}

static void window_volume_subtract_btn_up(lv_obj_t *obj)
{

	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (obj == lv_obj_get_child_form_id(window_cont, 55))
	{
		int *vol = monitor_channel_get() == MON_CH_DOOR_1 ? &user_data_get()->door1.out_talk_volume : &user_data_get()->door2.out_talk_volume;
		if (--(*vol) < 1)
		{
			*vol = 1;
		}
		lv_obj_t *cont = lv_obj_get_child_form_id(window_cont, 11);
		lv_obj_set_size(cont, 14 * (*vol), 12);

		lv_obj_t *label = lv_obj_get_child_form_id(window_cont, 100);
		static char buf[4] = {0};
		sprintf(buf, "%02d", *vol);
		lv_label_set_text(label, buf);
	}
	else if (obj == lv_obj_get_child_form_id(window_cont, 56))
	{
		int *vol = monitor_channel_get() == MON_CH_DOOR_1 ? &user_data_get()->door1.talk_volume : &user_data_get()->door2.talk_volume;
		if (--*vol < 0)
		{
			*vol = 0;
		}

		lv_obj_t *cont = lv_obj_get_child_form_id(window_cont, 22);
		lv_obj_set_size(cont, 14 * (*vol), 12);

		lv_obj_t *label = lv_obj_get_child_form_id(window_cont, 101);
		static char buf[4] = {0};
		sprintf(buf, "%02d", *vol);
		lv_label_set_text(label, buf);
		audio_volume_set(*vol);
		// user_data_save();
	}
}
static void window_indoor_volume_subtract_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_volume_subtract_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_VOLUME_DEL_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_VOLUME_DEL_UNFOCUS_PNG);
	lv_obj_t *btn = monitor_window_btn_create(parent, 24, 94, 38, 38, &btn_data, &info1, &info);
	lv_obj_set_ext_click_area(btn, 20, 20, 20, 20);
	lv_obj_set_id(btn, 56);
	btn_data.obj_tone = false;
}

static void window_outdoor_volume_subtract_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_volume_subtract_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_OUT_VOLUME_DEL_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_OUT_VOLUME_DEL_UNFOCUS_PNG);
	lv_obj_t *btn = monitor_window_btn_create(parent, 24, 28, 38, 38, &btn_data, &info1, &info);
	lv_obj_set_ext_click_area(btn, 20, 20, 20, 20);
	lv_obj_set_id(btn, 55);
	btn_data.obj_tone = false;
}

static void window_brightness_add_btn_up(lv_obj_t *obj)
{
	if (5 < set_monitor_brightness(get_monitor_brightness() + 1))
	{
		set_monitor_brightness(5);
	}
	int bri = get_monitor_brightness();
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);

	lv_obj_t *cont = lv_obj_get_child_form_id(window_cont, 44);
	lv_obj_set_size(cont, 28 * bri, 12);

	lv_obj_t *label = lv_obj_get_child_form_id(window_cont, 102);
	static char buf[4] = {0};
	sprintf(buf, "%02d", bri);
	lv_label_set_text(label, buf);

	backlight_open(true, true, bri);
	// user_data_save();
}
static void window_brightness_add_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_brightness_add_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_BRIGHTNESS_ADD_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_BRIGHTNESS_ADD_UNFOCUS_PNG);
	lv_obj_t *btn;
	if (monitor_channel_get() < MON_CH_CCTV_1)
		btn = monitor_window_btn_create(parent, 222, 160, 30, 30, &btn_data, &info1, &info);
	else
		btn = monitor_window_btn_create(parent, 222, 33, 30, 30, &btn_data, &info1, &info);
	lv_obj_set_ext_click_area(btn, 20, 20, 20, 20);
}

static void window_brightness_subtract_btn_up(lv_obj_t *obj)
{
	if (set_monitor_brightness(get_monitor_brightness() - 1) < 1)
	{
		set_monitor_brightness(1);
	}
	int bri = get_monitor_brightness();
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);

	lv_obj_t *cont = lv_obj_get_child_form_id(window_cont, 44);
	lv_obj_set_size(cont, 28 * bri, 12);

	lv_obj_t *label = lv_obj_get_child_form_id(window_cont, 102);
	static char buf[4] = {0};
	sprintf(buf, "%02d", bri);
	lv_label_set_text(label, buf);

	backlight_open(true, true, bri);
	// user_data_save();
}
static void window_brightness_subtract_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, window_brightness_subtract_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_BRIGHTNESS_DEL_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_BRIGHTNESS_DEL_UNFOCUS_PNG);
	lv_obj_t *btn;
	if (monitor_channel_get() < MON_CH_CCTV_1)
		btn = monitor_window_btn_create(parent, 22, 160, 34, 30, &btn_data, &info1, &info);
	else
		btn = monitor_window_btn_create(parent, 22, 33, 34, 30, &btn_data, &info1, &info);
	lv_obj_set_ext_click_area(btn, 20, 20, 20, 20);
}

void monitor_setting_window_create(void)
{
	int out_vol = monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.out_talk_volume : user_data_get()->door2.out_talk_volume;
	int in_vol = monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume : user_data_get()->door2.talk_volume;

	int bri = get_monitor_brightness();

	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_style_local_bg_color(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1D1D1D));
	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
	lv_obj_set_style_local_radius(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 40);
	if (monitor_channel_get() < MON_CH_CCTV_1)
		set_location(window_cont, 350, 187, 324, 226);
	else
		set_location(window_cont, 350, 269, 324, 90);
	// lv_cont_set_layout(window_cont, LV_FIT_NONE);
	lv_obj_set_id(window_cont, 888);

	lv_obj_t *bri_box = lv_cont_create(window_cont, NULL);
	lv_obj_set_style_local_bg_color(bri_box, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xc4c4c4));
	lv_obj_set_style_local_bg_opa(bri_box, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(bri_box, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
	if (monitor_channel_get() < MON_CH_CCTV_1)
		set_location(bri_box, 66, 168, 144, 16);
	else
		set_location(bri_box, 66, 39, 144, 16);

	lv_obj_t *bri_bar = lv_cont_create(window_cont, bri_box);
	lv_obj_set_style_local_bg_color(bri_bar, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x02A9FF));

	if (monitor_channel_get() < MON_CH_CCTV_1)
		set_location(bri_bar, 68, 170, 28 * bri, 12);
	else
		set_location(bri_bar, 68, 41, 28 * bri, 12);
	lv_obj_set_id(bri_bar, 44);

	lv_obj_t *bri_label = lv_label_create(window_cont, NULL);
	lv_obj_set_pos(bri_label, 273, 33);
	if (monitor_channel_get() < MON_CH_CCTV_1)
		lv_obj_set_y(bri_label, 165);
	else
		lv_obj_set_y(bri_label, 35);

	static char buf2[4] = {0};
	sprintf(buf2, "%02d", bri);
	lv_label_set_text(bri_label, buf2);
	lv_obj_set_id(bri_label, 102);

	if (monitor_channel_get() < MON_CH_CCTV_1)
	{
		window_outdoor_volume_add_btn_create(window_cont);
		window_outdoor_volume_subtract_btn_create(window_cont);

		window_indoor_volume_add_btn_create(window_cont);
		window_indoor_volume_subtract_btn_create(window_cont);

		lv_obj_t *outdoor_box = lv_cont_create(window_cont, bri_box);
		lv_obj_set_y(outdoor_box, 37);

		lv_obj_t *outdoor_bar = lv_cont_create(window_cont, bri_bar);
		set_location(outdoor_bar, 68, 39, 14 * out_vol, 12);
		lv_obj_set_id(outdoor_bar, 11);

		lv_obj_t *indoor_box = lv_cont_create(window_cont, bri_box);
		lv_obj_set_y(indoor_box, 105);

		lv_obj_t *indoor_bar = lv_cont_create(window_cont, bri_bar);
		set_location(indoor_bar, 68, 107, 14 * in_vol, 12);
		lv_obj_set_id(indoor_bar, 22);

		lv_obj_t *outdoor_label = lv_label_create(window_cont, NULL);
		lv_obj_set_pos(outdoor_label, 273, 33);
		// lv_obj_set_size(label, 105, 30);
		static char buf0[4] = {0};
		sprintf(buf0, "%02d", out_vol);
		lv_label_set_text(outdoor_label, buf0);
		lv_obj_set_id(outdoor_label, 100);

		lv_obj_t *indoor_label = lv_label_create(window_cont, NULL);
		lv_obj_set_pos(indoor_label, 273, 99);
		// lv_obj_set_size(label, 105, 30);
		static char buf1[4] = {0};
		sprintf(buf1, "%02d", in_vol);
		lv_label_set_text(indoor_label, buf1);
		lv_obj_set_id(indoor_label, 101);
	}

	window_brightness_add_btn_create(window_cont);
	window_brightness_subtract_btn_create(window_cont);
}

static bool monitor_setting_window_flag;

static void monitor_set_btn_up(lv_obj_t *obj)
{
	if (monitor_setting_window_flag)
	{
		lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
		if (window_cont != NULL)
		{
			lv_obj_del(window_cont);
			monitor_setting_window_flag = 0;
		}
	}
	else
	{
		monitor_setting_window_create();
		monitor_setting_window_flag = 1;
	}
}

static void monitor_set_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_set_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_COLOR_SET_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_COLOR_SET_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 6);
}

static void monitor_talk_open_task(lv_task_t *task)
{
	MONITOR_CH talk_ch = *(MONITOR_CH *)task->user_data;
	MONITOR_CH monitor_ch = monitor_channel_get();

	int vol = monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume : user_data_get()->door2.talk_volume;
	Debug("===========================>>ch:%d               monitor_ch:%d    vol:%d\n", talk_ch, monitor_ch, vol * 5 + 46);
	if (talk_ch == monitor_ch)
	{
		network_device devcie = monitor_ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
		audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN) | OPERATION_OPTION(AUDIO_OUT_EN) | OPERATION_OPTION(AUDIO_IN_EN)), AI_AO_O, true, true, vol * 5 + 46};
		audio_talk_open(ctrl);
		send_monitor_talk_cmd(true);
		audio_output_volume_set(vol * 5 + 46);
	}

	lv_task_del(montior_talk_task_t);
	montior_talk_task_t = NULL;
}

static void monitor_talk_image_display(bool is_talking)
{
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 7);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALLING_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_TALLING_FOCUS_PNG);
	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_TALK_UNFOCUS_PNG);
	static rom_bin_info info3 = rom_bin_info_get(ROM_RES_MONITOR_TALK_FOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, is_talking ? &info1 : &info3);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, is_talking ? &info1 : &info3);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, is_talking ? &info : &info2);
}
static void monitor_talk_btn_up(lv_obj_t *obj)
{
	Debug("===========================>>ch:%d   is_talking: %d\n", monitor_channel_get(), is_talking);
	monitor_click_up(NULL);
	if (is_talking == false) // 未通话
	{
		if (montior_talk_task_t == NULL) // 如果通话任务未空
		{
			monitor_ring_timeout_val = 0;
			is_talking = true;
			is_talked = true;
			if (record_message_ing) // 结束留言录制
			{
				record_message_ing = false;
				// record_video_stop(0x00); //停止录像
			}
			outdoor_order_set(NET_COMMON_PARAM_CAMERA_TALK);

			audio_play_stop_set();
			monitor_timer_set(MONITOR_DURATION * 4);

			static MONITOR_CH ch = MON_CH_NONE;
			ch = monitor_channel_get();
			montior_talk_task_t = lv_task_create(monitor_talk_open_task, 100, LV_TASK_PRIO_MID, &ch);
			send_monitor_talk_cmd(true);
			Debug("\n\n\n===%p========================>>%d\n\n\n\n", montior_talk_task_t, ch);

#ifdef MACHINE_CHIME
			Mechanical_Chime_Disable();
#endif

			monitor_talk_image_display(true);
		}
	}
}

static void monitor_talk_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_talk_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_TALK_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_TALK_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 7);
}

static void monitor_hand_btn_up(lv_obj_t *obj)
{
	extern INT_T tuya_ipc_get_client_online_num();
	send_monitor_hang_cmd();

	outdoor_order_set(NET_COMMON_CMD_NONE);

	if (monitor_enter_way_get() != MONITOR_ENTER_TUYA)
		goto_layout(pLAYOUT(standby)); // 页面跳转
}

static void monitor_hand_up_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_hand_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_HOLDUP_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_HOLDUP_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 8);
}

// static char monitor_light_flag = 0;
static void monitor_light_control(bool open)
{
	network_cmd_data data;
	data.device = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : monitor_channel_get() == MON_CH_DOOR_2 ? DEVICE_OUTDOOR_2
																													 : DEVICE_UNKONW;
	data.cmd = NET_COMMON_CMD_LIGHT;
	data.arg1 = open ? 1 : 0;
	network_send_cmd_data(&data);
	printf("%s ==========================+++++>%d\n", __func__, monitor_channel_get());

	if (tuya_online_status_get() == true)
		tuya_dp_138_response_light_switch(open);
}

#if 0
static void monitor_light_btn_up(lv_obj_t *obj)
{
	monitor_click_up(NULL);
	if (monitor_light_flag == 0)
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 9);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_ON_FOCUS_PNG);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_ON_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);

		//开灯
		monitor_light_control(true);
		// tuya_dp_138_response_light(true);
		monitor_light_flag = 1;
	}
	else
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 9);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_OFF_UNFOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_OFF_FOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
		//关灯
		monitor_light_control(false);
		// tuya_dp_138_response_light(false);
		monitor_light_flag = 0;
	}
}

static void monitor_light_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_light_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_OFF_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_LIGHT_OFF_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 9);
	monitor_light_flag = 0;
}
#endif

static bool layout_is_monitor;
static char monitor_Lock_1_task_lock_flag = 0;
static lv_task_t *unlock_1_task_t = NULL;
static void monitor_Lock_1_task(lv_task_t *task_t)
{
	printf("%s =================>>%d,%p,%d\n\r", __func__, __LINE__, unlock_1_task_t, layout_is_monitor);
	if (!layout_is_monitor)
	{
		if (unlock_1_task_t)
		{
			lv_task_del(unlock_1_task_t);
			unlock_1_task_t = NULL;
		}
		monitor_Lock_1_task_lock_flag = 0;
		return;
	}

	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 4);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LOCK_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_LOCK_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
	lv_obj_clear_state(btn, LV_STATE_FOCUSED);

	tuya_dp_234_response_outdoor_lock(false);

	if (unlock_1_task_t)
	{
		lv_task_del(unlock_1_task_t);
		unlock_1_task_t = NULL;
	}
	monitor_Lock_1_task_lock_flag = 0;
}

static void monitor_Lock_1_btn_up(lv_obj_t *obj)
{
	printf("%s =================>>%d,%d\n\r", __func__, __LINE__, user_data_get()->language.index);
	monitor_click_up(NULL);
	int lock_id = 1;
	int unlock_delay = monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.unlock_delay : user_data_get()->door1.unlock_delay;
	if (monitor_Lock_1_task_lock_flag == 0)
	{
		// monitor_ring_timeout_val = 0;
		audio_play_stop_set();
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 4);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
		lv_obj_add_state(btn, LV_STATE_FOCUSED);

		// 开锁
		network_cmd_data data;
		data.device = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : monitor_channel_get() == MON_CH_DOOR_2 ? DEVICE_OUTDOOR_2
																														 : DEVICE_UNKONW;
		data.cmd = NET_COMMON_CMD_UNLOCK;
		data.arg1 = unlock_delay;
		data.arg2 = lock_id | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
		network_send_cmd_data(&data);
		printf("%s =================>>%d,%d\n\r", __func__, __LINE__, data.arg2);
		tuya_dp_234_response_outdoor_lock(true);

		if (monitor_ring_time_ptask != NULL)
		{
			lv_task_del(monitor_ring_time_ptask);
			monitor_ring_time_ptask = NULL;
			monitor_ring_timeout_val = 0;
			audio_play_stop_set();
		}
		if (unlock_1_task_t == NULL)
			unlock_1_task_t = lv_task_create(monitor_Lock_1_task, unlock_delay * 1000, LV_TASK_PRIO_HIGH, NULL);

		monitor_Lock_1_task_lock_flag = 1;

		// if (is_monitor_talked() == false && user_data_get()->other.model != MUTE_PATTERN)
		// 	open_door_ring_play(80);
	}
}

// 创建Lock_1按钮
static void monitor_Lock_1_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_Lock_1_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LOCK_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_LOCK_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 4);
	monitor_Lock_1_task_lock_flag = 0;
}

static char monitor_Lock_2_task_lock_flag = 0;
static lv_task_t *unlock_2_task_t = NULL;
static void monitor_Lock_2_task(lv_task_t *task_t)
{
	printf("%s =================>>%d,%p,%d\n\r", __func__, __LINE__, unlock_2_task_t, layout_is_monitor);
	if (!layout_is_monitor)
	{
		if (unlock_2_task_t)
		{
			lv_task_del(unlock_2_task_t);
			unlock_2_task_t = NULL;
		}
		monitor_Lock_2_task_lock_flag = 0;
		return;
	}
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 5);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_LOCK_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_LOCK_UNFOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
	lv_obj_clear_state(btn, LV_STATE_FOCUSED);

	if (unlock_2_task_t)
	{
		lv_task_del(unlock_2_task_t);
		unlock_2_task_t = NULL;
	}
	monitor_Lock_2_task_lock_flag = 0;

	unlock_led_gpio_control(false);
}

static void monitor_Lock_2_btn_up(lv_obj_t *obj)
{
	printf("%s =================>>%d,%d\n\r", __func__, __LINE__, user_data_get()->language.index);
	monitor_click_up(NULL);
	int lock_id = 2;
	int unlock_delay = monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.ungate1_delay : user_data_get()->door1.ungate1_delay;
	if (monitor_Lock_2_task_lock_flag == 0)
	{
		// monitor_ring_timeout_val = 0;
		audio_play_stop_set();
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 5);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_UNLOCK_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_UNLOCK_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
		lv_obj_add_state(btn, LV_STATE_FOCUSED);

		// 开锁
		network_cmd_data data;
		data.device = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : monitor_channel_get() == MON_CH_DOOR_2 ? DEVICE_OUTDOOR_2
																														 : DEVICE_UNKONW;
		data.cmd = NET_COMMON_CMD_UNLOCK;
		data.arg1 = unlock_delay;
		data.arg2 = lock_id | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
		network_send_cmd_data(&data);
		unlock_led_gpio_control(true);
		if (monitor_ring_time_ptask != NULL)
		{
			lv_task_del(monitor_ring_time_ptask);
			monitor_ring_time_ptask = NULL;
			monitor_ring_timeout_val = 0;
			audio_play_stop_set();
		}

		// tuya_dp_233_response_gate2(true);
		if (unlock_2_task_t == NULL)
			unlock_2_task_t = lv_task_create(monitor_Lock_2_task, unlock_delay * 1000, LV_TASK_PRIO_HIGH, NULL);

		monitor_Lock_2_task_lock_flag = 1;

		// if (is_monitor_talked() == false && user_data_get()->other.model != MUTE_PATTERN)
		// 	open_door_ring_play(80);
	}
}

// 创建Lock_2按钮
static void monitor_Lock_2_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_Lock_2_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_LOCK_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_LOCK_FOCUS_PNG);
	lv_obj_t *btn = home_btn_create_1(coordinate, NULL, &btn_data, &info, &info1);
	lv_obj_set_id(btn, 5);
	monitor_Lock_2_task_lock_flag = 0;
}

static void monitor_enter_ui_different_processing(void)
{
	MONITOR_CH ch = monitor_channel_get();
	lv_obj_t *btn[8] = {NULL};

	for (int i = 0; i < 8; i++)
	{
		btn[i] = lv_obj_get_child_form_id(lv_scr_act(), i + 1);
	}

	if (ch == MON_CH_DOOR_1 || ch == MON_CH_DOOR_2)
	{

		if (monitor_enter_way_get() == MONITOR_ENTER_TUYA)
		{
			for (int i = 0; i < 8; i++)
			{
				// img = (lv_obj_t *)pdata->user_data;
				lv_obj_set_hidden(btn[i], true);
				// lv_obj_set_hidden(img, true);
			}
		}
		else
		{
			lv_obj_set_x(btn[0], 139);
			// lv_obj_align(img, btn[0], LV_ALIGN_CENTER, 0, 0);

			lv_obj_set_x(btn[1], 236);
			// pdata = (btn_data *)btn[1]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[1], LV_ALIGN_CENTER, 0, 0);

			lv_obj_set_x(btn[2], 333);
			// pdata = (btn_data *)btn[2]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[2], LV_ALIGN_CENTER, 0, 0);

			lv_obj_set_hidden(btn[3], false);
			// pdata = (btn_data *)btn[3]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_set_hidden(img, false);

			lv_obj_set_hidden(btn[4], false);
			lv_obj_set_x(btn[4], 527);
			// pdata = (btn_data *)btn[4]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[4], LV_ALIGN_CENTER, 0, 0);
			// lv_obj_set_hidden(img, false);

			lv_obj_set_x(btn[5], 624);
			// pdata = (btn_data *)btn[5]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[5], LV_ALIGN_CENTER, 0, 0);

			lv_obj_set_hidden(btn[6], false);
			// pdata = (btn_data *)btn[6]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_set_hidden(img, false);

			lv_obj_set_x(btn[7], 818);
			// pdata = (btn_data *)btn[7]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[7], LV_ALIGN_CENTER, 0, 0);

			// lv_obj_set_hidden(btn[8], false);
			// pdata = (btn_data *)btn[8]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_set_hidden(img, false);
		}
	}
	else if (ch == MON_CH_CCTV_1 || ch == MON_CH_CCTV_2)
	{

		if (monitor_enter_way_get() == MONITOR_ENTER_TUYA)
		{
			for (int i = 0; i < 8; i++)
			{
				// img = (lv_obj_t *)pdata->user_data;
				lv_obj_set_hidden(btn[i], true);
				// lv_obj_set_hidden(img, true);
			}
		}
		else
		{
			int btn_init_x = 275;
			lv_obj_set_x(btn[0], btn_init_x);
			// lv_obj_align(img, btn[0], LV_ALIGN_CENTER, 0, 0);

			lv_obj_set_x(btn[1], btn_init_x + 97);
			// pdata = (btn_data *)btn[1]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[1], LV_ALIGN_CENTER, 0, 0);

			lv_obj_set_x(btn[2], btn_init_x + (97 * 2));
			// pdata = (btn_data *)btn[2]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[2], LV_ALIGN_CENTER, 0, 0);

			lv_obj_set_hidden(btn[3], true);
			// pdata = (btn_data *)btn[3]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_set_hidden(img, true);

			lv_obj_set_hidden(btn[4], true);
			// pdata = (btn_data *)btn[4]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[4], LV_ALIGN_CENTER, 0, 0);
			// lv_obj_set_hidden(img, true);

			lv_obj_set_x(btn[5], btn_init_x + (97 * 3));
			// pdata = (btn_data *)btn[5]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[5], LV_ALIGN_CENTER, 0, 0);

			lv_obj_set_hidden(btn[6], true);
			// pdata = (btn_data *)btn[6]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_set_hidden(img, true);

			lv_obj_set_x(btn[7], btn_init_x + (97 * 4));
			// pdata = (btn_data *)btn[7]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_align(img, btn[7], LV_ALIGN_CENTER, 0, 0);

			// lv_obj_set_hidden(btn[8], true);
			// pdata = (btn_data *)btn[8]->user_data;
			// img = (lv_obj_t *)pdata->user_data;
			// lv_obj_set_hidden(img, true);
		}
	}
}

static void monitor_click_up(lv_obj_t *obj)
{
	if (monitor_setting_window_flag)
	{
		lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
		if (window_cont != NULL)
		{
			lv_obj_del(window_cont);
			monitor_setting_window_flag = 0;
		}
	}
}
#ifdef AUTO_RECORD_TIME
static int auto_rec_timer = 0;
static lv_task_t *auto_record_colse_task_t = NULL;
static void auto_record_colse_task(lv_task_t *task_t)
{
	if (is_video_recording() == false)
	{
		lv_task_del(auto_record_colse_task_t);
		auto_record_colse_task_t = NULL;
		return;
	}
	MONITOR_CH ch = monitor_channel_get();
	int record_time = 0;
	if (ch == MON_CH_DOOR_1)
	{
		record_time = user_data_get()->door1.record_time;
	}
	else if (ch == MON_CH_DOOR_2)
	{
		record_time = user_data_get()->door2.record_time;
	}

	if (++auto_rec_timer > record_time)
	{
		record_video_stop(0x00); // 停止录像
		lv_task_del(auto_record_colse_task_t);
		auto_record_colse_task_t = NULL;
	}
}
#endif

static bool is_monitor_call = false;
static lv_task_t *auto_record_task_t = NULL;
void monitor_auto_record_pictrue(bool only_send_tuya)
{
	if (only_send_tuya)
	{
		if (tuya_online_status_get() == true)
		{
			extern bool send_tuya_record(char record_mode);
			send_tuya_record(REC_MODE_TUYA);
		}
	}
	else
	{
		if (is_sdcard_insert() == false)
		{
			if (tuya_online_status_get() == true)
			{
				extern bool send_tuya_record(char record_mode);
				send_tuya_record(REC_MODE_TUYA);
			}
			return;
		}
		int free_space = sd_free_space_insufficient();
		if (free_space < 500)
		{
			if (sdcard_insert_msg_box == NULL)
			{
				sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
				record_message_ing = false;
			}
		}

		if (free_space > 200 && record_pictrue_start(REC_MODE_AUTO, monitor_channel_get()) == true)
		{
			lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 2);
			static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_SNAPING_FOCUS_PNG);
			static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SNAPING_UNFOCUS_PNG);
			lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
			lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
			lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
			lv_obj_set_click(btn, false);

			monitor_record_label_display(false, false);
		}
		else if (free_space < 200)
		{
			extern void detect_sd_free_space(void);
			detect_sd_free_space();
		}
	}
}
void monitor_auto_record_video(void)
{
	if (is_sdcard_insert() == false)
	{
		return;
	}

	int free_space = sd_free_space_insufficient();
	if (free_space < 500)
	{
		if (sdcard_insert_msg_box == NULL)
		{
			sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
			record_message_ing = false;
		}
	}

	if (free_space > 200 && record_video_start(REC_MODE_AUTO, true, monitor_channel_get()) == true)
	{
		lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 3);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORDING_FOCUS_PNG);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORDING_UNFOCUS_PNG);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);

#ifdef AUTO_RECORD_TIME
		auto_rec_timer = 0;
		auto_record_colse_task_t = lv_task_create(auto_record_colse_task, 1000, LV_TASK_PRIO_HIGH, NULL);
#endif
		monitor_record_label_display(true, false);
	}
	else if (free_space < 200)
	{
		extern void detect_sd_free_space(void);
		detect_sd_free_space();
	}
}

static void auto_record_task(lv_task_t *task_t)
{
	bool only_send_tuya = task_t->user_data == NULL ? false : *((bool *)(task_t->user_data));
	// printf("auto_record_task=====================++++++>:%d  task_t->user_data:%p\n\r",only_send_tuya,task_t->user_data);
	monitor_auto_record_pictrue(only_send_tuya);
	{
		lv_task_del(auto_record_task_t);
		auto_record_task_t = NULL;
	}
}

// lv_obj_t * slider_label;
// /*************************************************
//  *  函数名称 :  slider_show_event_cb
//  *  参    数 ： 无
//  *  函数功能 ： 事件回调
//  *************************************************/
// static void slider_show_event_cb(lv_event_t * e)
// {
//    lv_obj_t * slider = lv_event_get_target(e);
//    char buf[8];
//    lv_snprintf(buf,sizeof(buf),"%d%d",(int)lv_slider_get_value(slider));
//    lv_label_set_text(slider_label,buf);
//    lv_obj_align_to(slider_label,slider,LV_ALIGN_OUT_BOTTOM_MID,0,10);
// }
// /*************************************************
//  *  函数名称 :  slider_show_1
//  *  参    数 ： 无
//  *  函数功能 ： slider显示
//  *************************************************/
// void slider_show_1()
// {
//    lv_obj_t * slider = lv_slider_create(lv_scr_act());   //创建滑动条对象
//    lv_obj_center(slider);                                //居中显示
//    lv_obj_add_event_cb(slider,slider_show_event_cb,LV_EVENT_VALUE_CHANGED,NULL); //设置回调显示
//    lv_slider_set_value(slider,50,LV_ANIM_OFF);
//    slider_label = lv_label_create(lv_scr_act());         //创建Label
//    lv_label_set_text(slider_label,"0%");                 //设置label内容

//    lv_obj_align_to(slider_label,slider,LV_ALIGN_CENTER,0,10); //设置位置
// }
/* lv_task_t * tuya_call_task = NULL;
static void tuya_call_func(lv_task_t *task)
{

		tuya_ipc_door_bell_press(DOORBELL_AC,NULL,0, NOTIFICATION_CONTENT_JPEG);
} */

static void device_monitor_busy_func(unsigned long arg1, unsigned long arg2)
{
	if (current_layout_get() != &layout_standby && monitor_enter_way_get() != MONITOR_ENTER_TUYA && !is_talking)
	{
		goto_layout(pLAYOUT(standby));
	}
}

#ifdef INTERCOM_VERSION
static void LAYOUT_ENETER_FUNC(monitor1)
#else
static void LAYOUT_ENETER_FUNC(monitor)
#endif
{
	Debug("================================\n\r");

	monitor_video_mode_open(true);
	device_monitor_busy_register(device_monitor_busy_func);
	lv_area_t area[] = {
		{0, 0, 1024, 60},
		{0, 483, 1024, 570},
		{350, 187, 674, 413},
	};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

	// tuya_call_task = lv_task_create(tuya_call_func, 3000, LV_TASK_PRIO_HIGH, NULL);
	Controls_location module_coordinate[] = MONITOR_MODULE_COORDINATE_INIT;
	monitor_enter_parameter_init(); // 倒计时参数初始化
	monitor_2_head_create();
	lv_obj_t *obj = lv_scr_act();
	static btn_data btn_data = btn_data_up_create(monitor_click_up);
	obj->user_data = &btn_data;
	btn_touch_event_listen(obj);
	lv_obj_set_click(obj, true);
	monitor_switch_btn_create(module_coordinate[SWITCH_MODULE]);
	monitor_snap_photo_btn_create(module_coordinate[SANP_MODULE]);
	monitor_record_video_btn_create(module_coordinate[RECORD_MODULE]);
	monitor_Lock_1_btn_create(module_coordinate[LOCK1_MODULE]);
	monitor_Lock_2_btn_create(module_coordinate[LOCK2_MODULE]);
	monitor_set_btn_create(module_coordinate[SETTING_MODULE]);
	monitor_talk_btn_create(module_coordinate[TALK_MODULE]);
	monitor_hand_up_btn_create(module_coordinate[HAND_MODULE]);
	// monitor_light_btn_create();

	Debug("================================\n\r");
	monitor_enter_ui_different_processing();

	Debug("================================\n\r");
	if (is_monitor_call)
	{
		is_monitor_call = false;
		int record_mode = 0;
		if (monitor_channel_get() == MON_CH_DOOR_1)
		{
			record_mode = user_data_get()->door1.record_mode;
		}
		else if (monitor_channel_get() == MON_CH_DOOR_2)
		{
			record_mode = user_data_get()->door2.record_mode;
		}
		// if(!(monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.message_sw : user_data_get()->door2.message_sw))//未开启留言
		// {
		Debug("auto_record_task :%p       user_data_get()->other.model:%d \n\r", auto_record_task_t, user_data_get()->other.model);
		if (auto_record_task_t == NULL && record_message_ing == false && user_data_get()->other.model != NOT_AT_HOME_PATTERN)
		{
			Debug("user_data_get()->other.model:%d \n\r", user_data_get()->other.model);
			extern void jpg_push_to_tuya(int type);
			jpg_push_to_tuya(1);
			if (record_mode)
			{
				monitor_auto_record_video();
			}
			else
			{
			}
			static bool send_tuya;
			send_tuya = record_mode;
			auto_record_task_t = lv_task_create(auto_record_task, 2000, LV_TASK_PRIO_HIGH, &send_tuya);
		}
		else if (user_data_get()->other.model == NOT_AT_HOME_PATTERN) // 留言打开后，不自动拍录像，只将消息发送涂鸦
		{
			Debug("user_data_get()->other.model:%d \n\r", user_data_get()->other.model);
			static bool send_tuya = true;
			if (auto_record_task_t == NULL)
				auto_record_task_t = lv_task_create(auto_record_task, 2000, LV_TASK_PRIO_HIGH, &send_tuya);
		}
	}

	// slider_show_1();
	layout_is_monitor = 1;
	monitor_setting_window_flag = 0;

	/*
	monitor_swap_btn_create(); //更换通道按钮创建

	monitor_snap_photo_btn_create(); //抓拍按钮创建

	monitor_record_video_btn_create(); //录像按钮创建

	monitor_talk_btn_create(); //通话按钮创建

	monitor_door1_btn_create(); //门1按钮创建

	monitor_door2_btn_create(); //门2按钮创建
	//monitor_light_btn_create();

	monitor_home_btn_create(); //返回home界面按钮创建
*/
	// network_device devcie = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;

	// standby_timer_close();
	Debug("=====================================end\n");
	backlight_open(true, true, get_monitor_brightness());
}

// 退出

#ifdef INTERCOM_VERSION
static void LAYOUT_QUIT_FUNC(monitor1)
#else
static void LAYOUT_QUIT_FUNC(monitor)
#endif
{
	Debug("============================>>>\n");

	extern void main_device_monitor_busy_func(unsigned long arg1, unsigned long arg2);
	device_monitor_busy_register(main_device_monitor_busy_func);
	record_video_stop(0x00); // 停止录像

	lv_area_t area[] = {{0, 0, 1024, 600}};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

	monitor_light_control(false); // 确保灯光关闭

	monitor_video_mode_close(); // 监控和声音关闭

	if (sdcard_insert_msg_box != NULL)
	{
		lv_obj_del(sdcard_insert_msg_box);
		sdcard_insert_msg_box = NULL;
	}

	if (monitor_time_ptask != NULL) // 计时任务未退出
	{
		lv_task_del(monitor_time_ptask); // 退出计时任务
		monitor_time_ptask = NULL;		 // 置空指针
	}

	if (monitor_timer_ptask != NULL) //
	{
		lv_task_del(monitor_timer_ptask); //
		monitor_timer_ptask = NULL;		  //
	}

	if (montior_talk_task_t != NULL) // 对话任务未退出
	{
		lv_task_del(montior_talk_task_t); // 删除对话人物
		montior_talk_task_t = NULL;		  // 置空任务
	}

	if (auto_record_task_t != NULL)
	{
		lv_task_del(auto_record_task_t);
		auto_record_task_t = NULL;
	}

	if (record_message_task_t != NULL)
	{
		lv_task_del(record_message_task_t);
		record_message_task_t = NULL;
	}

	if (monitor_ring_time_ptask != NULL) //
	{
		lv_task_del(monitor_ring_time_ptask); //
		monitor_ring_time_ptask = NULL;		  //
	}

	if (monitor_chime_ptask != NULL)
	{
		lv_task_del(monitor_chime_ptask);
		monitor_chime_ptask = NULL;
		chime_gpio_disable();
	}
	/* 	if(tuya_call_task != NULL)
		{
			lv_task_del(tuya_call_task);
			tuya_call_task = NULL;
		} */

	/* 	if(monitor_tuya_exit_task_t != NULL)
		{
			lv_task_del(monitor_tuya_exit_task_t);
			monitor_tuya_exit_task_t = NULL;
		} */

#if 0
	if (unlock_task_t != NULL)
	{
		lv_task_del(unlock_task_t);
		unlock_task_t = NULL;
	}
#endif
	monitor_ring_timeout_val = 0;

	montior_busy_msg_box = NULL;
	is_tuya_enter = 0;

	is_monitor_call = false;

	audio_talk_close(true); // 音频关闭
	if (is_tuya_talking || is_talking)
	{
		send_monitor_talk_cmd(false);
	}

	is_talking = false;
	is_talked = false;
	is_tuya_talking = false;
	record_message_ing = false;
	outdoor_order_set(NET_COMMON_CMD_NONE);
	monitor_call_label_display(MON_CH_NONE, MON_CH_NONE);
	standby_timer_open(-1, NULL); // 获取系统时间

	outdoor_call_event_register(monitor_call_extern_func);

	mechanical_key_event_register(mechanical_key_extern_callback);

	record_jpeg_event_register(NULL); // 关闭所以注册表
	record_video_event_register(NULL);
	indoor_cmd_event_register(NULL);
	network_event_register(NULL);
	device_adc_key_register(NULL);
	tuya_event_register(tuya_event_extern_proc); // 涂鸦

	lv_obj_set_click(lv_scr_act(), false);
	wifi_control = false;
	extern bool need_load_bg_flag;
	need_load_bg_flag = true;
	layout_is_monitor = 0;

	user_data_save();

	backlight_open(true, false, user_data_get()->other.brightness);
	talk_led_gpio_control(false);
	unlock_led_gpio_control(false);
	monitor_enter_way_set(MONITOR_ENTER_NONE);

	Debug("============================>>>\n");
}

static void monitor_chime_task(struct _lv_task_t *task_t)
{
	if (monitor_chime_ptask != NULL)
	{
		lv_task_del(monitor_chime_ptask);
		monitor_chime_ptask = NULL;
	}
	chime_gpio_disable();
}

static void monitor_ring_time_task(struct _lv_task_t *task_t)
{

	static unsigned long sec = 0;
	struct ak_timeval timeval;
	ak_get_ostime(&timeval);

	if (timeval.sec != sec)
	{
		sec = timeval.sec;
		if (monitor_ring_timeout_val == 0)
		{
			audio_play_stop_set();
			// printf("monitor_ring_timeout_val ============:%d\n\r", monitor_ring_timeout_val);

			if (monitor_ring_time_ptask != NULL)
			{
				lv_task_del(monitor_ring_time_ptask);
				monitor_ring_time_ptask = NULL;
			}
			// record_video_stop(0x00);
			if (monitor_channel_get() < MON_CH_CCTV_1)
			{
				door_info curr_monitor_ch = monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1 : user_data_get()->door2;
				if (is_sdcard_insert() && curr_monitor_ch.message_sw && record_message_ing == false && is_talked == false && monitor_enter_way_get() == MONITOR_ENTER_CALL) // 门口机呼叫监控时间结束且未接听则进入录制留言操作
				{
					// Debug("monitor_timeout_val:%d  curr_monitor_ch.message_time:%d\n", monitor_timeout_val, curr_monitor_ch.message_time);

					{
						int free_space = sd_free_space_insufficient();
						if (free_space > 200)
						{
							record_video_stop(0x00);
							record_message_action(curr_monitor_ch);
						}
						else if (free_space < 200)
						{
							extern void detect_sd_free_space(void);
							detect_sd_free_space();
						}
						if (free_space < 500)
						{
							if (sdcard_insert_msg_box == NULL)
							{
								sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
								record_message_ing = false;
							}
						}
						return;
					}
				}
			}
		}
		else
		{
			monitor_ring_timeout_val--;
		}
	}
}

static void mechanical_key_extern_callback(unsigned long arg1, unsigned long arg2)
{
	extern bool backlight_status_get(void);
	Debug("backlight_status_get():%d\n", backlight_status_get());
	if (backlight_status_get() == false)
	{
		backlight_open(true, false, user_data_get()->other.brightness);
		if (user_data_get()->other.screen_saver)
			goto_layout(pLAYOUT(standby)); // 页面跳转
		else
			goto_layout(pLAYOUT(home)); // 页面跳转
		return;
	}

	if (format_sd_card_status() || get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2) || monitor_enter_way_get() == MONITOR_ENTER_TUYA || tuya_monitor_state_get())
		return;

	// system_bg_data_backup();//背景颜色恢复
	monitor_channel_set(MON_CH_DOOR_1); // 通道选择 手动进入就是DOOR1
	monitor_enter_way_set(MONITOR_ENTER_MANUAL);
	goto_layout(pLAYOUT(monitor)); // 页面跳转
	audio_talk_ctrl ctrl = {{DEVICE_OUTDOOR_1}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
	audio_talk_open(ctrl);
}

static void mechanical_key_inside_callback(unsigned long arg1, unsigned long arg2)
{
	if (monitor_enter_way_get() == MONITOR_ENTER_TUYA || tuya_monitor_state_get())
		return;

	MONITOR_CH ch = monitor_channel_get();
	Debug("is_talking:%d,ch:%d\n", is_talking, ch);

	if (is_talking == false && monitor_enter_way_get() == MONITOR_ENTER_CALL)
	{
		monitor_talk_btn_up(NULL);
		return;
	}

	if (is_talking || monitor_next_ch() == MON_CH_DOOR_1)
	{
		monitor_hand_btn_up(NULL);
		return;
	}

	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2))
	{
		return;
	}
#if 0
	record_video_stop(0x00);
	monitor_call_label_display(MON_CH_NONE,true);
	monitor_click_up(NULL);
	monitor_timer_set(MONITOR_DURATION);
	monitor_enter_way_set(MONITOR_ENTER_MANUAL);

	audio_talk_close(false);
	{
		audio_talk_ctrl ctrl = {{ch+DEVICE_INDOOR_ID6},(OPERATION_OPTION(AUDIO_SEND_EN)|OPERATION_OPTION(AUDIO_RECEIVE_EN)),
													AI_AO_C,true,false,ch == MON_CH_DOOR_1 ?  user_data_get()->door1.talk_volume*5 + 46:user_data_get()->door2.talk_volume*5 + 46};
		audio_talk_open(ctrl);
	}
	
	outdoor_order_set(NET_COMMON_CMD_NONE);
	monitor_channel_set(MON_CH_DOOR_2);
	monitor_switch();
	monitor_channel_label_display();
	monitor_enter_ui_different_processing();
	backlight_open(true, true,get_monitor_brightness());
#else
	monitor_switch_btn_up(NULL);
#endif
}

static void device_adc_key_callback(unsigned long arg1, unsigned long arg2)
{
	if (user_data_get()->other.model != MUTE_PATTERN && user_data_get()->audio.key_sound)
	{
		touch_sound_play(current_layout_get() == &layout_monitor ? (user_data_get()->audio.door_ring_val) : 7);
	}

	switch (arg1)
	{
	case 1:
		monitor_Lock_2_btn_up(NULL);
		break;

	case 2:
		/* 通话 */
		if (is_talking || monitor_channel_get() == MON_CH_CCTV_1 || monitor_channel_get() == MON_CH_CCTV_2)
			monitor_hand_btn_up(NULL);
		else
			monitor_talk_btn_up(NULL);
		break;
	default:
		break;
	}
}

static void monitor_call_extern_ring_play_finsih_callback(void)
{
	printf("%s==================>>>>>>>>>>>>>>\n\r", __func__);
	if ((monitor_ring_timeout_val) && is_audio_play_ing() == false)
	{
		if (monitor_channel_get() == MON_CH_DOOR_1)
		{
			if (ring_attr.door1.ring_mode)
			{
				media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, ring_attr.door1.custom_ring);
				custom_music_play(info->file_name, ring_attr.door1.ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_extern_ring_play_finsih_callback);
			}
			else
				door_ring_play(ring_attr.door1.ring, ring_attr.door1.ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_extern_ring_play_finsih_callback);
		}
		else if (monitor_channel_get() == MON_CH_DOOR_2)
		{
			if (ring_attr.door2.ring_mode)
			{
				media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, ring_attr.door2.custom_ring);
				custom_music_play(info->file_name, ring_attr.door2.ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_extern_ring_play_finsih_callback);
			}
			else
				door_ring_play(ring_attr.door2.ring, ring_attr.door2.ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_extern_ring_play_finsih_callback);
		}
	}
	else
	{
	}
}
#if 1
static void monitor_call_inside_ring_play_finsih_callback(void)
{
	// printf("%s==================>>>>>>>>>>>>>>\n\r",__func__);
	if (monitor_ring_timeout_val && is_audio_play_ing() == false)
	{
		if (monitor_channel_get() == MON_CH_DOOR_1)
		{
			if (ring_attr.door1.ring_mode)
			{
				static char str1[16] = {0};
				media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, ring_attr.door1.custom_ring);
				sprintf(str1, "%s", info->file_name);
				custom_music_play(info->file_name, ring_attr.door1.ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
			}
			else
				door_ring_play(ring_attr.door1.ring, ring_attr.door1.ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
		}
		else if (monitor_channel_get() == MON_CH_DOOR_2)
		{
			if (ring_attr.door2.ring_mode)
			{
				static char str1[16] = {0};
				media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, ring_attr.door2.custom_ring);
				sprintf(str1, "%s", info->file_name);
				custom_music_play(info->file_name, ring_attr.door2.ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
			}
			else
				door_ring_play(ring_attr.door2.ring, ring_attr.door2.ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
		}
	}
}
#endif

// 外部呼叫的回调函数
static void monitor_call_extern_func(unsigned long arg1, unsigned long arg2)
{

	if (arg2 == 0)
	{
		monitor_enter_way_set(MONITOR_ENTER_ALARM);
	}

	Debug("%s=======================>>>%d,%lu\n\r", __func__, __LINE__, arg2);
	if (format_sd_card_status() || get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2) || tuya_monitor_state_get() || monitor_enter_way_get() == MONITOR_ENTER_TUYA)
	{
		Debug("outdoor tlaking...,return %d %d %d %d\n\r", get_outdoor_talk_state(MON_CH_DOOR_1), get_outdoor_talk_state(MON_CH_DOOR_2), tuya_monitor_state_get(), monitor_enter_way_get());
		return;
	}

	network_device device = (network_device)arg1;					  // 确定呼叫的门口机设备
	if ((device != DEVICE_OUTDOOR_1) && (device != DEVICE_OUTDOOR_2)) // 不是门口机 返回
	{
		return;
	}

	if (device == DEVICE_OUTDOOR_2 && user_data_get()->door2.enable_sw == false)
	{
		return;
	}

	// if(current_layout_get() == &layout_video || current_layout_get() == &layout_video_list || current_layout_get() == &layout_photo || current_layout_get() == &layout_photo_list)
	// {
	// 	video_play_stop();
	// 	extern bool video_play_wait_thread_quit(void);
	// 	if(video_play_wait_thread_quit() == false)
	// 		return;

	// 	extern bool media_thumb_wait_thread_quit(void);
	// 	if(media_thumb_wait_thread_quit() == false)
	// 		return;
	// }

	is_monitor_call = true;
	monitor_channel_set(device == DEVICE_OUTDOOR_1 ? MON_CH_DOOR_1 : MON_CH_DOOR_2); // 选择通道
	monitor_enter_way_set(MONITOR_ENTER_CALL);
	goto_layout(pLAYOUT(monitor));

	if (user_data_get()->other.model != MUTE_PATTERN)
	{
#ifndef MACHINE_CHIME
		chime_gpio_enable();
		if (monitor_chime_ptask == NULL)
		{
			monitor_chime_ptask = lv_task_create(monitor_chime_task, 5000, LV_TASK_PRIO_HIGH, NULL);
		}
#else
		if (user_data_get()->other.chime_type == false)
			Mechanical_Chime_Enable();
		else
		{
			chime_gpio_enable();
			if (monitor_chime_ptask == NULL)
			{
				monitor_chime_ptask = lv_task_create(monitor_chime_task, 5000, LV_TASK_PRIO_HIGH, NULL);
			}
		}
#endif
	}

	if (device == DEVICE_OUTDOOR_1)
	{
		// 根据选择对应通道的铃声和音量
		if (user_data_get()->other.model == AT_HOME_PATTERN)
		{
			Debug("ring_attr.door1.ring_mode ================>>>%d\n", ring_attr.door1.ring_mode);
			// monitor_timer_task(monitor_ring_time_ptask);
			if (ring_attr.door1.ring_mode)
			{
				media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, ring_attr.door1.custom_ring);
				Debug("media_info_get %d================>>>%s\n", ring_attr.door1.custom_ring, info->file_name);
				custom_music_play(info->file_name, ring_attr.door1.ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
			}
			else
			{

				door_ring_play(ring_attr.door1.ring, ring_attr.door1.ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_extern_ring_play_finsih_callback);
			}

			monitor_ring_timeout_val = ring_attr.door1.ring_time;
			if (monitor_ring_time_ptask != NULL)
			{
				lv_task_del(monitor_ring_time_ptask);
			}
			monitor_ring_time_ptask = lv_task_create(monitor_ring_time_task, 500, LV_TASK_PRIO_HIGH, NULL);
			lv_task_ready(monitor_ring_time_ptask);
		}
		else if (user_data_get()->other.model == NOT_AT_HOME_PATTERN && is_sdcard_insert() && (monitor_enter_way_get() != MONITOR_ENTER_ALARM))
		{
			int free_space = sd_free_space_insufficient();
			if (free_space > 200)
			{
				record_message_action(user_data_get()->door1);
			}
			else if (free_space < 200)
			{
				extern void detect_sd_free_space(void);
				detect_sd_free_space();
			}
			if (free_space < 500)
			{
				if (sdcard_insert_msg_box == NULL)
				{
					sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
					record_message_ing = false;
				}
			}
		}
		else
		{
			monitor_ring_timeout_val = ring_attr.door1.ring_time;
			if (monitor_ring_time_ptask != NULL)
			{
				lv_task_del(monitor_ring_time_ptask);
			}
			monitor_ring_time_ptask = lv_task_create(monitor_ring_time_task, 500, LV_TASK_PRIO_HIGH, NULL);
			lv_task_ready(monitor_ring_time_ptask);
		}
	}
	else if (device == DEVICE_OUTDOOR_2)
	{
		if (user_data_get()->other.model == AT_HOME_PATTERN)
		{
			// monitor_timer_task(monitor_ring_time_ptask);
			if (ring_attr.door2.ring_mode)
			{
				media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, ring_attr.door2.custom_ring);
				custom_music_play(info->file_name, ring_attr.door2.ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
			}
			else
				door_ring_play(ring_attr.door2.ring, ring_attr.door2.ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_extern_ring_play_finsih_callback);

			monitor_ring_timeout_val = ring_attr.door2.ring_time;
			if (monitor_ring_time_ptask != NULL)
			{
				lv_task_del(monitor_ring_time_ptask);
			}
			monitor_ring_time_ptask = lv_task_create(monitor_ring_time_task, 500, LV_TASK_PRIO_HIGH, NULL);
			lv_task_ready(monitor_ring_time_ptask);
		}
		else if (user_data_get()->other.model == NOT_AT_HOME_PATTERN && is_sdcard_insert() && (monitor_enter_way_get() != MONITOR_ENTER_ALARM))
		{
			int free_space = sd_free_space_insufficient();
			if (free_space > 200)
			{
				record_message_action(user_data_get()->door2);
			}
			else if (free_space < 200)
			{
				extern void detect_sd_free_space(void);
				detect_sd_free_space();
			}
			if (free_space < 500)
			{
				if (sdcard_insert_msg_box == NULL)
				{
					sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
					record_message_ing = false;
				}
			}
		}
		else
		{
			monitor_ring_timeout_val = ring_attr.door2.ring_time;
			if (monitor_ring_time_ptask != NULL)
			{
				lv_task_del(monitor_ring_time_ptask);
			}
			monitor_ring_time_ptask = lv_task_create(monitor_ring_time_task, 500, LV_TASK_PRIO_HIGH, NULL);
			lv_task_ready(monitor_ring_time_ptask);
		}
	}

	// system_bg_data_backup();
	network_device devcie = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
	audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_channel_get() == MON_CH_DOOR_1 ? ring_attr.door1.ring_val * 5 + 46 : ring_attr.door2.ring_val * 5 + 46};
	audio_talk_open(ctrl);
}

static void monitor_call_inside_action(MONITOR_CH ch, unsigned long arg2)
{
	is_talking = false;
	is_talked = false;

	if (arg2 == false)
	{
		monitor_enter_way_set(MONITOR_ENTER_ALARM);
	}

	monitor_talk_image_display(false);

	door_info door_ch = ch == MON_CH_DOOR_1 ? user_data_get()->door1 : user_data_get()->door2;

	int ring_time;
	ring_time = ch == MON_CH_DOOR_1 ? ring_attr.door1.ring_time : ring_attr.door2.ring_time;
	monitor_ring_timeout_val = ring_time;
	/* 若切换前为录制留言状态，即当前通道若开启留言，则监控时间设置为留言时间，反之为铃声时长
	若切换前不为留言状态，即当前通道若开启留言，则监控时间为留言时间加铃声时间，反之为铃声时长 */

	monitor_timer_set(record_message_ing ? (door_ch.message_sw ? door_ch.message_time : ring_time) : ring_time + (door_ch.message_sw ? door_ch.message_time : 0));

	/* 关闭上一通道音视频处理*/
	audio_talk_close(false);
	record_video_stop(0x00);

	/* 若当前在涂鸦监控状态，清空上一通道所有音视频缓存 */
	if (monitor_enter_way_get() == MONITOR_ENTER_TUYA)
	{
		extern void tuya_ipc_ring_buffer_video_release_data(void);
		tuya_ipc_ring_buffer_video_release_data();
	}

	monitor_channel_set(ch);

	if (door_ch.message_sw && record_message_ing) // 留言过程中切换通道，即切换结束后若当前通道开启留言则继续留言
	{
		record_message_action(door_ch); // 留言录制延时两秒开启
	}
	else
	{
		outdoor_order_set(NET_COMMON_CMD_NONE);
		record_message_ing = false;
	}

	/* 开启当前通道音视频监控 */
	monitor_switch(); // monitor_open(true);//

	monitor_channel_label_display();
	audio_play_stop_set();
	if (user_data_get()->other.model == AT_HOME_PATTERN)
	{

		if (monitor_ring_time_ptask != NULL)
		{
			lv_task_del(monitor_ring_time_ptask);
			monitor_ring_time_ptask = NULL;
		}

		if (monitor_ring_time_ptask == NULL && record_message_ing != true) /*为留言状态，即当前通道不再进行铃声播放，直接进入留言状态 */
		{
			monitor_ring_time_ptask = lv_task_create(monitor_ring_time_task, 500, LV_TASK_PRIO_HIGH, NULL);
			lv_task_ready(monitor_ring_time_ptask);
		}

		// monitor_timer_task(monitor_ring_time_ptask);
		if (is_audio_play_ing() == false && record_message_ing != true) /* 为留言状态，即当前通道不再进行铃声播放，直接进入留言状态 */
		{
			if (ch == MON_CH_DOOR_1)
			{
				if (ring_attr.door1.ring_mode)
				{
					media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, ring_attr.door1.custom_ring);
					custom_music_play(info->file_name, ring_attr.door1.ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
				}
				else
				{
					door_ring_play(ring_attr.door1.ring, ring_attr.door1.ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
				}
			}
			else if (ch == MON_CH_DOOR_2)
			{
				if (ring_attr.door2.ring_mode)
				{
					media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, ring_attr.door2.custom_ring);
					custom_music_play(info->file_name, ring_attr.door2.ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
				}
				else
				{
					door_ring_play(ring_attr.door2.ring, ring_attr.door2.ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_inside_ring_play_finsih_callback);
				}
			}
		}
	}
	else if (user_data_get()->other.model == NOT_AT_HOME_PATTERN && is_sdcard_insert() && arg2)
	{
		int free_space = sd_free_space_insufficient();
		if (free_space > 200)
		{
			record_message_action(user_data_get()->door1);
		}
		else if (free_space < 200)
		{
			extern void detect_sd_free_space(void);
			detect_sd_free_space();
		}

		if (free_space < 500)
		{
			if (sdcard_insert_msg_box == NULL)
			{
				sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
				record_message_ing = false;
			}
		}
	}

	monitor_enter_ui_different_processing();
}

static void monitor_call_inside_func(unsigned long arg1, unsigned long arg2)
{
	printf("%s=======================>>>%d\n\r", __func__, __LINE__);
	network_device device = (network_device)arg1;
	if (((device != DEVICE_OUTDOOR_1) && (device != DEVICE_OUTDOOR_2)) || monitor_enter_way_get() == MONITOR_ENTER_TUYA)
	{
		return;
	}

	if (device == DEVICE_OUTDOOR_2 && user_data_get()->door2.enable_sw == false)
	{
		return;
	}

	if (/* (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2) )&& */ monitor_enter_way_get() == MONITOR_ENTER_CALL)
	{
		monitor_call_label_display(device == DEVICE_OUTDOOR_1 ? MON_CH_DOOR_1 : MON_CH_DOOR_2, false);
		return;
	}

	MONITOR_CH local_ch = monitor_channel_get();
	MONITOR_CH call_ch = device == DEVICE_OUTDOOR_1 ? MON_CH_DOOR_1 : MON_CH_DOOR_2;

	if (local_ch != call_ch)
	{
		if (user_data_get()->other.model != MUTE_PATTERN)
		{
#ifndef MACHINE_CHIME
			chime_gpio_enable();
			if (monitor_chime_ptask == NULL)
			{
				monitor_chime_ptask = lv_task_create(monitor_chime_task, 5000, LV_TASK_PRIO_HIGH, NULL);
			}
#else
			if (user_data_get()->other.chime_type == false)
				Mechanical_Chime_Enable();
			else
			{
				chime_gpio_enable();
				if (monitor_chime_ptask == NULL)
				{
					monitor_chime_ptask = lv_task_create(monitor_chime_task, 5000, LV_TASK_PRIO_HIGH, NULL);
				}
			}
#endif
		}

		monitor_call_inside_action(call_ch, arg2);
		audio_talk_ctrl ctrl = {{device}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, call_ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
		audio_talk_open(ctrl);

		if (auto_record_task_t == NULL && monitor_enter_way_get() != MONITOR_ENTER_TUYA && record_message_ing == false) /* */
		{
			extern void jpg_push_to_tuya(int type);
			jpg_push_to_tuya(1);
			auto_record_task_t = lv_task_create(auto_record_task, 2000, LV_TASK_PRIO_HIGH, NULL);
		}
	}
	if (!is_sdcard_insert())
	{
		if (tuya_online_status_get() == true)
		{
			extern bool send_tuya_record(char record_mode);
			send_tuya_record(REC_MODE_TUYA);
		}
	}
}

#ifdef INTERCOM_VERSION
CREATE_LAYOUT(monitor1);
#else
void layout_monitor_init(void)
{

	outdoor_call_event_register(monitor_call_extern_func); // 户外机呼叫事件注册表 回调
	/*  if (tuya_event_extern_proc == NULL)
{
Debug("=========layout_monitor_init=========>>>>\n\n\n\n\n\n\n");
}

Debug("=========layout_monitor_init=========>>>>\n\n\n\n\n\n\n"); */
	mechanical_key_event_register(mechanical_key_extern_callback);
	tuya_event_register(tuya_event_extern_proc); // 涂鸦时间注册表
}
CREATE_LAYOUT(monitor);
#endif