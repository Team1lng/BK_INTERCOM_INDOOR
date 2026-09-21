#include "layout_define.h"
#include "leo_api.h"

// #define ADMIN_POWER_ENABLE

// #define ADMIN_RESTORE_ENABLE

static int network_temp_family_id;

#define ADMIN_MODULE_COORDINATE_INIT { \
	{199, 75, 700, 52},                \
	{199, 127, 700, 52},               \
	{199, 179, 700, 52},               \
	{199, 231, 700, 52},               \
	{199, 285, 700, 52},               \
	{199, 339, 700, 52},               \
	{199, 393, 700, 52},               \
	{199, 447, 700, 52},               \
};

extern linked_info link_info;

static void admin_setting_display(void);

static void admin_setting_img_text_display(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_ADMIN_PNG);
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_value_str(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_ADMINISTRATORS));
	lv_obj_set_style_local_value_color(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_align(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
}

static void admin_set_btn_syn_up(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_DEFAULT);
}
static void admin_set_btn_syn_down(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_PRESSED);
}

static void admin_set_btn_syn_event(lv_obj_t *obj, lv_event_t event)
{

	if (LV_EVENT_PRESS_LOST == event)
	{
		admin_set_btn_syn_down(obj);
	}
}

#ifdef ADMIN_POWER_ENABLE
static void admin_admin_right_btn_up(lv_obj_t *obj)
{
	admin_set_btn_syn_up(obj);
	char *str1 = NULL;
	user_data_get()->admin_sw = !user_data_get()->admin_sw;
	str1 = user_data_get()->admin_sw ? text_str(STR_ON) : text_str(STR_OFF);

	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 1);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void admin_admin_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(admin_set_btn_syn_down, admin_admin_right_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(admin_set_btn_syn_down, admin_admin_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = admin_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = admin_set_btn_syn_event;
	char *str1 = NULL;
	str1 = user_data_get()->admin_sw ? text_str(STR_ON) : text_str(STR_OFF);

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_ADMINISTRATORS), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 1);
	(*coordinate)++;
}
#endif

static void admin_apartment_number_set_left_btn_up(lv_obj_t *obj)
{
	admin_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 2);
	static char str1[4] = {0};
	if (--user_data_get()->other.family_id < 1)
	{
		user_data_get()->other.family_id = 4;
	}

	sprintf(str1, "%d", user_data_get()->other.family_id);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	network_devices_enable_init();
}
static void admin_apartment_number_set_right_btn_up(lv_obj_t *obj)
{
	admin_set_btn_syn_up(obj);
	lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), 2);
	static char str1[4] = {0};
	if (++user_data_get()->other.family_id > 4)
	{
		user_data_get()->other.family_id = 1;
	}

	sprintf(str1, "%d", user_data_get()->other.family_id);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	network_devices_enable_init();
}

static void admin_apartment_number_set_btn_create(Controls_location **coordinate)
{

	static btn_data btn_data1 = btn_data_create(admin_set_btn_syn_down, admin_apartment_number_set_left_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(admin_set_btn_syn_down, admin_apartment_number_set_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = admin_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = admin_set_btn_syn_event;
	static char str1[4] = {0};
	sprintf(str1, "%d", user_data_get()->other.family_id);

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_ROOM_NUMBER), &btn_data3, &btn_data1, &btn_data2);
	lv_obj_set_id(btn, 2);
	(*coordinate)++;
}

static void admin_router_address_set_btn_up(lv_obj_t *obj)
{
	admin_set_btn_syn_up(obj);
}

