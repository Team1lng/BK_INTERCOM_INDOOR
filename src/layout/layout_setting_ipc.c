#include "layout_define.h"
#include "leo_api.h"
#include "sat_ipcamera.h"
#include "user_data.h"

#ifdef DHCP_IPCAMERA

#define BTN_PRESSED_COLOR lv_color_make(0x80, 0x0, 0x10)

static int ipc_online_total = 0;
static lv_obj_t *ipc_page = NULL;
static lv_task_t *ipcamera_search_task = NULL;
static unsigned int curr_select_ipc_index = 0;

static int ipc_info_input_progress = 0;

typedef enum
{
	SCAN_PAGE,
	MANUALLY_PAGE,
	KEYBOARD_PAGE,
	PREVIEW_PAGE,
	TOTAL_PAGE
} ipc_setting_page;

typedef enum
{
	IPC_UNKNOWN = 0,
	IPCAMERA1,
	IPCAMERA2
} ipc_dev_id;

struct
{
	lv_obj_t *layout_page[TOTAL_PAGE];
	ipc_setting_page ipc_page;
} ipc_setting_layout;

enum
{
	IPC_ONLINE_SEARCH,
	IPC_MANUALLY_ADD,
	IPC_SEARCH_UPDATE,
	IPC_URL_SEARCH,
	IPC_URL_UPDATE,
	TOTAL_PROCESS
} ipc_setting_process;

struct
{
	bool model;
	char ip[16];
	char name[16];
	char pwd[16];
	char url[128];
} ipcamera_temp_info;

extern void register_rtsp_callback(void (*backfunc)(bool));
// static void camera_temp_info_refresh(void);
static void ipc_msg_windows_start(char *str);
void ipcamera_page_switch(int page);
static lv_obj_t *ipc_preview_create(const char *str, ipc_dev_id DEV_ID);
static void onvif_video_mode_close(void);

static lv_task_t *manually_link_load = NULL;
static void manually_link_load_task(struct _lv_task_t *task_t)
{
	bool arg = (bool)task_t->user_data;
	Debug("manually_link retun %s\n", arg ? "true" : "flase");
	if (arg == false)
	{
		ipc_msg_windows_start(text_str(STR_LINK_DEVICE_FAILED));
		onvif_video_mode_close();
		Debug("鉴权失败 IPaddr:%s \n", ipcamera_temp_info.ip);
	}
	else
	{
		fb_video_mode_enable(true); // 打开
		ipc_preview_create(ipcamera_temp_info.ip, IPC_UNKNOWN);
	}

	if (msg_loading_t)
		lv_obj_del_reload(&(msg_loading_t)); /* !!! 一定要删除loading 弹窗的父对象 */

	if (manually_link_load)
	{
		lv_task_del(manually_link_load);
		manually_link_load = NULL;
	}
}

static void manually_link_backfunc(bool arg)
{
	if (manually_link_load == NULL)
	{
		manually_link_load = lv_task_create(manually_link_load_task, 2500, LV_TASK_PRIO_HIGH, (void *)arg);
	}

	register_rtsp_callback(NULL);
}
typedef enum camera_module_list
{
	// SWITCH_MODULE,
	MODEL_MODULE,
	IP_MODULE,
	ACCOUNT_MODULE,
	PASSWORD_MODULE,
	TOTAL_MODULE
} camera_module_list;

#define CAMERA_MODULE_COORDINATE_INIT { \
	{199, 75, 700, 52},                 \
	{199, 127, 700, 52},                \
	{199, 179, 700, 52},                \
	{199, 231, 700, 52},                \
	{199, 293, 700, 52},                \
};

static void ipc_page_create(lv_obj_t *parent);
static void ipc_page_destroy(void);

void ipcamera_temp_info_init(void)
{
	ipc_setting_layout.ipc_page = SCAN_PAGE;
	ipc_setting_layout.layout_page[SCAN_PAGE] = NULL;
	ipc_setting_layout.layout_page[MANUALLY_PAGE] = NULL;
	ipc_setting_layout.layout_page[KEYBOARD_PAGE] = NULL;
	ipc_setting_layout.layout_page[PREVIEW_PAGE] = NULL;
	memset(&ipcamera_temp_info, 0, sizeof(ipcamera_temp_info));
}

static void scan_page_display(void);
static void camera_manually_display(void);
void ipcamera_page_switch(int page)
{

	// Debug("curr page addr:%p   next page :%d\n",ipc_setting_layout.layout_page[ipc_setting_layout.ipc_page],page);
	if (ipc_setting_layout.layout_page[ipc_setting_layout.ipc_page])
	{
		// Debug("curr page :%d   next page :%d\n",ipc_setting_layout.ipc_page,page);
		if (ipc_setting_layout.ipc_page != page)
		{
			if (ipc_setting_layout.ipc_page == PREVIEW_PAGE)
			{
				lv_area_t area[] = {{0, 0, 1024, 600}};
				gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));
			}

			lv_obj_del_reload(&ipc_setting_layout.layout_page[ipc_setting_layout.ipc_page]);
			ipc_setting_layout.ipc_page = page;

			switch (page)
			{
			case SCAN_PAGE:
				if (ipc_setting_process == IPC_MANUALLY_ADD)
				{
					ipc_setting_process = IPC_ONLINE_SEARCH;
				}
				scan_page_display();
				break;
			case MANUALLY_PAGE:
				if (ipc_setting_process != IPC_MANUALLY_ADD)
				{
					ipc_setting_process = IPC_MANUALLY_ADD;
				}
				camera_manually_display();
				break;
			case KEYBOARD_PAGE:
				break;
			case PREVIEW_PAGE:
				break;
			}

			// Debug("enter page :%d   curr page (%d)addr:%p\n",page,ipc_setting_layout.ipc_page,ipc_setting_layout.layout_page[ipc_setting_layout.ipc_page]);
		}
	}
}

static void onvif_video_mode_open(char *url, bool fb_video_enable)
{
	if (url == NULL)
	{
		return;
	}
	Debug("==============monitor_video_mode_open url:%s====>>>>\n\n\n", url);
	fb_video_mode_enable(fb_video_enable); // 打开
	extern bool rtsp_stream_open(char *url);

	// extern void video_raw_clear(void);
	// video_raw_clear();
	video_decode_queue_reset();
	video_decode_open(0, DECODE_WIDTH, DECODE_HIGHT); // 640, 360);
	rtsp_stream_open(url);
}

static void onvif_video_mode_close(void)
{
	extern bool rtsp_stream_open(char *url);
	extern bool rtsp_stream_close(void);
	fb_video_mode_enable(false);
	rtsp_stream_close();
	video_decode_close();
	// gui_draw_area_set_2();
}

static void confirmation_box_create(void (*window_yse_btn_up)(lv_obj_t *), void (*window_no_btn_up)(lv_obj_t *), char *str, ipc_dev_id id)
{
	lv_area_t area[] = {
		{0, 0, 1024, 80},
		{0, 528, 1024, 600},
		{188, 79, 836, 520},
	};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	lv_obj_set_id(window_cont, 888);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_4_PNG);
	lv_obj_t *window_img = lv_img_create(window_cont, NULL);
	// lv_obj_set_size(window_img, 526, 358);
	lv_obj_set_size(window_img, 648, 441);
	lv_img_set_src(window_img, &info);
	lv_obj_align(window_img, window_cont, LV_ALIGN_CENTER, 0, 0);

	lv_obj_t *window_head_label = lv_label_create(window_img, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(window_head_label, str);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_align(window_head_label, window_img, LV_ALIGN_CENTER, 0, -30);

	lv_obj_t *window_ok_btn = lv_btn_create(window_img, NULL);
	lv_obj_set_pos(window_ok_btn, 0, 296);
	lv_obj_set_size(window_ok_btn, 261, 62);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static btn_data btn_data1 = {0};
	btn_data1.obj_tone = true;
	btn_data1.OPS_UP = window_yse_btn_up;

	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_YES));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_YES));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	window_ok_btn->user_data = &btn_data1;
	window_ok_btn->obj_data = id;
	btn_data1.user_data = window_cont;
	btn_touch_event_listen(window_ok_btn);

	lv_obj_t *window_cancel_btn = lv_btn_create(window_img, window_ok_btn);
	lv_obj_set_x(window_cancel_btn, 265);
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_NO));
	lv_obj_set_style_local_value_str(window_cancel_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_NO));
	static btn_data btn_data2;
	btn_data2.obj_tone = true;
	btn_data2.OPS_UP = window_no_btn_up;
	window_cancel_btn->user_data = &btn_data2;
	btn_data2.user_data = window_cont;
	btn_touch_event_listen(window_cancel_btn);
}

