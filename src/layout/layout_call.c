#include "layout_define.h"
#include "leo_api.h"

typedef enum call_module_list
{
	CALL_OUT_MODULE,
	CALL_IN_HAND_MODULE,
	CALL_IN_ANSWER_MODULE,
	CALL_TALK_HAND_MODULE,
	CALL_HAND_MODULE,
	TOTAL_MODULE
} call_module_list;

#define CALL_MODULE_COORDINATE_INIT { \
	{432, 224, 160, 160},             \
	{594, 224, 160, 160},             \
	{270, 224, 160, 160},             \
	{432, 224, 160, 160},             \
	{432, 224, 160, 160},             \
};

enum
{
	CALL_OUT = 1,
	RECEIVE_CALL,
	ANSWER_CALL,
	HANG_UP,
	BUSY_EQUIPMENT,
	HANG_UP_OTHER,
} interphone_call_msg;

static int call_timeout_flag = 0;
extern unsigned long long os_get_ms(void);
static void call_event_inside_func(unsigned long arg1, unsigned long arg2);
static void call_event_extern_func(unsigned long arg1, unsigned long arg2);

network_cmd_data temp_data = {.cmd = NET_COMMON_CMD_NONE};
interphone_status_enum call_status = INTERPHONE_STATUS_IDLE;
static void call_icon_display(void);
static lv_task_t *call_timer_ptask = NULL;
static lv_task_t *temp_data_ptask = NULL;
static unsigned long long call_timer_start_ms = 0;
static int call_timer_ms = 0;
network_device call_num;
int call_family_id = -1;

static void ID_display(int ID)
{
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);

	lv_obj_set_pos(label, 42, 42);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	static char buf[32] = {0};
	sprintf(buf, "%s: %0d", text_str(STR_LOCAL_DEVICE_ID), user_data_get()->other.network_device);
	lv_label_set_text(label, buf);

	lv_obj_t *label1 = lv_label_create(lv_scr_act(), label);
	lv_obj_set_pos(label, 820, 42);

	static char buf1[32] = {0};
	if (ID == 0xFF)
	{
		sprintf(buf1, "%s: ALL", text_str(STR_CALLED_DEVICE_ID));
	}
	else
	{
		sprintf(buf1, "%s: %0d", text_str(STR_CALLED_DEVICE_ID), ID);
	}
	lv_label_set_text(label1, buf1);
}

static void call_out_btn_up(lv_obj_t *obj)
{
	Debug("locat family_id:%d  call_family_id:%d\n", user_data_get()->other.family_id, call_family_id);
	temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
	temp_data.arg1 = CALL_OUT;
	temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);

	temp_data.device = call_num;
	network_send_cmd_data(&temp_data);
	call_status = INTERPHONE_STATUS_PUBLISH;
	call_timer_start_ms = os_get_ms();
	call_icon_display();
}

static void call_out_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_out_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_CALL_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_CALL_FOCUS_PNG);
	lv_obj_t *btu = home_btn_create_2(coordinate, &btn_data, &info, &info1);
	lv_obj_set_id(btu, 1);
}

static void call_hand_up_btn_up(lv_obj_t *obj)
{
	audio_play_stop_set();
	audio_talk_close(true);
	temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
	temp_data.arg1 = HANG_UP;
	temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);

	temp_data.device = call_num;
	network_send_cmd_data(&temp_data);
	call_status = INTERPHONE_STATUS_IDLE;
	goto_layout(pLAYOUT(standby));
}

static void call_hand_up_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_hand_up_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_HANGUP_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_HANGUP_FOCUS_PNG);
	lv_obj_t *btu = home_btn_create_2(coordinate, &btn_data, &info, &info1);
	lv_obj_set_id(btu, 2);
}

static void call_in_hand_up_btn_up(lv_obj_t *obj)
{
	audio_play_stop_set();
	audio_talk_close(true);
	temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
	temp_data.arg1 = HANG_UP;
	temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);

	temp_data.device = call_num;
	network_send_cmd_data(&temp_data);
	call_status = INTERPHONE_STATUS_IDLE;
	goto_layout(pLAYOUT(standby));
}

static void call_in_hand_up_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_in_hand_up_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_HANGUP_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_HANGUP_FOCUS_PNG);
	lv_obj_t *btu = home_btn_create_2(coordinate, &btn_data, &info, &info1);
	lv_obj_set_id(btu, 3);
}

