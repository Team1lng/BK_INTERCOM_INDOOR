#include "layout_define.h"
#include "leo_api.h"

typedef enum monitor_1_module_list
{
	DOOR1_MODULE,
	DOOR2_MODULE,

#ifdef CAMERA_MODULE_ENABLE
	CAMERA1_MODULE,
	CAMERA2_MODULE,
#endif

	TOTAL_MODULE
} monitor_1_module_list;

#ifdef CAMERA_MODULE_ENABLE
#define MONITOR_1_MODULE_COORDINATE_INIT { \
	{336, 100, 130, 130},                  \
	{574, 100, 130, 130},                  \
	{336, 320, 130, 130},                  \
	{574, 320, 130, 130},                  \
};
#else
#define MONITOR_1_MODULE_COORDINATE_INIT { \
	{336, 210, 130, 130},                  \
	{574, 210, 130, 130},                  \
};
#endif

bool get_outdoor_talk_state(MONITOR_CH ch);

static void monitor_1_door1_btn_up(lv_obj_t *obj)
{
	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2)) // 正在视频对讲其他机子无法操作
	{
		return;
	}
	system_bg_data_backup();			// 背景颜色恢复
	monitor_channel_set(MON_CH_DOOR_1); // 通道选择 手动进入就是DOOR1

	audio_talk_ctrl ctrl = {{DEVICE_OUTDOOR_1}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, user_data_get()->door1.talk_volume * 5 + 46};
	audio_talk_open(ctrl);

	monitor_enter_way_set(MONITOR_ENTER_MANUAL);
	goto_layout(pLAYOUT(monitor)); // 页面跳转
}

static void monitor_1_door1_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_1_door1_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_DOOR1_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_DOOR1_FOCUS_PNG);
	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_DOOR1_DISABLE_PNG);
	static rom_bin_info info3 = rom_bin_info_get(ROM_RES_MONITOR_DOOR1_DISABLE_FOCUS_PNG);
	home_btn_create_1(coordinate, text_str(STR_DOOR1), &btn_data, user_data_get()->door1.enable_sw ? &info : &info2, user_data_get()->door1.enable_sw ? &info1 : &info3);
}

static void monitor_1_door2_btn_up(lv_obj_t *obj)
{

	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2)) // 正在视频对讲其他机子无法操作
	{
		return;
	}
	system_bg_data_backup(); // 背景颜色恢复
	if (user_data_get()->door2.enable_sw)
	{
		monitor_channel_set(MON_CH_DOOR_2); // 通道选择 手动进入就是DOOR2
		audio_talk_ctrl ctrl = {{DEVICE_OUTDOOR_2}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, user_data_get()->door2.talk_volume * 5 + 46};
		audio_talk_open(ctrl);
		monitor_enter_way_set(MONITOR_ENTER_MANUAL);
		goto_layout(pLAYOUT(monitor)); // 页面跳转
	}
}

static void monitor_1_door2_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_1_door2_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_DOOR2_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_DOOR2_FOCUS_PNG);
	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_DOOR2_DISABLE_PNG);
	static rom_bin_info info3 = rom_bin_info_get(ROM_RES_MONITOR_DOOR2_DISABLE_FOCUS_PNG);
	Debug("user_data_get()->door2.enable_sw:%d\n", user_data_get()->door2.enable_sw);
	home_btn_create_1(coordinate, text_str(STR_DOOR2), &btn_data, user_data_get()->door2.enable_sw ? &info : &info2, user_data_get()->door2.enable_sw ? &info1 : &info3);
}

#ifdef CAMERA_MODULE_ENABLE
static void monitor_1_camera1_btn_up(lv_obj_t *obj)
{

	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2)) // 正在视频对讲其他机子无法操作
	{
		return;
	}
	system_bg_data_backup(); // 背景颜色恢复
	if (user_data_get()->camera1.enable)
	{
		monitor_channel_set(MON_CH_CCTV_1); //
		monitor_enter_way_set(MONITOR_ENTER_MANUAL);
		goto_layout(pLAYOUT(monitor)); // 页面跳转
	}
}

static void monitor_1_camera1_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_1_camera1_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_CAMERA1_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_CAMERA1_FOCUS_PNG);
	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_CAMERA1_DISABLE_UNFOCUS_PNG);
	static rom_bin_info info3 = rom_bin_info_get(ROM_RES_MONITOR_CAMERA1_DISABLE_FOCUS_PNG);
	home_btn_create_1(coordinate, text_str(STR_CAMERA1), &btn_data, user_data_get()->camera1.enable ? &info : &info2, user_data_get()->camera1.enable ? &info1 : &info3);
}

static void monitor_1_camera2_btn_up(lv_obj_t *obj)
{

	if (get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2)) // 正在视频对讲其他机子无法操作
	{
		return;
	}

	system_bg_data_backup(); // 背景颜色恢复

	if (user_data_get()->camera2.enable)
	{
		monitor_channel_set(MON_CH_CCTV_2);
		monitor_enter_way_set(MONITOR_ENTER_MANUAL);
		goto_layout(pLAYOUT(monitor)); // 页面跳转
	}
}

static void monitor_1_camera2_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, monitor_1_camera2_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_CAMERA2_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_CAMERA2_FOCUS_PNG);
	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_CAMERA2_DISABLE_UNFOCUS_PNG);
	static rom_bin_info info3 = rom_bin_info_get(ROM_RES_MONITOR_CAMERA2_DISABLE_FOCUS_PNG);
	home_btn_create_1(coordinate, text_str(STR_CAMERA2), &btn_data, user_data_get()->camera2.enable ? &info : &info2, user_data_get()->camera2.enable ? &info1 : &info3);
}
#endif

static void monitor_1_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(home));
}

static void LAYOUT_ENETER_FUNC(monitor_1)
{
	Controls_location module_coordinate[] = MONITOR_1_MODULE_COORDINATE_INIT;
	home_bg_display();
	monitor_1_door1_btn_create(module_coordinate[DOOR1_MODULE]);
	monitor_1_door2_btn_create(module_coordinate[DOOR2_MODULE]);
	if (wifi_usb_module_enable())
	{
#ifdef CAMERA_MODULE_ENABLE
		monitor_1_camera1_btn_create(module_coordinate[CAMERA1_MODULE]);
		monitor_1_camera2_btn_create(module_coordinate[CAMERA2_MODULE]);
#endif
	}

	home_back_btn_create(monitor_1_back_btn_up, NULL);
}

static void LAYOUT_QUIT_FUNC(monitor_1)
{
}

CREATE_LAYOUT(monitor_1);