static void logout_confirmation_up(lv_obj_t *obj)
{
	lv_obj_t *con_box = ((btn_data *)obj->user_data)->user_data;
	lv_obj_del_reload(&con_box);

	Debug("========================================%d\n", obj->obj_data);
	if (obj->obj_data == IPCAMERA1)
		memset(&(user_data_get()->camera1.ip[0]), 0, sizeof(user_data_get()->camera1.ip) + sizeof(user_data_get()->camera1.account) + sizeof(user_data_get()->camera1.pwd) + sizeof(user_data_get()->camera1.url));
	else if (obj->obj_data == IPCAMERA2)
		memset(&(user_data_get()->camera2.ip[0]), 0, sizeof(user_data_get()->camera2.ip) + sizeof(user_data_get()->camera2.account) + sizeof(user_data_get()->camera2.pwd) + sizeof(user_data_get()->camera2.url));

	if (ipc_setting_layout.layout_page[PREVIEW_PAGE])
	{
		onvif_video_mode_close();
		ipcamera_page_switch(SCAN_PAGE);
		user_data_save();
	}
}
static void logout_cancel_up(lv_obj_t *obj)
{
	Debug("========================================\n");
	lv_obj_t *con_box = ((btn_data *)obj->user_data)->user_data;
	lv_obj_del_reload(&con_box);
}
static void logout_confirmation_box(ipc_dev_id id)
{
	confirmation_box_create(logout_confirmation_up, logout_cancel_up, text_str(STR_CONFIRM_CANCELLATION), id);
}

static void replace_confirmation_up(lv_obj_t *obj)
{
	lv_obj_t *con_box = ((btn_data *)obj->user_data)->user_data;
	lv_obj_del_reload(&con_box);
	if (obj->obj_data == IPCAMERA1)
		memcpy(&(user_data_get()->camera1.model), &ipcamera_temp_info, sizeof(ipcamera_temp_info));
	else if (obj->obj_data == IPCAMERA2)
		memcpy(&(user_data_get()->camera2.model), &ipcamera_temp_info, sizeof(ipcamera_temp_info));

	if (ipc_setting_layout.layout_page[PREVIEW_PAGE])
	{
		onvif_video_mode_close();
		ipcamera_page_switch(SCAN_PAGE);
		user_data_save();
	}
}
static void replace_cancel_up(lv_obj_t *obj)
{
	lv_obj_t *con_box = ((btn_data *)obj->user_data)->user_data;
	lv_obj_del_reload(&con_box);
}
static void replace_confirmation_box(ipc_dev_id id)
{
	confirmation_box_create(replace_confirmation_up, replace_cancel_up, text_str(STR_REPLACE_DEV), id);
}

static void ipc_msg_windows_start(char *str)
{
	lv_obj_t *msg = lv_msgbox_create(lv_scr_act(), NULL);

	lv_obj_set_pos(msg, 240, 145);
	lv_obj_set_size(msg, 550, 300);

	lv_obj_set_style_local_bg_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x1A1A1A));
	lv_obj_set_style_local_bg_opa(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_obj_set_style_local_border_width(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
	lv_obj_set_style_local_text_font(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, FONT_SIZE_L(31));
	lv_obj_set_style_local_text_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	lv_msgbox_set_text(msg, str);
	lv_msgbox_start_auto_close(msg, 2500);
}

extern lv_obj_t *list_window_btn_create(lv_obj_t *parent, int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src1, const void *img_src2, char *string);
static void register_camera1_btn_up(lv_obj_t *obj)
{
	Debug("==================>>>%p\n", ipc_setting_layout.layout_page[PREVIEW_PAGE]);
	if (ipc_setting_layout.layout_page[PREVIEW_PAGE])
	{
		if (memcmp(user_data_get()->camera2.url, ipcamera_temp_info.url, sizeof(ipcamera_temp_info.url)) == 0 ||
			memcmp(user_data_get()->camera1.url, ipcamera_temp_info.url, sizeof(ipcamera_temp_info.url)) == 0)
		{
			ipc_msg_windows_start(text_str(STR_REG_SAME_DEVICE));
		}
		else
		{
			if (user_data_get()->camera1.url[0] != 'r')
			{
				onvif_video_mode_close();
				memcpy(&user_data_get()->camera1.model, &ipcamera_temp_info, sizeof(ipcamera_temp_info));
				ipcamera_page_switch(SCAN_PAGE);
				// Debug("camera1 url:%s\n    temp url:%s\n",user_data_get()->camera1.url,ipcamera_temp_info.url);
				user_data_save();
			}
			else
			{
				/* 弹出提示是否替换 */
				replace_confirmation_box(IPCAMERA1);
				lv_obj_t *windows = (((btn_data *)obj->user_data)->user_data);
				if (windows->user_data)
					lv_obj_del_reload((lv_obj_t **)(&(windows->user_data)));
			}
		}
	}
}
static void register_camera1_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, register_camera1_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	btn_data.user_data = parent;
	list_window_btn_create(parent, 43, 25, 217, 48, &btn_data, &info1, &info, text_str(STR_CAMERA1));
}
static void register_camera2_btn_up(lv_obj_t *obj)
{
	Debug("==================>>>%p\n", ipc_setting_layout.layout_page[PREVIEW_PAGE]);
	if (ipc_setting_layout.layout_page[PREVIEW_PAGE])
	{
		if (memcmp(user_data_get()->camera2.url, ipcamera_temp_info.url, sizeof(ipcamera_temp_info.url)) == 0 ||
			memcmp(user_data_get()->camera1.url, ipcamera_temp_info.url, sizeof(ipcamera_temp_info.url)) == 0)
		{
			ipc_msg_windows_start(text_str(STR_REG_SAME_DEVICE));
		}
		else
		{
			if (user_data_get()->camera2.url[0] != 'r')
			{
				onvif_video_mode_close();
				memcpy(&user_data_get()->camera2.model, &ipcamera_temp_info, sizeof(ipcamera_temp_info));
				ipcamera_page_switch(SCAN_PAGE);
				user_data_save();
			}
			else
			{
				/* 弹出提示是否替换 */

				replace_confirmation_box(IPCAMERA1);
				lv_obj_t *windows = (((btn_data *)obj->user_data)->user_data);
				if (windows->user_data)
					lv_obj_del_reload((lv_obj_t **)(&(windows->user_data)));
			}
		}
	}
}
static void register_camera2_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, register_camera2_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	btn_data.user_data = parent;
	list_window_btn_create(parent, 43, 97, 217, 48, &btn_data, &info1, &info, text_str(STR_CAMERA2));
}

