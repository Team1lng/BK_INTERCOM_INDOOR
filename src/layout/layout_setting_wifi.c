
#include "layout_define.h"
#include "leo_api.h"

static bool is_wifi_page_move = false;
bool wifi_control;
static lv_obj_t *wifi_page = NULL;
static lv_obj_t *qr = NULL;
linked_info link_info = {0};
char connectwifi_name[24] = {0};
int connectwifi_index = 0;
int connected_wifi_max = 0;
static void network_btn_up(lv_obj_t *obj);

void wifi_link_info_refresh(void)
{
	bool continue_flag = true;

	memset(&link_info, 0, sizeof(linked_info));
	wpa_cli_wlan_status(&continue_flag);
	get_linked_wifi_info(&link_info);
}

static void network_window_create(char *str);

typedef enum
{
	NET_PAIRING_BTN,
	WLAN_BTN,
	NET_INFO_BTN,
	QRCODE_INFO_BTN,
} WLAN_INFO_BTN;

WLAN_INFO_BTN WLAN_INFO_B = NET_PAIRING_BTN;

user_net_pairing net_pairing_mode = WLAN_NET;
user_ip_allocation net_alloction_mode = STATIC_ALLOC;
static void network_information_display(void);
static void wifi_page_create(lv_obj_t *parent);

void set_location(lv_obj_t *obj, int x, int y, int w, int h)
{
	lv_obj_set_pos(obj, x, y);
	lv_obj_set_size(obj, w, h);
}

static void alloction_mode_btn_down(lv_obj_t *obj)
{
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), 456);

	if (obj == lv_obj_get_child_form_id(cont, 458) || obj == lv_obj_get_child_form_id(cont, 459))
	{
		if (net_alloction_mode == STATIC_ALLOC)
		{
			net_alloction_mode = UDHCPC_ALLOC;
		}
		else
		{
			net_alloction_mode = STATIC_ALLOC;
		}

		lv_obj_t *obj = lv_obj_get_child_form_id(cont, 457);
		static char str1[32] = {0};

		sprintf(str1, "%s", text_str(net_alloction_mode + STR_STATIC_ALLOC));
		lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	}
}

static void alloction_mode_create(void)
{
	static btn_data btn_data1 = btn_data_create(alloction_mode_btn_down, NULL, NULL);
	static btn_data btn_data2 = btn_data_create(alloction_mode_btn_down, NULL, NULL);
	static btn_data btn_data3 = btn_data_create(alloction_mode_btn_down, NULL, NULL);

	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	set_location(cont, 460, 76, 422, 52);

	lv_obj_set_id(cont, 456);
	lv_obj_set_hidden(cont, user_data_get()->pairing_mode ? false : true);

	static char str1[32] = {0};

	sprintf(str1, "%s", text_str(user_data_get()->allocation_mode + STR_STATIC_ALLOC));
	net_alloction_mode = user_data_get()->allocation_mode;

	lv_obj_t *btn = lv_btn_create(cont, NULL);
	set_location(btn, 52, 0, 318, 52);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	btn_data3.user_data = btn;
	btn->user_data = &btn_data3;
	btn_touch_event_listen(btn);

	lv_obj_set_id(btn, 457);

	{
		lv_obj_t *imgbtn0 = lv_imgbtn_create(cont, NULL);
		lv_obj_set_pos(imgbtn0, 0, 0);
		lv_obj_set_size(imgbtn0, 52, 52);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ARROW_LEFT_UNFOCUS_PNG);
		lv_imgbtn_set_src(imgbtn0, LV_BTN_STATE_RELEASED, &info);

		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ARROW_LEFT_FOCUS_PNG);
		lv_imgbtn_set_src(imgbtn0, LV_BTN_STATE_PRESSED, &info1);
		btn_data1.user_data = btn;
		imgbtn0->user_data = &btn_data1;
		lv_imgbtn_set_checkable(imgbtn0, true);
		btn_touch_event_listen(imgbtn0);
		lv_obj_set_id(imgbtn0, 458);
	}

	{
		lv_obj_t *imgbtn1 = lv_imgbtn_create(cont, NULL);
		lv_obj_set_pos(imgbtn1, 370, 0);
		lv_obj_set_size(imgbtn1, 52, 52);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ARROW_RIGHT_UNFOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_RELEASED, &info);

		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ARROW_RIGHT_FOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_PRESSED, &info1);
		btn_data2.user_data = btn;
		imgbtn1->user_data = &btn_data2;
		lv_imgbtn_set_checkable(imgbtn1, true);
		btn_touch_event_listen(imgbtn1);

		lv_obj_set_id(imgbtn1, 459);
	}

	lv_obj_t *img = lv_img_create(cont, NULL);
	lv_obj_set_pos(img, 0, 50);
	lv_obj_set_size(img, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img, &info1);
}