static void admin_router_address_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(admin_set_btn_syn_down, admin_router_address_set_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(admin_set_btn_syn_down, admin_router_address_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = admin_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = admin_set_btn_syn_event;
	linked_info link_info = {0};
	static char ip[32] = {0};
	// printf_connected_info();
	if (user_data_get()->pairing_mode == WLAN_NET)
	{
		get_linked_wifi_info(&link_info);
		sprintf(ip, "%s", link_info.ip);
	}
	else
	{
		net_util_get_ipaddr("eth2", ip);
	}

	sys_setting_btn_create(**coordinate, ip, text_str(STR_ROUTER_ADDRESS), &btn_data3, &btn_data1, &btn_data2);
	(*coordinate)++;
}

extern int get_pwd_str;

static void admin_change_password_btn_up(lv_obj_t *obj)
{
	get_pwd_str = 1;
	goto_layout(pLAYOUT(password_input));
}
static void admin_change_password_btn_up_1(lv_obj_t *obj)
{
	get_pwd_str = 1;
	admin_set_btn_syn_up(obj);
	goto_layout(pLAYOUT(password_input));
}

static void admin_change_password_set_btn_create(Controls_location **coordinate)
{

	static btn_data btn_data3 = btn_data_create(NULL, admin_change_password_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(admin_set_btn_syn_down, admin_change_password_btn_up_1, NULL);

	sys_setting_btn_create(**coordinate, user_data_get()->user_pwd, text_str(STR_CHANGE_PASSWORD), &btn_data3, NULL, &btn_data2);
	(*coordinate)++;
}

#ifdef ADMIN_CARD_MANAGEMENT
static void admin_card_management_btn_up(lv_obj_t *obj)
{
	data_manage_type_set(MANAGE_CARD);
	goto_layout(pLAYOUT(data_list));
}
static void admin_card_management_switch_btn_up(lv_obj_t *obj)
{
	admin_set_btn_syn_up(obj);
	data_manage_type_set(MANAGE_CARD);
	goto_layout(pLAYOUT(data_list));
}
static void card_management_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data3 = btn_data_create(NULL, admin_card_management_btn_up, NULL);
	static btn_data btn_data1 = btn_data_create(admin_set_btn_syn_down, admin_card_management_switch_btn_up, NULL);
	btn_data1.OPS_ANYTHING = admin_set_btn_syn_event;
	user_data_get()->card_management = true;
	char *str = text_str(STR_CARD_MANAGE);
	sys_setting_btn_create(**coordinate, str, text_str(STR_CARD_MANAGE), &btn_data3, NULL, &btn_data1);
	(*coordinate)++;
}
#endif

#ifdef ADMIN_FINGER_MANAGEMENT
static void admin_finger_management_btn_up(lv_obj_t *obj)
{
	data_manage_type_set(MANAGE_FINGER);
	goto_layout(pLAYOUT(data_list));
}

static void admin_finger_management_switch_btn_up(lv_obj_t *obj)
{
	admin_set_btn_syn_up(obj);
	data_manage_type_set(MANAGE_FINGER);
	goto_layout(pLAYOUT(data_list));
}
static void finger_management_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data3 = btn_data_create(NULL, admin_finger_management_btn_up, NULL);
	static btn_data btn_data1 = btn_data_create(admin_set_btn_syn_down, admin_finger_management_switch_btn_up, NULL);
	btn_data1.OPS_ANYTHING = admin_set_btn_syn_event;
	user_data_get()->fingetprint_management = true;
	char *str = text_str(STR_FINGER_MANAGE);
	sys_setting_btn_create(**coordinate, str, text_str(STR_FINGER_MANAGE), &btn_data3, NULL, &btn_data1);
	(*coordinate)++;
}
#endif

#ifdef ADMIN_PASSW_MANAGEMENT
static void admin_passw_management_btn_up_1(lv_obj_t *obj)
{
	data_manage_type_set(MANAGE_PASSW);
	goto_layout(pLAYOUT(data_list));
}
static void passw_management_set_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data3 = btn_data_create(NULL, admin_passw_management_btn_up_1, NULL);
	static btn_data btn_data1 = btn_data_create(admin_set_btn_syn_down, admin_passw_management_btn_up_1, NULL);
	btn_data1.OPS_ANYTHING = admin_set_btn_syn_event;
	sys_setting_btn_create(**coordinate, text_str(STR_PASSW_MANAGE), text_str(STR_PASSW_MANAGE), &btn_data3, NULL, &btn_data1);
	(*coordinate)++;
}
#endif