static void register_close_btn_up(lv_obj_t *obj)
{
	Debug("==================>>>%p\n", ipc_setting_layout.layout_page[PREVIEW_PAGE]);
	if (ipc_setting_layout.layout_page[PREVIEW_PAGE])
	{
		onvif_video_mode_close();
		ipcamera_page_switch(SCAN_PAGE);
	}
}
static void register_close_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, register_close_btn_up, NULL);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_FOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_UNFOCUS_PNG);
	list_window_btn_create(parent, 43, 169, 217, 48, &btn_data, &info1, &info, text_str(STR_CLOSE));
	btn_data.user_data = parent;
	// Debug("register_close_btn_create %p:%p:%p\n",parent,&btn_data,btn_data.user_data);
}
static void register_ipc_window_create(void)
{
	lv_area_t area[] = {
		{0, 0, 1024, 80},
		{0, 528, 1024, 600},
		{188, 79, 836, 520},
	};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

	lv_obj_t *window = lv_cont_create(ipc_setting_layout.layout_page[PREVIEW_PAGE], NULL);
	lv_obj_set_style_local_bg_opa(window, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window, 0, 0);
	lv_obj_set_size(window, 1024, 600);
	lv_obj_set_id(window, 8888);

	lv_obj_t *window_cont = lv_cont_create(window, NULL);
	lv_obj_set_style_local_bg_color(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00131D));
	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
	lv_obj_set_style_local_radius(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);
	set_location(window_cont, 380, 163, 304, 242);
	window_cont->user_data = window;

	register_camera1_btn_create(window_cont);
	register_camera2_btn_create(window_cont);
	register_close_btn_create(window_cont);
}

static void ipc_register_btn_up(lv_obj_t *obj)
{
	register_ipc_window_create();
}

static void ipc_logout_btn_up(lv_obj_t *obj)
{
	logout_confirmation_box(obj->obj_data);
}
/* 预览返回 */
static void ipc_preview_backbtn_up(lv_obj_t *obj)
{
	Debug("==================>>>%p\n", ipc_setting_layout.layout_page[PREVIEW_PAGE]);
	if (ipc_setting_layout.layout_page[PREVIEW_PAGE])
	{
		onvif_video_mode_close();
		if (ipc_setting_process == IPC_MANUALLY_ADD)
		{
			ipcamera_page_switch(MANUALLY_PAGE);
		}
		else
		{
			ipcamera_page_switch(SCAN_PAGE);
		}
	}
}

static lv_obj_t *ipc_preview_create(const char *str, ipc_dev_id DEV_ID)
{

	lv_area_t area[] = {
		{0, 0, 1024, 80},
		{0, 528, 1024, 600},
	};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

	ipc_setting_layout.layout_page[PREVIEW_PAGE] = lv_cont_create(lv_scr_act(), NULL);
	set_location(ipc_setting_layout.layout_page[PREVIEW_PAGE], 0, 0, 1024, 600);
	lv_obj_set_style_local_bg_opa(ipc_setting_layout.layout_page[PREVIEW_PAGE], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(ipc_setting_layout.layout_page[PREVIEW_PAGE], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

	lv_obj_t *head = lv_obj_create(ipc_setting_layout.layout_page[PREVIEW_PAGE], NULL);
	set_location(head, 0, 0, 1024, 80);
	lv_obj_set_style_local_bg_color(head, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(head, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);

	lv_obj_set_style_local_value_align(head, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(head, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE_L(36));
	lv_obj_set_style_local_value_str(head, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, str);

	{
		lv_obj_t *back_btn = lv_btn_create(ipc_setting_layout.layout_page[PREVIEW_PAGE], NULL);

		lv_obj_set_pos(back_btn, 30, 0);
		lv_obj_set_size(back_btn, 80, 80);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);

		lv_obj_set_style_local_bg_opa(back_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		lv_obj_set_style_local_pattern_image(back_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, (void *)&info1);

		static btn_data btn_data0 = btn_data_up_create(ipc_preview_backbtn_up);
		back_btn->user_data = &btn_data0;
		btn_touch_event_listen(back_btn);
	}

	lv_obj_t *register_btn = lv_obj_create(ipc_setting_layout.layout_page[PREVIEW_PAGE], NULL);
	set_location(register_btn, 0, 528, 1024, 72);

	lv_obj_set_style_local_bg_color(register_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x002538));
	lv_obj_set_style_local_bg_opa(register_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);

	lv_obj_set_style_local_value_align(register_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(register_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE_L(31));
	lv_obj_set_style_local_value_str(register_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, DEV_ID ? text_str(STR_CANCELLATION) : text_str(STR_REGISTER));

	static btn_data btn_data1;
	btn_data1.obj_tone = true;
	btn_data1.OPS_UP = DEV_ID ? ipc_logout_btn_up : ipc_register_btn_up;
	btn_data1.user_data = ipc_setting_layout.layout_page[PREVIEW_PAGE];
	register_btn->user_data = &btn_data1;
	register_btn->obj_data = DEV_ID;
	btn_touch_event_listen(register_btn);

	ipcamera_page_switch(PREVIEW_PAGE);
	return NULL;
}

extern void keyboard_layout(lv_obj_t *obj);
extern const char **kb_map[];
extern const lv_btnmatrix_ctrl_t *kb_ctrl[];
#if 1
// 键盘的回调函数
static void lv_keyboard_event1_cb(lv_obj_t *kb)
{
	LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

	//  if(event != LV_EVENT_VALUE_CHANGED) return;

	lv_keyboard_ext_t *ext = lv_obj_get_ext_attr(kb);
	uint16_t btn_id = lv_btnmatrix_get_active_btn(kb);
	if (btn_id == LV_BTNMATRIX_BTN_NONE)
		return;
	if (lv_btnmatrix_get_btn_ctrl(kb, btn_id, LV_BTNMATRIX_CTRL_HIDDEN | LV_BTNMATRIX_CTRL_DISABLED))
		return;
	//  if(lv_btnmatrix_get_btn_ctrl(kb, btn_id, LV_BTNMATRIX_CTRL_NO_REPEAT) && event == LV_EVENT_LONG_PRESSED_REPEAT) return;

	const char *txt = lv_btnmatrix_get_active_btn_text(kb); // 获得活跃的按钮的文本
	if (txt == NULL)
		return;

	/*Do the corresponding action according to the text of the button*/
	if (strcmp(txt, "abc") == 0)
	{
		ext->mode = LV_KEYBOARD_MODE_TEXT_LOWER;
		lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_LOWER]);
		lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_LOWER]);

		lv_keyboard_img_set(kb, 10, NULL);

		static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
		lv_keyboard_img_set(kb, 11, &info11);

		static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
		lv_keyboard_img_set(kb, 22, &info22);

		return;
	}
#if LV_USE_ARABIC_PERSIAN_CHARS == 1
	else if (strcmp(txt, "أب") == 0)
	{
		ext->mode = LV_KEYBOARD_MODE_TEXT_ARABIC;
		lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_ARABIC]);
		lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_ARABIC]);
		return;
	}