static void call_in_answer_btn_up(lv_obj_t *obj)
{
	temp_data.device = call_num;
	temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
	temp_data.arg1 = ANSWER_CALL;
	temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);
	network_send_cmd_data(&temp_data);

	call_status = INTERPHONE_STATUS_TALK;

	audio_play_stop_set();

	audio_talk_ctrl ctrl = {{call_num, call_family_id}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN) | OPERATION_OPTION(AUDIO_OUT_EN) | OPERATION_OPTION(AUDIO_IN_EN)), AI_AO_O, true, true, 84};
	audio_talk_open(ctrl);

	audio_output_volume_set(84);
	call_timer_start_ms = os_get_ms();
	call_icon_display();
}

static void call_in_answer_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_in_answer_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_CALL_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_CALL_FOCUS_PNG);
	lv_obj_t *btu = home_btn_create_2(coordinate, &btn_data, &info, &info1);
	btn_data.obj_tone = false;
	lv_obj_set_id(btu, 4);
}

static void call_talk_hand_up_btn_up(lv_obj_t *obj)
{
	audio_play_stop_set();
	audio_talk_close(true);

	temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
	temp_data.arg1 = HANG_UP;
	temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);

	temp_data.device = call_num;
	network_send_cmd_data(&temp_data);

	call_status = INTERPHONE_STATUS_IDLE;
	goto_layout(pLAYOUT(standby));
}

static void call_talk_hand_up_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_talk_hand_up_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_HANGUP_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_DIAL_HANGUP_FOCUS_PNG);
	lv_obj_t *btu = home_btn_create_2(coordinate, &btn_data, &info, &info1);
	lv_obj_set_id(btu, 5);
}

static void call_icon_display(void)
{
	lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), 1);
	lv_obj_t *obj2 = lv_obj_get_child_form_id(lv_scr_act(), 2);
	lv_obj_t *obj3 = lv_obj_get_child_form_id(lv_scr_act(), 3);
	lv_obj_t *obj4 = lv_obj_get_child_form_id(lv_scr_act(), 4);
	lv_obj_t *obj5 = lv_obj_get_child_form_id(lv_scr_act(), 5);
	lv_obj_set_hidden(obj1, true);
	lv_obj_set_hidden(obj2, true);
	lv_obj_set_hidden(obj3, true);
	lv_obj_set_hidden(obj4, true);
	lv_obj_set_hidden(obj5, true);

	if (call_status == INTERPHONE_STATUS_IDLE)
	{
		lv_obj_set_hidden(obj1, false);
	}
	else if (call_status == INTERPHONE_STATUS_PUBLISH)
	{
		call_timer_ms = 3000;
		call_timeout_flag = 1;
		lv_obj_set_hidden(obj2, false);
	}
	else if (call_status == INTERPHONE_STATUS_OUT)
	{
		call_timer_ms = 60000;
		call_timeout_flag = 2;
		lv_obj_set_hidden(obj2, false);
	}
	else if (call_status == INTERPHONE_STATUS_IN)
	{
		call_timer_ms = 60000;
		call_timeout_flag = 2;
		lv_obj_set_hidden(obj3, false);
		lv_obj_set_hidden(obj4, false);
	}
	else if (call_status == INTERPHONE_STATUS_TALK)
	{
		call_timer_ms = 120000;
		call_timeout_flag = 3;
		lv_obj_set_hidden(obj5, false);
	}
}

static void call_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(home));
}

static lv_task_t *msgbox_task_t = NULL;
static void call_msgbox_task(lv_task_t *task_t)
{

	lv_obj_t *msgbox_cont = lv_obj_get_child_form_id(lv_scr_act(), 666);
	if (msgbox_cont != NULL)
	{
		lv_obj_del(msgbox_cont);
	}
	temp_data.cmd = NET_COMMON_CMD_NONE;
	lv_task_del(msgbox_task_t);
	msgbox_task_t = NULL;
	goto_layout(pLAYOUT(home));
}

void call_msgbox_create(char *str)
{
	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	lv_obj_set_id(window_cont, 666);

	lv_obj_t *msgbox_cont = lv_cont_create(window_cont, NULL);

	lv_obj_set_pos(msgbox_cont, 228, 103);
	lv_obj_set_size(msgbox_cont, 648, 441);
	lv_obj_set_style_local_bg_color(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00131D));
	lv_obj_set_style_local_bg_opa(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 18);

	lv_obj_t *img = lv_img_create(msgbox_cont, NULL);
	lv_obj_set_pos(img, 0, 82);
	lv_obj_set_size(img, 648, 3);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_MSGBOX_LINE_PNG);
	lv_img_set_src(img, &info1);

	lv_obj_t *window_head_label = lv_label_create(msgbox_cont, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(window_head_label, str);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_align(window_head_label, msgbox_cont, LV_ALIGN_CENTER, 0, -30);

	msgbox_task_t = lv_task_create(call_msgbox_task, 800, LV_TASK_PRIO_HIGH, NULL);
}