static void pairing_switch_btn_down(lv_obj_t *obj)
{

#ifdef CLOSE_WIRE_PAIRING
	return;
#endif
	// btn_data *pdata = (btn_data *) obj->user_data;
	// lv_obj_t * btn = (lv_obj_t *) pdata->user_data;
	// lv_obj_set_state(btn,LV_STATE_PRESSED);

	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), 7);

	if (obj == lv_obj_get_child_form_id(cont, 777))
	{
		if (net_pairing_mode == WLAN_NET)
		{
			net_pairing_mode = WIRED_NET;
		}
		else
		{
			net_pairing_mode--;
		}

		lv_obj_t *obj = lv_obj_get_child_form_id(cont, 77);
		static char str1[32] = {0};

		sprintf(str1, "%s", text_str(net_pairing_mode + STR_WLAN_PAIRING));
		lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	}
	else if (obj == lv_obj_get_child_form_id(cont, 7777))
	{
		if (net_pairing_mode == WIRED_NET)
		{
			net_pairing_mode = WLAN_NET;
		}
		else
		{
			net_pairing_mode++;
		}

		lv_obj_t *obj = lv_obj_get_child_form_id(cont, 77);
		static char str1[32] = {0};

		sprintf(str1, "%s", text_str(net_pairing_mode + STR_WLAN_PAIRING));
		lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	}

	if ((obj = lv_obj_get_child_form_id(lv_scr_act(), 456)) != NULL)
	{
		lv_obj_set_hidden(obj, net_pairing_mode ? false : true);
	}
}

static void pairing_switch_create(void)
{

	static btn_data btn_data1 = btn_data_create(pairing_switch_btn_down, NULL, NULL);
	static btn_data btn_data2 = btn_data_create(pairing_switch_btn_down, NULL, NULL);
	static btn_data btn_data3 = btn_data_create(pairing_switch_btn_down, NULL, NULL);

	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	set_location(cont, 460, 24, 422, 52);

	lv_obj_set_id(cont, 7);

	static char str1[32] = {0};
	Debug("user_data_get()->pairing_mode:%d\n", user_data_get()->pairing_mode);
	sprintf(str1, "%s", text_str(user_data_get()->pairing_mode + STR_WLAN_PAIRING));
	net_pairing_mode = user_data_get()->pairing_mode;

	lv_obj_t *btn = lv_btn_create(cont, NULL);
	set_location(btn, 52, 0, 318, 52);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
	lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	btn_data3.user_data = btn;
	btn->user_data = &btn_data3;
	btn_touch_event_listen(btn);

	lv_obj_set_id(btn, 77);

	{
		lv_obj_t *imgbtn0 = lv_imgbtn_create(cont, NULL);
		lv_obj_set_pos(imgbtn0, 0, 0);
		lv_obj_set_size(imgbtn0, 52, 52);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ARROW_LEFT_UNFOCUS_PNG);
		lv_imgbtn_set_src(imgbtn0, LV_BTN_STATE_RELEASED, &info);

		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ARROW_LEFT_FOCUS_PNG);
		lv_imgbtn_set_src(imgbtn0, LV_BTN_STATE_PRESSED, &info1);
		btn_data1.user_data = btn;
		imgbtn0->user_data = &btn_data1;
		lv_imgbtn_set_checkable(imgbtn0, true);
		btn_touch_event_listen(imgbtn0);
		lv_obj_set_id(imgbtn0, 777);
	}

	{
		lv_obj_t *imgbtn1 = lv_imgbtn_create(cont, NULL);
		lv_obj_set_pos(imgbtn1, 370, 0);
		lv_obj_set_size(imgbtn1, 52, 52);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ARROW_RIGHT_UNFOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_RELEASED, &info);

		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ARROW_RIGHT_FOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_PRESSED, &info1);
		btn_data2.user_data = btn;
		imgbtn1->user_data = &btn_data2;
		lv_imgbtn_set_checkable(imgbtn1, true);
		btn_touch_event_listen(imgbtn1);

		lv_obj_set_id(imgbtn1, 7777);
	}

	lv_obj_t *img = lv_img_create(cont, NULL);
	lv_obj_set_pos(img, 0, 50);
	lv_obj_set_size(img, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img, &info1);
}