#endif
	else if (strcmp(txt, "ABC") == 0)
	{
		ext->mode = LV_KEYBOARD_MODE_TEXT_UPPER;
		lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_TEXT_UPPER]);
		lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_TEXT_UPPER]);

		// 对图片进行调整、
		lv_keyboard_img_set(kb, 10, NULL);

		static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
		lv_keyboard_img_set(kb, 11, &info11);

		static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
		lv_keyboard_img_set(kb, 22, &info22);

		return;
	}
	else if (strcmp(txt, "1#") == 0)
	{
		ext->mode = LV_KEYBOARD_MODE_SPECIAL;
		lv_btnmatrix_set_map(kb, kb_map[LV_KEYBOARD_MODE_SPECIAL]);
		lv_btnmatrix_set_ctrl_map(kb, kb_ctrl[LV_KEYBOARD_MODE_SPECIAL]);

		static rom_bin_info info10 = rom_bin_info_get(ROM_RES_KB_DELETL_PNG);
		lv_keyboard_img_set(kb, 10, &info10);

		lv_keyboard_img_set(kb, 11, NULL);

		lv_keyboard_img_set(kb, 22, NULL);
		return;
	}

	/****************************************设置模式************************************************************************/
	/*Add the characters to the text area if set*/

	if (ext->ta == NULL)
		return;

	if (ext->btnm.pattern_p[btn_id])
	{

		if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_CANCEL_PNG)
		{ /*LV_SYMBOL_CLOSE*/

			lv_textarea_set_text(ext->ta, ""); /*De-assign the text area to hide it cursor if needed*/
			return;
		}
		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_APPLY_PNG)
		{ /*LV_SYMBOL_OK*/

			if (ipc_setting_process == IPC_MANUALLY_ADD)
			{
				switch (kb->obj_data)
				{
				case IP_MODULE:
					memset(ipcamera_temp_info.ip, 0, sizeof(ipcamera_temp_info.ip));
					sprintf(ipcamera_temp_info.ip, "%s", lv_textarea_get_text(ext->ta));
					break;

				case ACCOUNT_MODULE:
					memset(ipcamera_temp_info.name, 0, sizeof(ipcamera_temp_info.name));
					sprintf(ipcamera_temp_info.name, "%s", lv_textarea_get_text(ext->ta));
					break;

				case PASSWORD_MODULE:
					memset(ipcamera_temp_info.pwd, 0, sizeof(ipcamera_temp_info.pwd));
					sprintf(ipcamera_temp_info.pwd, "%s", lv_textarea_get_text(ext->ta));
					break;

				default:
					break;
				}
				Debug("kb->obj_data:%d   ipcamera_temp_info ip:%s   textarea:%s\n", kb->obj_data, ipcamera_temp_info.ip, lv_textarea_get_text(ext->ta));
				ipcamera_page_switch(MANUALLY_PAGE);
				return;
			}
			else
			{
				if (ipc_info_input_progress == 0)
				{
					memset(ipcamera_temp_info.name, 0, sizeof(ipcamera_temp_info.name));
					sprintf(ipcamera_temp_info.name, "%s", lv_textarea_get_text(ext->ta));
					lv_textarea_set_text(ext->ta, "");
					lv_textarea_set_placeholder_text(ext->ta, text_str(STR_ENTER_IPC_PWD));
					ipc_info_input_progress = 1;
				}
				else if (ipc_info_input_progress)
				{
					memset(ipcamera_temp_info.pwd, 0, sizeof(ipcamera_temp_info.pwd));
					sprintf(ipcamera_temp_info.pwd, "%s", lv_textarea_get_text(ext->ta));

					// lv_obj_t * obj = lv_obj_get_child(lv_scr_act(),NULL );
					// lv_obj_del_reload(&obj);

					ipc_setting_process = IPC_URL_SEARCH;

					curr_select_ipc_index = kb->obj_data;

					sat_ipcamera_user_password_set(curr_select_ipc_index, ipcamera_temp_info.name, ipcamera_temp_info.pwd);

					memcpy(ipcamera_temp_info.ip, sat_ipcamera_ipaddr_get(curr_select_ipc_index), sizeof(ipcamera_temp_info.ip));
					if (msg_loading_t == NULL)
						msg_loading_t = msg_window_create(text_str(STR_LOAD), true);
					return;
				}
			}
		}
		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_ENTER_PNG) /* LV_SYMBOL_NEW_LINE*/
			printf("***LV_SYMBOL_NEW_LINE***\n");

		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_LEFT_PNG) /*LV_SYMBOL_LEFT*/
			printf("***LV_SYMBOL_LEFT***\n");

		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_RIGHT_PNG) /*LV_SYMBOL_RIGHT*/
			printf("***LV_SYMBOL_RIGHT***\n");

		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_DELETL_PNG || ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_DELETS_PNG) /*LV_SYMBOL_BACKSPACE*/
			lv_textarea_del_char(ext->ta);

		else if (ext->btnm.pattern_p[btn_id]->offset == ROM_RES_KB_SPACE_PNG) /*LV_SYMBOL_SPACE*/
			lv_textarea_add_char(ext->ta, ' ');
	}
	else if (strcmp(txt, "+/-") == 0)
	{
		uint16_t cur = lv_textarea_get_cursor_pos(ext->ta);
		const char *ta_txt = lv_textarea_get_text(ext->ta);
		if (ta_txt[0] == '-')
		{
			lv_textarea_set_cursor_pos(ext->ta, 1);
			lv_textarea_del_char(ext->ta);
			lv_textarea_add_char(ext->ta, '+');
			lv_textarea_set_cursor_pos(ext->ta, cur);
		}
		else if (ta_txt[0] == '+')
		{
			lv_textarea_set_cursor_pos(ext->ta, 1);
			lv_textarea_del_char(ext->ta);
			lv_textarea_add_char(ext->ta, '-');
			lv_textarea_set_cursor_pos(ext->ta, cur);
		}
		else
		{
			lv_textarea_set_cursor_pos(ext->ta, 0);
			lv_textarea_add_char(ext->ta, '-');
			lv_textarea_set_cursor_pos(ext->ta, cur + 1);
		}
	}
	else
	{
		lv_textarea_add_text(ext->ta, txt);
	}
}
#endif
#if 1
// 创建键盘
static lv_obj_t *keybord_create(lv_obj_t *parent, lv_obj_t *ta)
{
	lv_obj_t *kb = lv_keyboard_create(parent, NULL);

	set_location(kb, 0, 200, 1024, 400);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
	lv_keyboard_set_textarea(kb, ta);
	lv_keyboard_set_cursor_manage(kb, false);
	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x060606));
	lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_OPA_60);

	lv_obj_set_style_local_pad_top(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 7);
	lv_obj_set_style_local_pad_bottom(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 5);
	lv_obj_set_style_local_pad_left(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 7);
	lv_obj_set_style_local_pad_right(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 7);
	lv_obj_set_style_local_pad_inner(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 7);
	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0x002538));
	lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BTN, LV_STATE_PRESSED, lv_color_hex(0xFC0000));

	lv_obj_set_style_local_text_font(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_set_style_local_radius(kb, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);

	keyboard_layout(kb);

	ipc_info_input_progress = 0;

	static btn_data kb_data = btn_data_up_create(lv_keyboard_event1_cb);

	kb->user_data = &kb_data;
	btn_touch_event_listen(kb);


	return kb;
}

#endif

// 添加ipc按钮的回调
static void camera_manually_display(void);
static void add_ipc_btn_up(lv_obj_t *obj)
{
	memset(&ipcamera_temp_info, 0, sizeof(ipcamera_temp_info));
	ipcamera_page_switch(MANUALLY_PAGE);
}

static lv_obj_t *addipc_btn_create(lv_obj_t *parent, int x, int y, int w, int h)
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);

	// lv_obj_set_parent_event(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, BTN_PRESSED_COLOR);

	lv_btn_set_layout(btn, LV_LAYOUT_OFF);

	lv_obj_t *img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, x, y + 50);
	lv_obj_set_size(img1, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img1, &info1);

	// ipc的名称
	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(label, text_str(STR_ADD_MANUALLY));
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	// ipc的图片 根据信号强度选择对应的图片
	rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ADD_WIFI_PNG);
	lv_obj_t *img = lv_img_create(btn, NULL);

	lv_img_set_src(img, &info);
	lv_obj_align(img, btn, LV_ALIGN_IN_RIGHT_MID, -10, 0);

	static btn_data btn_data = btn_data_up_create(add_ipc_btn_up);
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	return btn;
}

// 从输入ipc的界面返回
static void keyback_btn_up(lv_obj_t *obj)
{
	// home_bg_display();
	// fb_video_mode_enable(false);

	// lv_obj_t * obj_temp = lv_obj_get_child(lv_scr_act(),NULL );
	// lv_obj_del_reload(&obj_temp);

	if (ipc_setting_process != IPC_MANUALLY_ADD)
	{
		ipcamera_page_switch(SCAN_PAGE);
	}
	else
	{
		ipcamera_page_switch(MANUALLY_PAGE);
	}
}