#ifdef ADMIN_RESTORE_ENABLE
static void window_admin_restore_yse_btn_up(lv_obj_t *obj)
{

	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}

	strncpy(user_data_get()->user_pwd, "111111", 6);
	user_data_get()->admin_sw = 0;
	user_data_get()->other.family_id = 1;
	network_local_family_set(user_data_get()->other.family_id);

	goto_layout(pLAYOUT(setting_admin));
}

static void window_admin_restore_no_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
}

static void admin_restore_window_create(void)
{
	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	lv_obj_set_id(window_cont, 888);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_3_PNG);
	lv_obj_t *window_img = lv_img_create(window_cont, NULL);
	lv_obj_set_pos(window_img, 228, 103);
	lv_obj_set_size(window_img, 648, 441);
	lv_img_set_src(window_img, &info);
	lv_obj_set_id(window_img, 666);

	lv_obj_t *window_head_label = lv_label_create(window_img, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(window_head_label, text_str(STR_RESTORE_ADMIN));
	lv_label_set_align(window_head_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_align(window_head_label, window_img, LV_ALIGN_CENTER, 0, -30);

	lv_obj_t *window_ok_btn = lv_btn_create(window_img, NULL);
	lv_obj_set_pos(window_ok_btn, 0, 360);
	lv_obj_set_size(window_ok_btn, 320, 77);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static btn_data btn_data1 = btn_data_create(NULL, window_admin_restore_yse_btn_up, NULL);

	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_YES));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_YES));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	lv_obj_t *window_cancel_btn = lv_btn_create(window_img, window_ok_btn);
	lv_obj_set_x(window_cancel_btn, 324);

	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_NO));
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_NO));
	static btn_data btn_data2 = btn_data_create(NULL, window_admin_restore_no_btn_up, NULL);
	window_cancel_btn->user_data = &btn_data2;
	btn_touch_event_listen(window_cancel_btn);
}

static void admin_restore_btn_up(lv_obj_t *obj)
{
	admin_restore_window_create();
}
static void admin_restore_btn_up_1(lv_obj_t *obj)
{
	admin_set_btn_syn_up(obj);

	admin_restore_window_create();
}

static void admin_restore_set_btn_create(Controls_location **coordinate)
{

	static btn_data btn_data3 = btn_data_create(NULL, admin_restore_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(admin_set_btn_syn_down, admin_restore_btn_up_1, NULL);

	sys_setting_btn_create(coordinate, text_str(STR_CONFIRM), text_str(STR_RESTOR), &btn_data3, NULL, &btn_data2);
	(*coordinate)++;
}
#endif

static void admin_setting_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_sys));
}

static void admin_setting_display(void)
{

	Controls_location module_coordinate[] = ADMIN_MODULE_COORDINATE_INIT;
	Controls_location *module_p = &module_coordinate[0];
	admin_setting_img_text_display();

#ifdef ADMIN_POWER_ENABLE
	admin_admin_set_btn_create(&module_p);
#endif

	admin_apartment_number_set_btn_create(&module_p);

	if (wifi_usb_module_enable())
	{
		admin_router_address_set_btn_create(&module_p);
	}

	admin_change_password_set_btn_create(&module_p);

#ifdef ADMIN_CARD_MANAGEMENT
	card_management_set_btn_create(&module_p);
#endif

#ifdef ADMIN_FINGER_MANAGEMENT
	if (get_outdoor_finerger_status(DEVICE_OUTDOOR_1) || get_outdoor_finerger_status(DEVICE_OUTDOOR_2))
		finger_management_set_btn_create(&module_p);
#endif

#ifdef ADMIN_PASSW_MANAGEMENT
	passw_management_set_btn_create(&module_p);
#endif

#ifdef ADMIN_RESTORE_ENABLE
	admin_restore_set_btn_create(&module_p);
#endif

	home_back_btn_create(admin_setting_back_btn_up, NULL);
}

static void LAYOUT_ENETER_FUNC(setting_admin)
{
	setting_bg_display();
	admin_setting_display();
}

static void LAYOUT_QUIT_FUNC(setting_admin)
{
	user_data_save();

	if (network_temp_family_id != user_data_get()->other.family_id)
	{
		network_local_family_set(user_data_get()->other.family_id);
		network_local_mac_set();
	}
}

CREATE_LAYOUT(setting_admin);