static void net_pairing_btn_down(lv_obj_t *obj)
{
	if (WLAN_INFO_B == NET_PAIRING_BTN)
	{
		return;
	}

	lv_obj_t *img = lv_obj_get_child_form_id(lv_scr_act(), 0);
	lv_obj_set_hidden(img, false);

	if (WLAN_INFO_B == WLAN_BTN)
	{
		lv_obj_t *img1 = lv_obj_get_child_form_id(lv_scr_act(), 1);
		lv_obj_set_hidden(img1, true);
		lv_obj_del(wifi_page);
		wifi_page = NULL;
	}
	else if (WLAN_INFO_B == NET_INFO_BTN)
	{
		lv_obj_t *img1 = lv_obj_get_child_form_id(lv_scr_act(), 2);
		lv_obj_set_hidden(img1, true);
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 10086));
	}
	else if (WLAN_INFO_B == QRCODE_INFO_BTN)
	{
		lv_obj_t *img3 = lv_obj_get_child_form_id(lv_scr_act(), 3);
		lv_obj_set_hidden(img3, true);
		if (qr)
			lv_obj_set_hidden(qr, true);
	}

	pairing_switch_create();
	alloction_mode_create();
	WLAN_INFO_B = NET_PAIRING_BTN;
}

static void net_pairing_btn_create(void)
{

	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_pos(img, 199, 24);
	lv_obj_set_size(img, 252, 52);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ITEM_FOCUS_PNG);
	lv_img_set_src(img, &info);
	lv_obj_set_id(img, 0);

	static btn_data btn_data1 = btn_data_create(net_pairing_btn_down, NULL, NULL);
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);

	lv_obj_set_pos(btn, 199, 24);
	lv_obj_set_size(btn, 252, 52);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_NET_PAIRING));
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	btn->user_data = &btn_data1;
	btn_touch_event_listen(btn);
}

static void WLAN_btn_down(lv_obj_t *obj)
{
	if (WLAN_INFO_B == WLAN_BTN)
	{
		return;
	}
	lv_obj_t *img = lv_obj_get_child_form_id(lv_scr_act(), 1);
	lv_obj_set_hidden(img, false);

	if (WLAN_INFO_B == NET_PAIRING_BTN)
	{
		lv_obj_t *img1 = lv_obj_get_child_form_id(lv_scr_act(), 0);
		lv_obj_set_hidden(img1, true);
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 7));
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 456));
	}
	else if (WLAN_INFO_B == NET_INFO_BTN)
	{
		lv_obj_t *img1 = lv_obj_get_child_form_id(lv_scr_act(), 2);
		lv_obj_set_hidden(img1, true);
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 10086));
	}
	else if (WLAN_INFO_B == QRCODE_INFO_BTN)
	{
		lv_obj_t *img3 = lv_obj_get_child_form_id(lv_scr_act(), 3);
		lv_obj_set_hidden(img3, true);
		if (qr)
			lv_obj_set_hidden(qr, true);
	}

	wifi_page_create(lv_scr_act());
	WLAN_INFO_B = WLAN_BTN;
}

static void WLAN_btn_create(void)
{

	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_pos(img, 199, 76);
	lv_obj_set_size(img, 252, 52);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ITEM_FOCUS_PNG);
	lv_img_set_src(img, &info);
	lv_obj_set_id(img, 1);
	lv_obj_set_hidden(img, true);

	static btn_data btn_data1 = btn_data_create(WLAN_btn_down, NULL, NULL);
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);

	lv_obj_set_pos(btn, 199, 76);
	lv_obj_set_size(btn, 252, 52);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_WLAN));
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	btn->user_data = &btn_data1;
	btn_touch_event_listen(btn);
}

static void net_info_btn_down(lv_obj_t *obj)
{
	if (WLAN_INFO_B == NET_INFO_BTN)
	{
		return;
	}
	lv_obj_t *img = lv_obj_get_child_form_id(lv_scr_act(), 2);
	lv_obj_set_hidden(img, false);

	if (WLAN_INFO_B == NET_PAIRING_BTN)
	{
		lv_obj_t *img1 = lv_obj_get_child_form_id(lv_scr_act(), 0);
		lv_obj_set_hidden(img1, true);
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 7));
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 456));
	}
	if (WLAN_INFO_B == WLAN_BTN)
	{
		lv_obj_t *img1 = lv_obj_get_child_form_id(lv_scr_act(), 1);
		lv_obj_set_hidden(img1, true);
		lv_obj_del(wifi_page);
		wifi_page = NULL;
	}
	else if (WLAN_INFO_B == QRCODE_INFO_BTN)
	{
		lv_obj_t *img3 = lv_obj_get_child_form_id(lv_scr_act(), 3);
		lv_obj_set_hidden(img3, true);

		if (qr)
			lv_obj_set_hidden(qr, true);
	}

	network_information_display();

	WLAN_INFO_B = NET_INFO_BTN;
}

static void net_info_btn_up(lv_obj_t *obj)
{
}

static void net_info_btn_create(void)
{

	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_pos(img, 199, 128);
	lv_obj_set_size(img, 252, 52);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ITEM_FOCUS_PNG);
	lv_img_set_src(img, &info);
	lv_obj_set_id(img, 2);
	lv_obj_set_hidden(img, true);

	static btn_data btn_data1 = btn_data_create(net_info_btn_down, net_info_btn_up, NULL);
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);

	lv_obj_set_pos(btn, 199, 128);
	lv_obj_set_size(btn, 252, 52);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_NETWORK_INFO));
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	btn->user_data = &btn_data1;
	btn_touch_event_listen(btn);
}