static void keyback_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data = btn_data_create(NULL, keyback_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_pos(btn, 25, 25);
	lv_obj_set_size(btn, 60, 60);

	{
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
		lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0x4d, 0x7a, 0xFF));
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
		lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_70);

		lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 45);
		lv_obj_set_style_local_radius(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, 45);
	}

	lv_obj_t *img = lv_img_create(btn, NULL);
	lv_img_set_src(img, (void *)&info);

	// setting_wifi_btn_img_transform_set(img);
	lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);

	btn_data.user_data = img;
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);

	lv_obj_set_ext_click_area(btn, 10, 10, 10, 10);
}

extern lv_obj_t *input_textarea_create(lv_obj_t *parent, int x, int y, int w, int h, int max_length, bool pwd_mode, const char *txt);

static void keyboard_page_create(lv_obj_t *obj, char *str1)
{
	/*创建新的容器 在容器上操作*/

	// Debug("obj addr:%p obj_data:%s  || %s\n",obj,obj->clict_data,sat_ipcamera_ipaddr_get(1));
	ipc_setting_layout.layout_page[KEYBOARD_PAGE] = lv_cont_create(lv_scr_act(), NULL);
	ipc_setting_layout.layout_page[KEYBOARD_PAGE]->user_data = obj->user_data;
	set_location(ipc_setting_layout.layout_page[KEYBOARD_PAGE], 0, 0, 1024, 600);
	lv_cont_set_layout(ipc_setting_layout.layout_page[KEYBOARD_PAGE], LV_LAYOUT_OFF);
	lv_obj_set_style_local_bg_color(ipc_setting_layout.layout_page[KEYBOARD_PAGE], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00090E));
	lv_obj_set_style_local_bg_opa(ipc_setting_layout.layout_page[KEYBOARD_PAGE], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	keyback_btn_create(ipc_setting_layout.layout_page[KEYBOARD_PAGE]); // 创建放回按钮

	// 创建一个输入框
	lv_obj_t *ta = input_textarea_create(ipc_setting_layout.layout_page[KEYBOARD_PAGE], 40, 87, 942, 77, 16, false, str1);
	// if(sizeof(obj->clict_data))
	// 	lv_textarea_set_text(ta, obj->clict_data);

	// 创建一个键盘 并且绑定这个输入框
	lv_obj_t *kb = keybord_create(ipc_setting_layout.layout_page[KEYBOARD_PAGE], ta);
	kb->obj_data = obj->obj_data;
	lv_obj_set_id(kb, 1);

	lv_obj_t *img = lv_img_create(ipc_setting_layout.layout_page[KEYBOARD_PAGE], NULL);
	lv_obj_set_pos(img, 0, 198);
	lv_obj_set_size(img, 1024, 4);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE3_PNG);
	lv_img_set_src(img, &info1);

	ipcamera_page_switch(KEYBOARD_PAGE);
	// 键盘的保存回调 保存wifi名称
}

static void ipc_btn_up(lv_obj_t *obj)
{
	keyboard_page_create(obj, text_str(STR_ENTER_IPC_USER));
}

static lv_obj_t *ipc_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int index)
{

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_id(btn, y / 52);

	lv_obj_set_drag_parent(btn, true);

	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, BTN_PRESSED_COLOR);

	lv_obj_t *img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, x, y + 50);
	lv_obj_set_size(img1, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img1, &info1);

	// ipc的名称
	lv_obj_t *label = lv_label_create(btn, NULL);

	lv_label_set_text(label, sat_ipcamera_ipaddr_get(index));
	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	// 单独的回调函数
	static btn_data btn_data = btn_data_up_create(ipc_btn_up);
	btn->obj_data = index;
	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);
	// Debug("btn addr:%p btn_data:%s\n",btn,sat_ipcamera_ipaddr_get(index));

	return btn;
}

static void connect_ipc_btn_up(lv_obj_t *obj)
{
	if (obj == NULL)
	{
		return;
	}
	onvif_video_mode_open(obj->obj_data == IPCAMERA1 ? user_data_get()->camera1.url : user_data_get()->camera2.url, true);
	ipc_preview_create(obj->obj_data == IPCAMERA1 ? user_data_get()->camera1.ip : user_data_get()->camera2.ip, obj->obj_data);
}

static lv_obj_t *connect_ipc_btn_create(lv_obj_t *parent, int x, int y, int w, int h, camera_info *ipc, ipc_dev_id dev_id)
{

	lv_obj_t *btn = lv_btn_create(parent, NULL);
	lv_obj_set_drag_parent(btn, true);
	lv_btn_set_layout(btn, LV_FIT_NONE);
	set_location(btn, x, y, w, h);
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x20, 0x20, 0x20));
	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, BTN_PRESSED_COLOR);

	lv_obj_t *img1 = lv_img_create(parent, NULL);
	lv_obj_set_pos(img1, x, y + 50);
	lv_obj_set_size(img1, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img1, &info1);

	// ipc的名称

	lv_obj_t *label = lv_label_create(btn, NULL);
	lv_label_set_text(label, dev_id == IPCAMERA1 ? text_str(STR_CAMERA1) : text_str(STR_CAMERA2)); // ipc名称

	lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0);

	// 单独的回调函数
	static btn_data btn_data = btn_data_up_create(connect_ipc_btn_up);
	btn->user_data = &btn_data;
	btn->obj_data = dev_id;
	btn_touch_event_listen(btn);

	return btn;
}

static void findipc_ipcbtn_create(lv_obj_t *parent)
{
	// Debug("cctv1 url:%s\n",user_data_get()->camera1.url);
	// Debug("cctv2 url:%s\n",user_data_get()->camera2.url);
	int8_t ctrl_sum = 0; // 控件数
	if (user_data_get()->camera1.url[0] == 'r')
	{
		// ipc已经连接 创建按钮显示当前ipc的按钮
		connect_ipc_btn_create(parent, 0, ((ctrl_sum) * 52), 422, 52, &user_data_get()->camera1, IPCAMERA1);
		ctrl_sum++;
	}

	if (user_data_get()->camera2.url[0] == 'r')
	{
		// ipc已经连接 创建按钮显示当前ipc的按钮
		connect_ipc_btn_create(parent, 0, ((ctrl_sum) * 52), 422, 52, &user_data_get()->camera2, IPCAMERA2);
		ctrl_sum++;
	}

	addipc_btn_create(parent, 0, ((ctrl_sum) * 52) + 10, 422, 52);
	ctrl_sum++;

	if (ipc_online_total != 0)
	{
		for (int i = 0; i < ipc_online_total; i++)
		{
			if ((memcmp(sat_ipcamera_ipaddr_get(i), user_data_get()->camera1.ip, sizeof(user_data_get()->camera1.ip)) == 0 && user_data_get()->camera1.url[0] == 'r') ||
				(memcmp(sat_ipcamera_ipaddr_get(i), user_data_get()->camera2.ip, sizeof(user_data_get()->camera2.ip)) == 0 && user_data_get()->camera2.url[0] == 'r'))
			{
				continue;
			}
			ipc_btn_create(parent, 0, (ctrl_sum * 52) + 20, 422, 52, i);
			ctrl_sum++;
		}
	}
}

static void ipc_page_touch_anything(lv_obj_t *obj, lv_event_t event)
{
}

static void ipc_page_destroy(void)
{
	if (ipc_setting_layout.layout_page[SCAN_PAGE] == NULL)
	{
		return;
	}

	lv_obj_t *parent = lv_obj_get_child_form_id(ipc_setting_layout.layout_page[SCAN_PAGE], 987);
	if (parent == NULL)
	{
		return;
	}
	lv_obj_del_reload(&parent);
}

