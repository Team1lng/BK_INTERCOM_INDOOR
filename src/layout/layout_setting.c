#include "layout_define.h"
#include "leo_api.h"

extern bool set_door_ring_flag;

typedef enum setting_module_list
{
	SYSTEM_MODULE,
	DOOR_MODULE,

#ifdef CAMERA_MODULE_ENABLE
	CAMERA_MODULE,
#endif

	NETWORK_MODULE,
	SCENE_MODULE,
	SENIOR_MODULE,
	INFOMATION_MODULE,
} setting_module_list;

#define SETTING_MODULE_COORDINATE_INIT { \
	{157, 100, 130, 130},                \
	{356, 100, 130, 130},                \
	{554, 100, 130, 130},                \
	{752, 100, 130, 130},                \
	{157, 320, 130, 130},                \
	{356, 320, 130, 130},                \
	{555, 320, 130, 130}};

void setting_bg_display(void)
{
	Debug("\n\n\n\n");
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
	system_bg_fill_color(0xff00131D, 0, 0, 190, 600);
	// system_bg_fill_color(0xff002538,190,0,16,600);
	system_bg_fill_color(0xff002538, 190, 0, 834, 600);
	system_bg_fill_color(0xff01344F, 200, 0, 252, 600);
}

void home_bg_display(void)
{
	// system_bg_fill_color(0xff002538,0,0,1024,600);
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_disp_set_bg_color(lv_disp_get_default(), lv_color_hex(0xff002538));
}
void music_bg_display(void)
{
	// system_bg_fill_color(0xff002538,0,0,1024,600);
	lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
	lv_disp_set_bg_color(lv_disp_get_default(), lv_color_hex(0xff000000));
}

static void setting_1_sys_set_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_sys));
}

static void setting_1_sys_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, setting_1_sys_set_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_SETTING_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_SETTING_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_SYSTEM_SET), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void setting_1_door_setting_btn_up(lv_obj_t *obj)
{
	set_door_ring_flag = 0;
	goto_layout(pLAYOUT(setting_door));
}

static void setting_1_door_setting_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, setting_1_door_setting_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_DOOR_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_DOOR_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_DOOR_SET), &btn_data, &info, &info1);
	(*coordinate)++;
}

#ifdef CAMERA_MODULE_ENABLE
static void setting_1_camera_setting_btn_up(lv_obj_t *obj)
{
#ifdef DHCP_IPCAMERA
	goto_layout(pLAYOUT(setting_ipc));
#else
	goto_layout(pLAYOUT(setting_camera));
#endif
}

static void setting_1_camera_setting_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, setting_1_camera_setting_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_CAMERA_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_CAMERA_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_CAMERA_SET), &btn_data, &info, &info1);
	(*coordinate)++;
}
#endif

static void setting_1_network_setting_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_wifi));
}

static void setting_1_network_setting_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, setting_1_network_setting_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_NETWORK_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_NETWORK_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_NETWORK_SET), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void setting_1_scene_setting_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_scene));
}

static void setting_1_scene_setting_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, setting_1_scene_setting_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_SCENE_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_SCENE_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_SCENE_SET), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void setting_1_senior_setting_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_senior));
}

static void setting_1_senior_setting_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, setting_1_senior_setting_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_SENIOR_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_SENIOR_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_SENIOR_SET), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void setting_1_infomation_btn_up(lv_obj_t *obj)
{

	goto_layout(pLAYOUT(setting_info));
}

static void setting_1_infomation_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, setting_1_infomation_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_INFO_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_INFO_FOCUS_PNG);
	home_btn_create_1(**coordinate, text_str(STR_SYSTEM_INFO), &btn_data, &info, &info1);
	(*coordinate)++;
}

static void setting_1_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(home));
}

static void LAYOUT_ENETER_FUNC(setting)
{
	Controls_location module_coordinate[] = SETTING_MODULE_COORDINATE_INIT;
	Controls_location *module_p = &module_coordinate[0];

	home_bg_display();

	setting_1_sys_set_btn_create(&module_p);
	setting_1_door_setting_btn_create(&module_p);

	if (wifi_usb_module_enable())
	{
#ifdef CAMERA_MODULE_ENABLE
		setting_1_camera_setting_btn_create(&module_p);
#endif
		setting_1_network_setting_btn_create(&module_p);
	}

	setting_1_scene_setting_btn_create(&module_p);
	setting_1_senior_setting_btn_create(&module_p);
	setting_1_infomation_btn_create(&module_p);
	home_back_btn_create(setting_1_back_btn_up, NULL);
	dev_info_status_event_register(dev_info_status_callback);
	dev_info_status_callback(1, 0);
}

static void LAYOUT_QUIT_FUNC(setting)
{
	dev_info_status_event_register(NULL);
	// sdcard_event_register(NULL);
}

CREATE_LAYOUT(setting);