static void tuya_qrcode_destroy(void)
{
	if (qr != NULL)
	{
		ak_mem_free((void *)(lv_canvas_get_img(qr)->data));
		lv_qrcode_delete(qr);
		qr = NULL;
	}
}

static void tuya_qrcode_display(void)
{
	// extern bool is_tuya_sdk_inited(void);
	// extern char *tuya_qrcode_str_get(void);
	// if (tuya_online_status_get() == false)
	// 	return;

	if (qr == NULL)
	{
		const char *info = tuya_qrcode_shorturl_get();
		// tuya_ipc_get_qrcode(NULL, info, 32);
		// printf("tuya qrcode info :%s\n\r", info);

		if (info == NULL)
		{
			printf("QR Code get fail.....\n\r");
			return;
		}
		rom_bin_info img = rom_bin_raw_get();
		/*****  data:手动分配，不需要的时候需要手动释放 *****/
		unsigned char *data = ak_mem_alloc(MODULE_ID_APP, 300 * 300 * LV_COLOR_SIZE / 8);
		rom_bin_raw_init(img, data, 300, 300);
		// Debug("qrcode data :%lu         img offset :%lu\n\r",(unsigned long)data,img.offset);
		qr = lv_qrcode_create(lv_scr_act(), &img, 300, lv_color_hex(0x1e1e1e), lv_color_hex(0xececec));
		lv_obj_set_id(qr, 32);

		// Debug("qr data:%lu\n\r",(unsigned long)lv_canvas_get_img(qr)->data);
		/*Set data*/
		lv_obj_set_pos(qr, 500, 90);
		lv_obj_set_size(qr, 300, 300);
		lv_qrcode_update(qr, info, strlen(info));
	}
	else
	{
		lv_obj_set_hidden(qr, false);
	}
}

static void qrcode_info_btn_down(lv_obj_t *obj)
{
	if (WLAN_INFO_B == QRCODE_INFO_BTN)
	{
		return;
	}
	lv_obj_t *img3 = lv_obj_get_child_form_id(lv_scr_act(), 3);
	lv_obj_set_hidden(img3, false);

	if (WLAN_INFO_B == NET_PAIRING_BTN)
	{
		lv_obj_t *img1 = lv_obj_get_child_form_id(lv_scr_act(), 0);
		lv_obj_set_hidden(img1, true);
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 7));
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 456));
	}
	else if (WLAN_INFO_B == WLAN_BTN)
	{
		lv_obj_t *img1 = lv_obj_get_child_form_id(lv_scr_act(), 1);
		lv_obj_set_hidden(img1, true);
		lv_obj_del(wifi_page);
		wifi_page = NULL;
	}
	else if (WLAN_INFO_B == NET_INFO_BTN)
	{
		lv_obj_t *img = lv_obj_get_child_form_id(lv_scr_act(), 2);
		lv_obj_set_hidden(img, true);
		lv_obj_del(lv_obj_get_child_form_id(lv_scr_act(), 10086));
	}

	tuya_qrcode_display();
	WLAN_INFO_B = QRCODE_INFO_BTN;
}

static void qrcode_info_btn_create(void)
{

	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_pos(img, 199, 180);
	lv_obj_set_size(img, 252, 52);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ITEM_FOCUS_PNG);
	lv_img_set_src(img, &info);
	lv_obj_set_id(img, 3);
	lv_obj_set_hidden(img, true);

	static btn_data btn_data1 = btn_data_create(qrcode_info_btn_down, NULL, NULL);
	lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);

	lv_obj_set_pos(btn, 199, 180);
	lv_obj_set_size(btn, 252, 52);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_QR_CODE_NETWORK));
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	btn->user_data = &btn_data1;
	btn_touch_event_listen(btn);
}