// 创建整个关于ipc的页面 可以上下滑动
static void ipc_page_create(lv_obj_t *parent)
{
	// 创建页面
	ipc_page = lv_page_create(parent, NULL); // 在当前活跃的屏幕上创建页面

	static btn_data page_data = btn_data_anything_create(ipc_page_touch_anything);
	ipc_page->user_data = &page_data;
	btn_touch_event_listen(ipc_page);
	lv_obj_set_id(ipc_page, 987);

	lv_obj_t *cont = lv_obj_get_child_back(ipc_page, NULL);
	if (cont != NULL)
	{
		lv_cont_set_fit4(cont, LV_FIT_PARENT, LV_FIT_PARENT, LV_FIT_TIGHT, LV_FIT_MAX);
		lv_cont_set_layout(cont, LV_LAYOUT_OFF);
	}

	set_location(ipc_page, 452, 0, 466, 600);
	lv_page_set_edge_flash(ipc_page, 1);
	lv_page_set_scrollbar_mode(ipc_page, LV_SCROLLBAR_MODE_DRAG);
	lv_obj_set_style_local_bg_color(ipc_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, lv_color_make(0x0, 0x0, 0x00));
	lv_obj_set_style_local_bg_opa(ipc_page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_OPA_10);
	lv_obj_set_style_local_bg_color(ipc_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, lv_color_make(0x0, 0x20, 0x00));
	lv_obj_set_style_local_bg_opa(ipc_page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	findipc_ipcbtn_create(ipc_page); // 搜索ipc并且创建按钮

	// lv_switch_on(sw, LV_ANIM_OFF);

	//}//else
	// lv_switch_off(sw, LV_ANIM_OFF);//如果ipc是关闭状态 无需搜索
}

static void IPC_btn_down(lv_obj_t *obj)
{
}

static void IPC_btn_create(lv_obj_t *parent)
{

	lv_obj_t *img = lv_img_create(parent, NULL);
	lv_obj_set_pos(img, 199, 24);
	lv_obj_set_size(img, 252, 52);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ITEM_FOCUS_PNG);
	lv_img_set_src(img, &info);
	lv_obj_set_id(img, 1);

	static btn_data btn_data1 = btn_data_create(IPC_btn_down, NULL, NULL);
	lv_obj_t *btn = lv_btn_create(parent, NULL);

	lv_obj_set_pos(btn, 199, 24);
	lv_obj_set_size(btn, 252, 52);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_WEBCAM));
	lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	btn->user_data = &btn_data1;
	btn_touch_event_listen(btn);
}

static void ipc_img_text_display(lv_obj_t *parent)
{
	lv_obj_t *img = lv_img_create(parent, NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_CAMERA_UNFOCUS_PNG);
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_value_str(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CAMERA_SET));
	lv_obj_set_style_local_value_color(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_align(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	IPC_btn_create(parent);
}

static void ipc_setting_back_btn_up(lv_obj_t *obj)
{
	// Debug("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	goto_layout(pLAYOUT(setting));
}
lv_obj_t *ipc_setting_back_create(lv_obj_t *parent, void (*up)(lv_obj_t *obj))
{
	static btn_data btn_data;
	btn_data.OPS_UP = up;

	static rom_bin_info info = rom_bin_info_get(ROM_RES_HOME_BACK_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_HOME_BACK_FOCUS_PNG);
	lv_obj_t *btn = lv_btn_create(parent, NULL);

	lv_obj_set_pos(btn, 929, 505);
	lv_obj_set_size(btn, 67, 67);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	}

	{
		lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
	}
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);

	btn->user_data = &btn_data;
	btn_touch_event_listen(btn);
	return btn;
}

static void scan_page_init(void);
static void scan_page_display(void)
{
	scan_page_init();
	ipc_img_text_display(ipc_setting_layout.layout_page[SCAN_PAGE]);
	ipc_page_create(ipc_setting_layout.layout_page[SCAN_PAGE]);
	ipc_setting_back_create(ipc_setting_layout.layout_page[SCAN_PAGE], ipc_setting_back_btn_up);
}

static void ipc_search_task(struct _lv_task_t *task_t)
{
	static int url_get_frequency = 0;
	// extern unsigned long long os_get_ms(void);
	// if(ipc_setting_process != IPC_ONLINE_SEARCH && os_get_ms() - prev_ts > 3000)
	// {
	// 	ipc_setting_process = IPC_ONLINE_SEARCH;
	// }

	switch (ipc_setting_process)
	{
	case IPC_ONLINE_SEARCH:
		if (sat_ipcamera_status_get() == false)
		{
			sat_ipcamera_device_online_search();
			ipc_setting_process = IPC_SEARCH_UPDATE;
		}
		break;
	case IPC_SEARCH_UPDATE:
		if (ipc_setting_layout.ipc_page == SCAN_PAGE && (sat_ipcamera_status_get() == false))
		{
			if (sat_ipcamera_info_update())
			{
				ipc_online_total = sat_ipcamera_online_num_get();
				ipc_page_destroy();
				ipc_page_create(ipc_setting_layout.layout_page[SCAN_PAGE]);
				printf("sat_ipcamera_online_num_get: %d\n", ipc_online_total);
				for (int i = 0; i < ipc_online_total; i++)
				{
					printf("device %d      IP: %s    port: %d\n", i, sat_ipcamera_ipaddr_get(i), sat_ipcamera_port_get(i));
				}
			}
			ipc_setting_process = IPC_ONLINE_SEARCH;

			if (msg_loading_t)
				lv_obj_del_reload(&(msg_loading_t)); /* !!! 一定要删除loading 弹窗的父对象 */
		}
		break;
	case IPC_URL_SEARCH:
		if (sat_ipcamera_rtsp_url_get(curr_select_ipc_index))
		{
			ipc_setting_process = IPC_URL_UPDATE;
			url_get_frequency++;
		}
		break;
	case IPC_URL_UPDATE:
		if (sat_ipcamera_status_get() == false)
		{
			int nstream;
			char *pstr = NULL;
			if ((nstream = sat_ipcamera_profile_token_num_get(curr_select_ipc_index)))
			{

				Debug("鉴权成功 nstream:%d\n", nstream);
				for (int i = 0; i < nstream; i++)
				{
					pstr = strstr(sat_ipcamera_rtsp_addr_get(curr_select_ipc_index, i), "//");
					if (pstr != NULL)
					{
						Debug("GET Valid URL,break %s!!!!!!!!!!!!\n", pstr);
						break;
					}
				}

				if (pstr)
				{
					pstr += 2;
					memset(ipcamera_temp_info.url, 0, sizeof(ipcamera_temp_info.url));
					sprintf(ipcamera_temp_info.url, "rtsp://%s:%s@%s", ipcamera_temp_info.name, ipcamera_temp_info.pwd, pstr);
					onvif_video_mode_open(ipcamera_temp_info.url, true);
					ipc_preview_create(sat_ipcamera_ipaddr_get(curr_select_ipc_index), IPC_UNKNOWN);
				}
				else
				{
					ipc_msg_windows_start(text_str(STR_LINK_DEVICE_FAILED));
					Debug("鉴权失败 URL is NULL\n");
					url_get_frequency = 0;
				}
			}
			else if (url_get_frequency > 2)
			{
				// 鉴权失败
				ipc_msg_windows_start(text_str(STR_LINK_DEVICE_FAILED));
				Debug("鉴权失败 IPaddr:%s \n", ipcamera_temp_info.ip);
				url_get_frequency = 0;
			}
			else
			{
				ipc_setting_process = IPC_URL_SEARCH;
				return;
			}

			ipc_setting_process = IPC_ONLINE_SEARCH;

			if (msg_loading_t)
				lv_obj_del_reload(&(msg_loading_t)); /* !!! 一定要删除loading 弹窗的父对象 */
		}
		break;
	default:
		break;
	}
}

/* ******************************************************************************** */
/* ******************************************************************************** */
/* ************************** ↓↓↓ demo ↓↓↓ ****************************************** */