void call_msg_text(int state)
{
	if (state == 1)
	{

		call_msgbox_create(text_str(STR_CALL_TIMEOUT));

		temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
		temp_data.arg1 = BUSY_EQUIPMENT;
		temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);

		temp_data.device = call_num;
		network_send_cmd_data(&temp_data);
	}
	else if (state == 2)
	{
		call_msgbox_create(text_str(STR_WAIT_TIMEOUT));
	}
	else if (state == 3)
	{
		call_msgbox_create(text_str(STR_CALL_TIMED_OUT));
	}
}

static void call_timer_task(struct _lv_task_t *task_t)
{
	if (call_timer_start_ms > 0 && call_timer_ms > 0)
	{
		if ((os_get_ms() - call_timer_start_ms) > call_timer_ms)
		{

			call_msg_text(call_timeout_flag);
			if (call_timer_ptask != NULL)
			{
				lv_task_del(call_timer_ptask);
				call_timer_ptask = NULL;
			}
		}
	}
}

static void temp_data_task(struct _lv_task_t *task_t)
{
	// static int send_count = 0;
	// send_count = send_count == 0 ? 3 : send_count - 1;
	// if(send_count)
	// {
	network_send_cmd_data(&temp_data);
	// }
	// else
	// {
	if (temp_data_ptask != NULL)
	{
		lv_task_del(temp_data_ptask);
		temp_data_ptask = NULL;
	}
	// }
}

static void LAYOUT_ENETER_FUNC(call)
{
	Debug("=========================>>\n\r");
	if (call_family_id == -1)
	{
		call_family_id = user_data_get()->other.family_id;
	}

	Controls_location module_coordinate[] = CALL_MODULE_COORDINATE_INIT;
	// standby_timer_close();
	home_bg_display();
	interphone_call_event_register(call_event_inside_func);
	call_out_btn_create(module_coordinate[CALL_OUT_MODULE]);
	call_hand_up_btn_create(module_coordinate[CALL_HAND_MODULE]);
	call_in_hand_up_btn_create(module_coordinate[CALL_IN_HAND_MODULE]);
	call_in_answer_btn_create(module_coordinate[CALL_IN_ANSWER_MODULE]);
	call_talk_hand_up_btn_create(module_coordinate[CALL_TALK_HAND_MODULE]);
	call_icon_display();

	ID_display(call_num);
	home_back_btn_create(call_back_btn_up, NULL);

	if (call_timer_ptask != NULL)
	{
		lv_task_del(call_timer_ptask);
	}
	call_timer_ptask = lv_task_create(call_timer_task, 500, LV_TASK_PRIO_HIGH, NULL);
	backlight_open(true, false, user_data_get()->other.brightness);
}

static void LAYOUT_QUIT_FUNC(call)
{
	Debug("=========================>>\n\r");

	temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
	temp_data.arg1 = HANG_UP;
	temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);

	Debug("call_family_id:%d\n", call_family_id);
	temp_data.device = call_num;
	network_send_cmd_data(&temp_data);

	call_family_id = -1;
	call_timer_ms = 0;
	call_timer_start_ms = 0;
	if (call_timer_ptask != NULL)
	{
		lv_task_del(call_timer_ptask);
		call_timer_ptask = NULL;
	}
	if (msgbox_task_t != NULL)
	{
		lv_task_del(msgbox_task_t);
		msgbox_task_t = NULL;
	}

	if (temp_data_ptask != NULL)
	{
		lv_task_del(temp_data_ptask);
		temp_data_ptask = NULL;
	}

	if (current_layout_get() == &layout_monitor && user_data_get()->other.model != MUTE_PATTERN)
	{
	}
	else
	{
		audio_play_stop_set();
	}

	audio_talk_close(true);
	call_status = INTERPHONE_STATUS_IDLE;
	call_num = DEVICE_UNKONW;
	interphone_call_event_register(call_event_extern_func);

	standby_timer_open(-1, NULL);
}