static void wifi_img_text_display(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_NETWORK_UNFOCUS_PNG);
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_value_str(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_NETWORK_SET));
	lv_obj_set_style_local_value_color(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_align(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	net_pairing_btn_create();
	WLAN_btn_create();
	net_info_btn_create();
	qrcode_info_btn_create();

	// char eth2_mac[32] = {0};
	// char wlan0_mac[32] = {0};
	// char eth2_ip[32] = {0};
	// char wlan0_ip[32] = {0};
	// if(getmac(eth2_mac,"eth2") == 0)
	// {
	// 	Debug("eth2_mac====================>%s\n",eth2_mac);
	// }
	// if(net_util_get_ipaddr("eth2",eth2_ip) == 0)
	// {
	// 	Debug("eth2_ip====================>%s\n",eth2_ip);
	// }

	// if(getmac(wlan0_mac,"wlan0") == 0)
	// {
	// 	Debug("wlan0_mac====================>%s\n",wlan0_mac);
	// }
	// if(net_util_get_ipaddr("wlan0",wlan0_ip) == 0)
	// {
	// 	Debug("wlan0_ip====================>%s\n",wlan0_ip);
	// }
}

static lv_task_t *msg_ui_ptask = NULL;
static bool connect_falge = false;
static lv_obj_t *msg = NULL;

extern lv_obj_t *connect_wifi_cb(void);
extern void set_msg_text(lv_obj_t *msg, int state);

#define CONNECTING 0
#define CONNECT_SUCCESS 1
#define CONNECT_FAIL 2
static void close_wifi_connect(void)
{
	if (connect_falge)
	{
		connect_falge = false;
		system("killall wpa_supplicant");
		system("killall udhcpc");

		system("rm -rf /tmp/wpa_supplicant.conf &");

		char cmd[128] = {0};
		snprintf(cmd, sizeof(cmd), "wpa_supplicant -Dnl80211 -i wlan0 -c %s -B &", WPA_SUPPLICANT_PATH);
		system(cmd);
		system("udhcpc -i wlan0 -n 4 -R &");
	}
}
static void msg_task(struct _lv_task_t *task_t)
{
	static int task_timer = 0;
	if(connect_falge)
	{
			
		if (task_timer <= 12)
		{
			task_timer++;
			int connect_ret = wifi_connection_status_sucess();
			if (connect_ret == 1 && (task_timer > 3))
			{ // 连接成功

                system("\\cp -rf /tmp/wpa_supplicant.conf " WPA_SUPPLICANT_PATH " &");
				system("sync");
				user_data_get()->wifi.wifi_connect_flag = true;
				wifi_link_info_refresh();
				user_data_save();
				goto_layout(pLAYOUT(setting_wifi));
				lv_obj_t *msg1 = connect_wifi_cb();
				set_msg_text(msg1, CONNECT_SUCCESS);
				task_timer = 0;
			}
		}
		else
		{ // 连接失败
			connect_falge = false;
			task_timer = 0;
			lv_obj_t *msg1 = connect_wifi_cb();
			set_msg_text(msg1, CONNECT_FAIL);
			if (msg)
				lv_obj_del(msg);
			system("killall wpa_supplicant &");
			system("killall udhcpc &");
			system("rm -rf /tmp/wpa_supplicant.conf &");
			
			char cmd[128] = {0};
			snprintf(cmd, sizeof(cmd), "wpa_supplicant -Dnl80211 -i wlan0 -c %s -B &", WPA_SUPPLICANT_PATH);
			system(cmd);
			system("udhcpc -i wlan0 -n 4 -R &");
		}
	}
	return;
}
static void wifibtn_up(lv_obj_t *obj)
{
	Debug("===============================\n");
	if (is_wifi_page_move == true)
	{
		return;
	}

	memset(connectwifi_name, 0, strlen(connectwifi_name));
	sprintf(connectwifi_name, "%s", lv_label_get_text(lv_obj_get_child_back(obj, NULL)));
	connectwifi_index = obj->obj_id;
	if (obj->obj_data)
	{
		msg = connect_wifi_cb();
		set_msg_text(msg, 0);

		tuya_ipc_reconnect_wifi();
		wpa_cli_connect_new_wifi(connectwifi_name, NULL);

		if (msg_ui_ptask != NULL)
		{
			lv_task_del(msg_ui_ptask);
		}
		connect_falge = true;
		msg_ui_ptask = lv_task_create(msg_task,1000, LV_TASK_PRIO_HIGH, NULL);
	}
	else
	{
		goto_layout(pLAYOUT(connect_wifi));
	}
}

// 给已经搜索到的wifi创建显示按钮
static lv_obj_t *wifi_btn_create(lv_obj_t *parent, int x, int y, int w, int h, wifi_info *w_info)
{

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	if (link_info.completed)
		lv_obj_set_id(btn, y / 52 + 3);
	else
		lv_obj_set_id(btn, y / 52 - 2);

	lv_obj_set_drag_parent(btn, true);

	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);

	lv_obj_t *img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, x, y + 50);
	lv_obj_set_size(img1, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img1, &info1);

	// wifi的名称
	lv_obj_t *label = lv_label_create(btn, NULL);

	lv_label_set_text(label, w_info->ssid);
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	// 根据搜索到的wifi的信号强度 和上锁情况 选择对应的图片显示(-100, -88] (-88, -77] (-77, -55] rssi>=-55)
	lv_obj_t *img = lv_img_create(btn, NULL);

	if (w_info->signal_level > -55 && w_info->signal_level < 0)
	{ // 信号满格

		if (w_info->free)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_PNG);
			lv_img_set_src(img, &info);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_C_PNG);
			lv_img_set_src(img, &info);
		}
	}
	else if (w_info->signal_level <= -55 && w_info->signal_level > -77)
	{ // 2格信号

		if (w_info->free)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_2_PNG);
			lv_img_set_src(img, &info);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_2_C_PNG);
			lv_img_set_src(img, &info);
		}
	}
	else if (w_info->signal_level <= -77 && w_info->signal_level)
	{ // 一格信号

		if (w_info->free)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_1_PNG);
			lv_img_set_src(img, &info);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_1_C_PNG);
			lv_img_set_src(img, &info);
		}
	}
	else if (w_info->signal_level <= -88)
	{
		if (w_info->free)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_0_PNG);
			lv_img_set_src(img, &info);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_0_C_PNG);
			lv_img_set_src(img, &info);
		}
	}

	lv_obj_align(img, btn, LV_ALIGN_IN_RIGHT_MID, -10, 0);

	// 单独的回调函数
	static btn_data btn_data = btn_data_up_create(wifibtn_up);
	btn->obj_data = w_info->free;
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	return btn;
}