static void scan_page_init(void)
{
	ipc_setting_layout.layout_page[SCAN_PAGE] = lv_cont_create(lv_scr_act(), NULL);

	set_location(ipc_setting_layout.layout_page[SCAN_PAGE], 0, 0, 1024, 600);
	lv_cont_set_layout(ipc_setting_layout.layout_page[SCAN_PAGE], LV_LAYOUT_OFF);
	lv_obj_set_style_local_bg_opa(ipc_setting_layout.layout_page[SCAN_PAGE], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}

/* **********************************************manually camera add*************************************************** */

static void camera_set_btn_syn_up(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_DEFAULT);
}
static void camera_set_btn_syn_down(lv_obj_t *obj)
{
	btn_data *pdata = (btn_data *)obj->user_data;
	lv_obj_t *btn = (lv_obj_t *)pdata->user_data;
	lv_obj_set_state(btn, LV_STATE_PRESSED);
}

static void camera_set_btn_syn_event(lv_obj_t *obj, lv_event_t event)
{

	if (LV_EVENT_PRESS_LOST == event)
	{
		camera_set_btn_syn_up(obj);
	}
}

lv_obj_t *ipc_manually_btn_create(Controls_location coordinate, char *string1, char *string_lable, btn_data *btn_pdata, btn_data *btn_pdata1, btn_data *btn_pdata2, lv_obj_t *parent, unsigned int id)
{
	lv_obj_t *btn = lv_btn_create(parent, NULL);

	lv_obj_set_id(btn, id);
	lv_obj_set_pos(btn, coordinate.x, coordinate.y);
	lv_obj_set_size(btn, coordinate.width, coordinate.high);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ITEM_FOCUS_PNG);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
	// lv_obj_set_style_local_pattern_image(btn,LV_OBJ_PART_MAIN,LV_STATE_FOCUSED,&info);
	lv_obj_set_style_local_pattern_align(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_ALIGN_IN_LEFT_MID);

	if (string1 != NULL)
	{
		if (user_data_get()->language.index == HEBREW)
		{
			lv_obj_set_base_dir(btn, LV_BIDI_DIR_RTL);
		}
		lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string1);
		lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_align(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
		lv_obj_set_style_local_value_ofs_x(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 120);
		lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	}
	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_set_pos(label, coordinate.x + 6, coordinate.y + 6);
	lv_obj_set_size(label, 250, 52);
	lv_label_set_long_mode(label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(label, string_lable);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);

	btn_pdata->user_data = btn;
	btn->user_data = btn_pdata;

	btn_touch_event_listen(btn);

	if (btn_pdata1 != NULL)
	{
		lv_obj_t *imgbtn1 = lv_imgbtn_create(parent, NULL);
		lv_obj_set_pos(imgbtn1, 460, coordinate.y);
		lv_obj_set_size(imgbtn1, 52, 52);
		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ARROW_LEFT_UNFOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_RELEASED, &info);

		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ARROW_LEFT_FOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_PRESSED, &info1);
		btn_pdata1->user_data = btn;
		imgbtn1->user_data = btn_pdata1;
		lv_imgbtn_set_checkable(imgbtn1, true);
		btn_touch_event_listen(imgbtn1);
	}
	if (btn_pdata2 != NULL)
	{
		lv_obj_t *imgbtn1 = lv_imgbtn_create(parent, NULL);
		lv_obj_set_pos(imgbtn1, 867, coordinate.y);
		lv_obj_set_size(imgbtn1, 52, 52);

		static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_ARROW_RIGHT_UNFOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_RELEASED, &info);

		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_ARROW_RIGHT_FOCUS_PNG);
		lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_PRESSED, &info1);
		btn_pdata2->user_data = btn;
		imgbtn1->user_data = btn_pdata2;
		imgbtn1->obj_data = id;
		lv_imgbtn_set_checkable(imgbtn1, true);
		btn_touch_event_listen(imgbtn1);
	}
	lv_obj_t *img = lv_img_create(parent, NULL);
	lv_obj_set_pos(img, 472, coordinate.y + 50);
	lv_obj_set_size(img, 422, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE_PNG);
	lv_img_set_src(img, &info1);
	btn->obj_data = id;
	return btn;
}

static void camera_setting_btn_text_create(lv_obj_t *parent)
{
	lv_obj_t *img = lv_img_create(parent, NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_CAMERA_UNFOCUS_PNG);
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_value_str(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CAMERA_SET));
	lv_obj_set_style_local_value_color(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_align(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_set_pos(label, 470, 28);
	lv_obj_set_size(label, 124, 38);

	lv_label_set_text(label, text_str(STR_ADD_MANUALLY));
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
}
static void camera_camera_model_right_btn_up(lv_obj_t *obj)
{
	camera_set_btn_syn_up(obj);
	char *str1 = NULL;
	ipcamera_temp_info.model = !ipcamera_temp_info.model;
	str1 = ipcamera_temp_info.model ? text_str(STR_HIKVISION) : text_str(STR_DAHUA);
	lv_obj_t *btn = lv_obj_get_child_form_id(ipc_setting_layout.layout_page[MANUALLY_PAGE], MODEL_MODULE);
	lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str1);
}

static void camera_camera_model_set_btn_create(Controls_location coordinate, lv_obj_t *parent)
{
	static btn_data btn_data1 = btn_data_create(camera_set_btn_syn_down, camera_camera_model_right_btn_up, NULL);
	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_model_right_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data1.OPS_ANYTHING = camera_set_btn_syn_event;
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;

	static char *str1 = NULL;
	str1 = ipcamera_temp_info.model ? text_str(STR_HIKVISION) : text_str(STR_DAHUA);
	ipc_manually_btn_create(coordinate, str1, text_str(STR_CAMERA_MODEL), &btn_data3, &btn_data1, &btn_data2, parent, MODEL_MODULE);
}
static void camera_camera_ip_set_btn_up(lv_obj_t *obj)
{
	camera_set_btn_syn_up(obj);
	keyboard_page_create(obj, text_str(STR_ENTER_CAMERA_IP_ADDR));
}
static void camera_camera_ip_set_btn_up_1(lv_obj_t *obj)
{
	keyboard_page_create(obj, text_str(STR_ENTER_CAMERA_IP_ADDR));
}

static void camera_camera_ip_set_btn_create(Controls_location coordinate, lv_obj_t *parent)
{
	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_ip_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, camera_camera_ip_set_btn_up_1, NULL);
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;

	static char *str1 = NULL;

	str1 = ipcamera_temp_info.ip;
	Debug("camera_ip:%s\n", str1);
	ipc_manually_btn_create(coordinate, str1, text_str(STR_CAMERA_IP_ADDRESS), &btn_data3, NULL, &btn_data2, parent, IP_MODULE);
}
static void camera_camera_account_set_btn_up(lv_obj_t *obj)
{
	camera_set_btn_syn_up(obj);
	keyboard_page_create(obj, text_str(STR_ENTER_CAMERA_ACCOUNT));
}
static void camera_camera_account_set_btn_up_1(lv_obj_t *obj)
{
	keyboard_page_create(obj, text_str(STR_ENTER_CAMERA_ACCOUNT));
}

static void camera_camera_account_set_btn_create(Controls_location coordinate, lv_obj_t *parent)
{

	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_account_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, camera_camera_account_set_btn_up_1, NULL);
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;

	static char *str1 = NULL;
	str1 = ipcamera_temp_info.name;
	ipc_manually_btn_create(coordinate, str1, text_str(STR_ACCOUNT_NUMBER), &btn_data3, NULL, &btn_data2, parent, ACCOUNT_MODULE);
}

static void camera_camera_pwd_mode_set_btn_up(lv_obj_t *obj)
{
	camera_set_btn_syn_up(obj);
	keyboard_page_create(obj, text_str(STR_ENTER_CAMERA_PASSWORD));
}
static void camera_camera_pwd_mode_set_btn_up_1(lv_obj_t *obj)
{
	keyboard_page_create(obj, text_str(STR_ENTER_CAMERA_PASSWORD));
}