static void interphone_ring_play_finsih_callback(void)
{
	if ((call_status == INTERPHONE_STATUS_OUT) || (call_status == INTERPHONE_STATUS_IN))
	{
		interphone_ring_play(6, NULL, false, interphone_ring_play_finsih_callback);
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
static void call_event_extern_func(unsigned long arg1, unsigned long arg2)
{
	Debug("arg1 :%ld    call device:%ld        call_status:%d\n", arg1 >> 4, arg1 >> 4, call_status);
	network_device device = (network_device)(arg1 >> 4);
	arg1 = arg1 & 0x0F;
	if (arg1 == CALL_OUT)
	{
		if (call_status == INTERPHONE_STATUS_IDLE)
		{
			call_num = device;
			call_family_id = arg2 >> 4;
			Debug("call_family_id:%d\n", call_family_id);
			temp_data.device = call_num;
			temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			temp_data.arg1 = RECEIVE_CALL;
			temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);
			network_send_cmd_data(&temp_data);

			call_status = INTERPHONE_STATUS_IN;
			goto_layout(pLAYOUT(call));

			if (user_data_get()->other.model != MUTE_PATTERN)
			{
				interphone_ring_play(6, NULL, false, interphone_ring_play_finsih_callback);
			}
		}
	}
}

static void call_event_inside_func(unsigned long arg1, unsigned long arg2)
{
	Debug("arg1 :%ld    call device:%ld        call_status:%d\n", arg1 >> 4, arg1 >> 4, call_status);
	network_device device = (network_device)(arg1 >> 4);
	arg1 = arg1 & 0x0F;
	if (arg1 == CALL_OUT)
	{
		if (call_status == INTERPHONE_STATUS_IDLE)
		{
			call_num = device;
			call_family_id = arg2 >> 4;
			Debug("call_family_id:%d\n", call_family_id);

			temp_data.device = call_num;
			temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			temp_data.arg1 = RECEIVE_CALL;
			temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);
			network_send_cmd_data(&temp_data);

			if (temp_data_ptask != NULL)
			{
				lv_task_del(temp_data_ptask);
			}
			temp_data_ptask = lv_task_create(temp_data_task, 200, LV_TASK_PRIO_HIGH, NULL);

			call_status = INTERPHONE_STATUS_IN;
			call_timer_start_ms = os_get_ms();
			call_icon_display();
			if (user_data_get()->other.model != MUTE_PATTERN)
			{
				interphone_ring_play(6, NULL, false, interphone_ring_play_finsih_callback);
			}
		}
		else
		{
			temp_data.device = device;
			temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			temp_data.arg1 = BUSY_EQUIPMENT;
			temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (arg2 & 0x0F);
			network_send_cmd_data(&temp_data);
		}
	}
	else if (arg1 == RECEIVE_CALL)
	{
		if (call_status != INTERPHONE_STATUS_PUBLISH)
		{
			return;
		}

		call_status = INTERPHONE_STATUS_OUT;
		call_timer_start_ms = os_get_ms();
		call_icon_display();
		if (user_data_get()->other.model != MUTE_PATTERN)
		{
			interphone_ring_play(6, NULL, false, interphone_ring_play_finsih_callback);
		}
	}
	else if (arg1 == ANSWER_CALL)
	{
		if (call_status == INTERPHONE_STATUS_OUT)
		{

			if (call_num == DEVICE_ALL)
			{
				call_num = device;
				temp_data.device = DEVICE_ALL;
				temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
				temp_data.arg1 = HANG_UP_OTHER;
				temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);
				network_send_cmd_data(&temp_data);
			}

			if (device == call_num)
			{
				audio_play_stop_set();
				audio_talk_ctrl ctrl = {{device, call_family_id}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN) | OPERATION_OPTION(AUDIO_OUT_EN) | OPERATION_OPTION(AUDIO_IN_EN)), AI_AO_O, true, true, 84};
				audio_talk_open(ctrl);
				audio_output_volume_set(84);
				call_status = INTERPHONE_STATUS_TALK;
				call_timer_start_ms = os_get_ms();
				call_icon_display();
			}
		}
		else
		{
			temp_data.device = device;
			temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			temp_data.arg1 = BUSY_EQUIPMENT;
			temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);
			network_send_cmd_data(&temp_data);
		}
	}
	else if (arg1 == HANG_UP)
	{
		if (device == call_num || call_num == DEVICE_ALL)
		{
			call_status = INTERPHONE_STATUS_IDLE;
			goto_layout(pLAYOUT(standby));
		}
	}
	else if (arg1 == BUSY_EQUIPMENT) // 设备繁忙
	{
		if (device == call_num || call_num == DEVICE_ALL)
		{
			call_status = INTERPHONE_STATUS_IDLE;

			temp_data.cmd = NET_COMMON_CMD_INTERCOM_CALL;
			temp_data.arg1 = BUSY_EQUIPMENT;
			temp_data.arg2 = (char)(user_data_get()->other.family_id) << 4 | (call_family_id);

			temp_data.device = call_num;
			network_send_cmd_data(&temp_data);

			goto_layout(pLAYOUT(standby));
		}
	}
	else if (arg1 == HANG_UP_OTHER) // 设备繁忙
	{
		if (call_status != INTERPHONE_STATUS_TALK)
		{
			call_status = INTERPHONE_STATUS_IDLE;
			goto_layout(pLAYOUT(standby));
		}
	}
}

void layout_call_init(void)
{
	interphone_call_event_register(call_event_extern_func);
}

CREATE_LAYOUT(call);