// 添加wifi按钮的回调
static void add_wifi_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(add_wifi));
}

static lv_obj_t *wifipage_addwifi_btn_create(lv_obj_t *parent, int x, int y, int w, int h)
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);

	// lv_obj_set_parent_event(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);

	lv_btn_set_layout(btn, LV_LAYOUT_OFF);

	lv_obj_t *img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, x, y + 50);
	lv_obj_set_size(img1, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img1, &info1);

	// wifi的名称
	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(label, text_str(STR_ADD_MANUALLY));
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	// wifi的图片 根据信号强度选择对应的图片
	rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ADD_WIFI_PNG);
	lv_obj_t *img = lv_img_create(btn, NULL);

	lv_img_set_src(img, &info);
	lv_obj_align(img, btn, LV_ALIGN_IN_RIGHT_MID, -10, 0);

	static btn_data btn_data = btn_data_up_create(add_wifi_btn_up);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	return btn;
}

static lv_obj_t *connectwifi_btn_create(lv_obj_t *parent, int x, int y, int w, int h)
{

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);

	lv_obj_t *img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, x, y + 50);
	lv_obj_set_size(img1, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img1, &info1);

	// wifi的名称

	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_text(label, link_info.wlan_ssid); // wifi名称

	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	// 根据搜索到的wifi的信号强度 和上锁情况 选择对应的图片显示(-100, -88] (-88, -77] (-77, -55] rssi>=-55)

	lv_obj_t *img = lv_img_create(btn, NULL);

	if (link_info.level > -55 && link_info.level < 0)
	{ // 信号满格

		if (strcmp(link_info.key_mgmt, "[ESS]") == 0)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_PNG);
			lv_img_set_src(img, &info);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_C_PNG);
			lv_img_set_src(img, &info);
		}
	}
	else if (link_info.level <= -55 && link_info.level > -77)
	{ // 2格信号

		if (strcmp(link_info.key_mgmt, "[ESS]") == 0)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_2_PNG);
			lv_img_set_src(img, &info);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_2_C_PNG);
			lv_img_set_src(img, &info);
		}
	}
	else if (link_info.level <= -77 && link_info.level > -88)
	{ // 一格信号

		if (strcmp(link_info.key_mgmt, "[ESS]") == 0)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_1_PNG);
			lv_img_set_src(img, &info);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_1_C_PNG);
			lv_img_set_src(img, &info);
		}
	}
	else if (link_info.level <= -88)
	{
		if (strcmp(link_info.key_mgmt, "[ESS]") == 0)
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_0_PNG);
			lv_img_set_src(img, &info);
		}
		else
		{
			rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_WIFI_0_C_PNG);
			lv_img_set_src(img, &info);
		}
	}

	lv_obj_align(img, btn, LV_ALIGN_IN_RIGHT_MID, -10, 0);

	// 单独的回调函数
	static btn_data btn_data = btn_data_up_create(network_btn_up);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	return btn;
}