static void camera_camera_pwd_mode_set_btn_create(Controls_location coordinate, lv_obj_t *parent)
{
	static btn_data btn_data2 = btn_data_create(camera_set_btn_syn_down, camera_camera_pwd_mode_set_btn_up, NULL);
	static btn_data btn_data3 = btn_data_create(NULL, camera_camera_pwd_mode_set_btn_up_1, NULL);
	btn_data2.OPS_ANYTHING = camera_set_btn_syn_event;

	static char *str1 = NULL;
	str1 = ipcamera_temp_info.pwd;
	ipc_manually_btn_create(coordinate, str1, text_str(STR_PASSWORD), &btn_data3, NULL, &btn_data2, parent, PASSWORD_MODULE);
}
static void camera_setting_back_btn_up(lv_obj_t *obj)
{
	ipc_setting_process = IPC_ONLINE_SEARCH;
	ipcamera_page_switch(SCAN_PAGE);
}

static lv_task_t *manually_link_t = NULL;
static void manually_link_task(struct _lv_task_t *task_t)
{
	if (msg_loading_t)
	{
		lv_obj_del_reload(&(msg_loading_t));
		register_rtsp_callback(NULL);
		ipc_msg_windows_start(text_str(STR_LINK_DEVICE_FAILED));
		onvif_video_mode_close();
		Debug("鉴权失败\n\n\n");
	}

	if (manually_link_t)
	{
		lv_task_del(manually_link_t);
		manually_link_t = NULL;
	}
}
static void affirm_btn_up(lv_obj_t *obj)
{

	if (obj == NULL)
	{
		Debug("ERROR!!!!!!\n\n\n\n");
		return;
	}

	lv_obj_t *btn = (lv_obj_t *)obj->user_data;
	if (btn == NULL)
	{
		Debug("ERROR!!!!!!\n\n\n\n");
		return;
	}

	// Debug("%p,%p\n",obj,obj->user_data);
	lv_obj_set_state(obj, LV_STATE_DEFAULT);

	memset(ipcamera_temp_info.url, 0, sizeof(ipcamera_temp_info.url));

	if (ipcamera_temp_info.model)
	{
		sprintf(ipcamera_temp_info.url, "rtsp://%s:%s@%s:554/Streaming/Channels/1", ipcamera_temp_info.name, ipcamera_temp_info.pwd, ipcamera_temp_info.ip);
	}
	else
	{
		sprintf(ipcamera_temp_info.url, "rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=1", ipcamera_temp_info.name, ipcamera_temp_info.pwd, ipcamera_temp_info.ip);
	}

	if (msg_loading_t == NULL)
	{
		msg_loading_t = msg_window_create(text_str(STR_LOAD), true);
		if (manually_link_t == NULL)
		{
			manually_link_t = lv_task_create(manually_link_task, 3000, LV_TASK_PRIO_HIGH, NULL);
		}
	}

	onvif_video_mode_open(ipcamera_temp_info.url, false);

	register_rtsp_callback(manually_link_backfunc);
	return;
}

static lv_obj_t *affirm_btn_create(lv_obj_t *parent)
{
	static btn_data btn_data;
	btn_data.OPS_UP = affirm_btn_up;
	btn_data.OPS_ANYTHING = NULL;
	btn_data.user_data = NULL;
	btn_data.obj_tone = true;
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_AFFIRM_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_AFFIRM_FOCUS_PNG);
	Controls_location coordinate = {929, 402, 67, 67};

	lv_obj_t *btn = lv_btn_create(parent, NULL);

	lv_obj_set_pos(btn, coordinate.x, coordinate.y);
	lv_obj_set_size(btn, coordinate.width, coordinate.high);

	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, (void *)&info);
	lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, (void *)&info1);

	btn->user_data = &btn_data;
	// Debug("%p,%p\n",btn,&btn_data);
	btn_touch_event_listen(btn);
	return btn;
}
/*
static void camera_temp_info_refresh(void)
{
	lv_obj_t * obj = NULL;
	if((obj = lv_obj_get_child_form_id(ipc_setting_layout.layout_page[MANUALLY_PAGE],IP_MODULE)))
	{
		lv_obj_set_style_local_value_str(obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,ipcamera_temp_info.ip);
		lv_obj_set_style_local_value_color(obj,LV_OBJ_PART_MAIN,LV_STATE_PRESSED,BTN_PRESS_COLOR);
		lv_obj_set_style_local_value_align(obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_ALIGN_CENTER);
		lv_obj_set_style_local_value_ofs_x(obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,120);
		lv_obj_set_style_local_value_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	}
	if((obj = lv_obj_get_child_form_id(ipc_setting_layout.layout_page[MANUALLY_PAGE],ACCOUNT_MODULE)))
	{

		lv_obj_set_style_local_value_str(obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,ipcamera_temp_info.name);
	}
	if((obj = lv_obj_get_child_form_id(ipc_setting_layout.layout_page[MANUALLY_PAGE],PASSWORD_MODULE)))
	{

		lv_obj_set_style_local_value_str(obj,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,ipcamera_temp_info.pwd);
	}
} */
void manually_page_init(void)
{
	ipc_setting_layout.layout_page[MANUALLY_PAGE] = lv_cont_create(lv_scr_act(), NULL);

	set_location(ipc_setting_layout.layout_page[MANUALLY_PAGE], 0, 0, 1024, 600);
	lv_cont_set_layout(ipc_setting_layout.layout_page[MANUALLY_PAGE], LV_LAYOUT_OFF);
	lv_obj_set_style_local_bg_opa(ipc_setting_layout.layout_page[MANUALLY_PAGE], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}

static void camera_manually_display(void)
{
	Controls_location module_coordinate[] = CAMERA_MODULE_COORDINATE_INIT;
	manually_page_init();
	camera_setting_btn_text_create(ipc_setting_layout.layout_page[MANUALLY_PAGE]);
	camera_camera_model_set_btn_create(module_coordinate[MODEL_MODULE], ipc_setting_layout.layout_page[MANUALLY_PAGE]);
	camera_camera_ip_set_btn_create(module_coordinate[IP_MODULE], ipc_setting_layout.layout_page[MANUALLY_PAGE]);
	camera_camera_account_set_btn_create(module_coordinate[ACCOUNT_MODULE], ipc_setting_layout.layout_page[MANUALLY_PAGE]);
	camera_camera_pwd_mode_set_btn_create(module_coordinate[PASSWORD_MODULE], ipc_setting_layout.layout_page[MANUALLY_PAGE]);
	ipc_setting_back_create(ipc_setting_layout.layout_page[MANUALLY_PAGE], camera_setting_back_btn_up);
	affirm_btn_create(ipc_setting_layout.layout_page[MANUALLY_PAGE]);
}

static void LAYOUT_ENETER_FUNC(setting_ipc)
{
	ipc_device_param_init();
	ipcamera_temp_info_init();
	setting_bg_display();
	scan_page_display();

	ipcamera_search_task = lv_task_create(ipc_search_task, 1000, LV_TASK_PRIO_HIGH, NULL);
	if (msg_loading_t == NULL)
		msg_loading_t = msg_window_create(text_str(STR_LOAD), true);
	ipc_search_task(NULL);
}

static void LAYOUT_QUIT_FUNC(setting_ipc)
{

	lv_area_t area[] = {{0, 0, 1024, 600}};
	gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

	if (msg_loading_t)
		lv_obj_del_reload(&(msg_loading_t)); /* !!! 一定要删除loading 弹窗的父对象 */

	if (manually_link_t)
	{
		lv_task_del(manually_link_t);
		manually_link_t = NULL;
	}
	onvif_video_mode_close();

	register_rtsp_callback(NULL);

	if (ipcamera_search_task)
	{
		lv_task_del(ipcamera_search_task);
		ipcamera_search_task = NULL;
	}
	ipc_setting_process = IPC_ONLINE_SEARCH;
	ipc_online_total = 0;
}

CREATE_LAYOUT(setting_ipc);

#endif