static void findwifi_wifibtn_create(lv_obj_t *parent)
{
	Debug("=======================================================>>>\n");
	int8_t sum = 0; // 划线的数量
	connectwifi_index = 0;

	// 打开后
	bool a = true;
	wpa_cli_scan_wifi(&a);

	wifi_link_info_refresh();

	if (link_info.completed)
	{
		user_data_get()->wifi.wifi_connect_flag = true;

		// wifi已经连接 创建按钮显示当前wifi的按钮
		connectwifi_btn_create(parent, 0, 0, 422, 52);

		// 创建添加wifi的按钮
		wifipage_addwifi_btn_create(parent, 0, 52, 422, 52); // 修改回调 点击按钮之后显示wifi名称 ip 密码 和 加密方式
		sum = 3;
	}
	else
	{
		wifipage_addwifi_btn_create(parent, 0, 0, 422, 52);
		sum = 2;
	}

#if defined(RFV8919_WIFI)
	extern bool is_Hisilicon0_paltform(void);
	if (is_Hisilicon0_paltform())
	{
		return;
	}
#endif

	// wpa_cli_scan_wifi(&a);
	// wpa_cli_wlan_status(&a);

	connected_wifi_max = get_max_wifi_list_index(); // 获取到搜索到的wifi的总数
	printf("@@@@@@@@@@@@@@@@@@@@@@@%d\n", connected_wifi_max);

	static wifi_info info;
	for (int i = 0; i < connected_wifi_max; i++)
	{
		memset(&info, 0, sizeof(wifi_info));
		get_scanned_wifi_info(i, &info); // 获取指定结点的搜索的wifi结点

		if (strcmp(info.ssid, link_info.wlan_ssid) == 0)
		{
			sum--;
			continue;
		}
		if (info.hidden == true)
		{
			sum--;
			continue;
		}
		wifi_btn_create(parent, 0, ((i + sum) * 52 - 20), 422, 52, &info);
	}
}
static void wifi_page_touch_anything(lv_obj_t *obj, lv_event_t event)
{
	Debug("event:%d \n", event);
	if (LV_EVENT_DRAG_BEGIN == event)
	{
		is_wifi_page_move = true;
	}
	else if (LV_EVENT_DRAG_END == event)
	{
		is_wifi_page_move = false;
	}
}

// 创建整个关于wifi的页面 可以上下滑动
static void wifi_page_create(lv_obj_t *parent)
{
	// 创建页面
	wifi_page = lv_page_create(parent, NULL); // 在当前活跃的屏幕上创建页面

	static btn_data page_data = btn_data_anything_create(wifi_page_touch_anything);
	wifi_page->user_data = &page_data;
	btn_touch_event_listen(wifi_page);

	lv_obj_t *cont = lv_obj_get_child_back(wifi_page, NULL);
	if (cont != NULL)
	{
		lv_cont_set_fit4(cont, LV_FIT_PARENT, LV_FIT_PARENT, LV_FIT_TIGHT, LV_FIT_MAX);
		lv_cont_set_layout(cont, LV_LAYOUT_OFF);
	}

	set_location(wifi_page, 452, 0, 466, 600);
	lv_page_set_edge_flash(wifi_page, 1);
	lv_page_set_scrollbar_mode(wifi_page, LV_SCROLLBAR_MODE_DRAG);
	lv_obj_set_style_local_bg_color(wifi_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x0, 0x0, 0x00));
	lv_obj_set_style_local_bg_opa(wifi_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_OPA_10);
	lv_obj_set_style_local_bg_color(wifi_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, lv_color_make(0x0, 0x20, 0x00));
	lv_obj_set_style_local_bg_opa(wifi_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	// if(user_data_get()->wifi.wifi_open_flag){//如果进来wifi是打开的 那么需要搜索和创建一些按钮

	if (user_data_get()->pairing_mode == WLAN_NET)
		findwifi_wifibtn_create(wifi_page); // 搜索wifi并且创建按钮

	// lv_switch_on(sw, LV_ANIM_OFF);

	//}//else
	// lv_switch_off(sw, LV_ANIM_OFF);//如果wifi是关闭状态 无需搜索
}

static void IP_address_btn_create(lv_obj_t *parent)
{

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, 0, 0, 422, 52);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);

	lv_obj_t *img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, 0, 0 + 50);
	lv_obj_set_size(img1, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img1, &info1);

	// wifi的名称

	static char buf[32] = {0};
	if (user_data_get()->pairing_mode == WLAN_NET)
		sprintf(buf, "%s : %s", text_str(STR_IP_ADDRESS), link_info.ip);
	else if (user_data_get()->pairing_mode == WIRED_NET)
	{
		char ip[32] = {0};
		net_util_get_ipaddr("eth2", ip);
		sprintf(buf, "%s : %s", text_str(STR_IP_ADDRESS), ip);
	}

	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_text(label, buf); // wifi名称

	if (user_data_get()->language.index == HEBREW)
	{
		lv_obj_set_base_dir(label, LV_BIDI_DIR_LTR);
	}
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 40, 0);

	static btn_data btn_data = btn_data_up_create(NULL);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);
}

static void MAC_btn_create(lv_obj_t *parent)
{

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, 0, 52, 422, 52);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);

	lv_obj_t *img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, 0, 0 + 50 + 52);
	lv_obj_set_size(img1, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img1, &info1);

	// wifi的名称
	static char buf[32] = {0};

	if (user_data_get()->pairing_mode == WLAN_NET)
		sprintf(buf, "%s : %s", text_str(STR_MAC), link_info.mac);
	else if (user_data_get()->pairing_mode == WIRED_NET)
	{
		char mac[32] = {0};
		getmac(mac, "eth2");
		sprintf(buf, "%s : %s", text_str(STR_MAC), mac);
	}

	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_text(label, buf); // wifi名称

	if (user_data_get()->language.index == HEBREW)
	{
		lv_obj_set_base_dir(label, LV_BIDI_DIR_LTR);
	}
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 40, 0);

	static btn_data btn_data = btn_data_up_create(NULL);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);
}

static void network_information_display(void)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL); // 在当前活跃的屏幕上创建页面

	lv_obj_set_pos(cont, 472, 76);
	lv_obj_set_size(cont, 422, 120);

	lv_obj_set_id(cont, 10086);
	IP_address_btn_create(cont);
	MAC_btn_create(cont);
}

static void wifi_setting_back_btn_up(lv_obj_t *obj)
{

	if (net_pairing_mode != user_data_get()->pairing_mode)
	{
		network_window_create(text_str(STR_RESTART_SYSTEM));
	}
	else
	{
		goto_layout(pLAYOUT(setting));
	}
}

static void wifi_setting_display(void)
{
	wifi_img_text_display();
	pairing_switch_create();
	alloction_mode_create();
	// network_information_display();

	home_back_btn_create(wifi_setting_back_btn_up, NULL);
}

static void window_yes_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}

	btn_data *pdata = (btn_data *)obj->user_data;
	char *str = (char *)pdata->user_data;

	if (str == text_str(STR_DISCONNECT_WIFI))
	{
		turn_off_wlan_break();
		user_data_get()->wifi.wifi_connect_flag = false;
	}
	else if (str == text_str(STR_RESTART_SYSTEM))
	{
		user_data_get()->pairing_mode = net_pairing_mode;
		user_data_save();
		backlight_open(false, false, 0);
		extern int lcd_reset_pin_higt(void);
		lcd_reset_pin_higt(); /* 防止上电复位失败 */
		ak_sleep_ms(1000 * 2);
		system("reboot");
	}

	goto_layout(pLAYOUT(setting_wifi));
}

static void window_no_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
}

static void network_btn_up(lv_obj_t *obj)
{
	network_window_create(text_str(STR_DISCONNECT_WIFI));
}
static void network_window_create(char *str)
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

	lv_obj_t *window_head_label = lv_label_create(window_img, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(window_head_label, str);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_align(window_head_label, window_img, LV_ALIGN_CENTER, 0, -30);

	lv_obj_t *window_ok_btn = lv_btn_create(window_img, NULL);
	lv_obj_set_pos(window_ok_btn, 0, 360);
	lv_obj_set_size(window_ok_btn, 320, 77);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static btn_data btn_data1 = btn_data_create(NULL, window_yes_btn_up, NULL);

	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_YES));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_YES));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	btn_data1.user_data = str;
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	lv_obj_t *window_cancel_btn = lv_btn_create(window_img, window_ok_btn);
	lv_obj_set_x(window_cancel_btn, 324);
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_NO));
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_NO));
	static btn_data btn_data2 = btn_data_create(NULL, window_no_btn_up, NULL);
	window_cancel_btn->user_data = &btn_data2;
	btn_touch_event_listen(window_cancel_btn);
}

void find_link_wifi(void)
{
	if (!wifi_usb_module_enable())
		return;

	bool a = true;
	wpa_cli_scan_wifi(&a);
	wifi_link_info_refresh();

	printf("#################:%s\n", link_info.wlan_ssid);
	printf("@@@@@@@@@@@@@@@@@@:%d\n", link_info.completed);
}

static void LAYOUT_ENETER_FUNC(setting_wifi)
{
	WLAN_INFO_B = NET_PAIRING_BTN;
	setting_bg_display();
	is_wifi_page_move = false;
	wifi_control = true;
	wifi_link_info_refresh();
	// wpa_cli_scan_wifi(&wifi_control);
	// wpa_cli_wlan_status(&wifi_control);
	wifi_setting_display();
}

static void LAYOUT_QUIT_FUNC(setting_wifi)
{
	if (user_data_get()->allocation_mode != net_alloction_mode)
	{
		user_data_get()->allocation_mode = net_alloction_mode;

		if (user_data_get()->allocation_mode == UDHCPC_ALLOC)
		{
			system("udhcpc -i eth2 &");
			Debug("udhcpc -i eth2 &");
		}
		else if (user_data_get()->allocation_mode == STATIC_ALLOC)
		{
			system("ifconfig eth2 192.168.188.1");
			Debug("ifconfig eth2 192.168.188.1");
		}
	}

	tuya_qrcode_destroy();
	user_data_save();
	close_wifi_connect();

	if (msg_ui_ptask != NULL) // 计时任务未退出
	{
		lv_task_del(msg_ui_ptask); // 退出计时任务
		msg_ui_ptask = NULL;	   // 置空指针
	}
}

CREATE_LAYOUT(setting_wifi);
