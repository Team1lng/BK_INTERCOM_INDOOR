#include "layout_define.h"

#ifndef PUBLIC_VERSION
#define PASSWORD_LEN 32
#else
#define PASSWORD_LEN 32
#endif

int get_pwd_str = 0; // 0:验证密码进入管理员设置  1：修改密码                    2：输入摄像机IP地址 3：输入摄像机账号

#define MANUFACTURER_PASSWORD "9149"
#define ADD_LOCK_CARD_PASSWORD "*077#"
#define DEL_LOCK_CARD_PASSWORD "*177#"
#define ADD_GATE_CARD_PASSWORD "*088#"
#define DEL_GATE_CARD_PASSWORD "*188#"
#define PWD_SHOT 6
#define PWD_LONG 7
#define PWD_ERROR 8
#define ADD_LOCK_CARD 9
#define DEL_LOCK_CARD 10
#define ADD_GATE1_CARD 11
#define DEL_GATE1_CARD 12

extern void add_del_card_cmd(network_device ch, char mode, char lock);

extern lv_obj_t *addwifi_back_btn_create(void);
extern lv_obj_t *input_textarea_create(lv_obj_t *parent, int x, int y, int w, int h, int max_length, bool pwd_mode, const char *txt);
extern char connectwifi_name[24];
extern const char **kb_map[];
extern const lv_btnmatrix_ctrl_t *kb_ctrl[];
extern int connectwifi_index;
extern lv_obj_t *connect_wifi_cb(void);
extern void set_msg_text(lv_obj_t *msg, int state);
extern int wifi_connection_status_sucess(void);

extern void set_location(lv_obj_t *obj, int x, int y, int w, int h);
extern void setting_btn_state_set(lv_obj_t *obj, lv_state_t state);
extern lv_obj_t *setting_wifi_btn_create(int x, int y, int w, int h, btn_data *btn_pdata, const void *img_src, bool bg_color);

static void password_input_back_btn_down(lv_obj_t *obj)
{
	setting_btn_state_set(obj, LV_STATE_PRESSED);
}

static void password_input_back_btn_up(lv_obj_t *obj)
{

	setting_btn_state_set(obj, LV_STATE_DEFAULT);

	if (get_pwd_str == 1)
	{
		goto_layout(pLAYOUT(setting_admin));
	}
	else if (get_pwd_str == 0)
	{
		goto_layout(pLAYOUT(setting_sys));
	}
	else if (get_pwd_str == 2 || get_pwd_str == 3 || get_pwd_str == 4 || get_pwd_str == 5 || get_pwd_str == 6 || get_pwd_str == 7)
	{
		goto_layout(pLAYOUT(setting_camera));
	}
}

// 创建返回按钮 返回wifi页面
lv_obj_t *password_input_back_btn_create(void)
{

	static btn_data btn_data = btn_data_create(password_input_back_btn_down, password_input_back_btn_up, NULL);
	rom_bin_info info = rom_bin_info_get(ROM_RES_THUMB_EXIT_PNG);
	lv_obj_t *btn = setting_wifi_btn_create(25, 25, 60, 60, &btn_data, &info, true);
	lv_obj_set_ext_click_area(btn, 10, 10, 10, 10);
	return btn;
}

// 创建标题
static void Password_head_label_create(void)
{
	lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_text_font(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_label_set_text(obj, get_pwd_str == 1 ? text_str(STR_CHANGE_PASSWORD) : get_pwd_str == 0 ? text_str(STR_PASSWORD_INPUT)
																							   : text_str(STR_PLEASE_ENTER));
	lv_obj_align(obj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);
}

// connect键盘的回调函数
static void lv_keyboard_event_cb(lv_obj_t *kb)
{
	LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

	// if(event != LV_EVENT_VALUE_CHANGED) return;

	lv_keyboard_ext_t *ext = lv_obj_get_ext_attr(kb);
	uint16_t btn_id = lv_btnmatrix_get_active_btn(kb);

	if (btn_id == LV_BTNMATRIX_BTN_NONE)
		return;
	if (lv_btnmatrix_get_btn_ctrl(kb, btn_id, LV_BTNMATRIX_CTRL_HIDDEN | LV_BTNMATRIX_CTRL_DISABLED))
		return;
	// if(lv_btnmatrix_get_btn_ctrl(kb, btn_id, LV_BTNMATRIX_CTRL_NO_REPEAT) && event == LV_EVENT_LONG_PRESSED_REPEAT) return;

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

		// 对图片进行调整
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

			const char *txt = lv_textarea_get_text(ext->ta);
			if (get_pwd_str == 1)
			{
				if (strlen(lv_textarea_get_text(ext->ta)) < 6)
				{
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, PWD_SHOT);
					lv_textarea_set_text(ext->ta, "");
					return;
				}
				else if (strlen(lv_textarea_get_text(ext->ta)) > 6)
				{
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, PWD_LONG);
					lv_textarea_set_text(ext->ta, "");
					return;
				}
				strncpy(user_data_get()->user_pwd, txt, 6);
				goto_layout(pLAYOUT(setting_admin));
			}
			else if (get_pwd_str == 0)
			{
				if (strcmp(txt, MANUFACTURER_PASSWORD) == 0)
				{
					strncpy(user_data_get()->user_pwd, "111111", 6);
					user_data_get()->admin_sw = 0;
					user_data_get()->other.family_id = 1;
					network_local_family_set(user_data_get()->other.family_id);
					goto_layout(pLAYOUT(setting_admin));
					return;
				}
				else if (strcmp(txt, ADD_LOCK_CARD_PASSWORD) == 0)
				{
					goto_layout(pLAYOUT(setting_sys));
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, ADD_LOCK_CARD);
					add_del_card_cmd(DEVICE_ALL, 2, 0);
					return;
				}
				else if (strcmp(txt, DEL_LOCK_CARD_PASSWORD) == 0)
				{
					goto_layout(pLAYOUT(setting_sys));
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, DEL_LOCK_CARD);
					add_del_card_cmd(DEVICE_ALL, 3, 2);
					return;
				}
				else if (strcmp(txt, ADD_GATE_CARD_PASSWORD) == 0)
				{
					goto_layout(pLAYOUT(setting_sys));
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, ADD_GATE1_CARD);
					add_del_card_cmd(DEVICE_ALL, 2, 1);
					return;
				}
				else if (strcmp(txt, DEL_GATE_CARD_PASSWORD) == 0)
				{
					goto_layout(pLAYOUT(setting_sys));
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, DEL_GATE1_CARD);
					add_del_card_cmd(DEVICE_ALL, 3, 3);
					return;
				}
				else if (strlen(lv_textarea_get_text(ext->ta)) < 6)
				{
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, PWD_SHOT);
					lv_textarea_set_text(ext->ta, "");
					return;
				}
				else if (strlen(lv_textarea_get_text(ext->ta)) > 6)
				{
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, PWD_LONG);
					lv_textarea_set_text(ext->ta, "");
					return;
				}
				if (strcmp(txt, user_data_get()->user_pwd) == 0)
				{
					goto_layout(pLAYOUT(setting_admin));
				}
				else
				{
					lv_obj_t *msg = connect_wifi_cb();
					set_msg_text(msg, PWD_ERROR);
					lv_textarea_set_text(ext->ta, "");
				}
			}
			else if (get_pwd_str == 2)
			{
				strncpy(user_data_get()->camera1.ip, txt, 16);
				user_data_get()->camera1.ip[15] = '\0';
				goto_layout(pLAYOUT(setting_camera));
			}
			else if (get_pwd_str == 3)
			{
				strncpy(user_data_get()->camera2.ip, txt, 16);
				user_data_get()->camera2.ip[15] = '\0';
				goto_layout(pLAYOUT(setting_camera));
			}
			else if (get_pwd_str == 4)
			{
				strncpy(user_data_get()->camera1.account, txt, 16);
				user_data_get()->camera1.account[15] = '\0';
				goto_layout(pLAYOUT(setting_camera));
			}
			else if (get_pwd_str == 5)
			{
				strncpy(user_data_get()->camera2.account, txt, 16);
				user_data_get()->camera2.account[15] = '\0';
				goto_layout(pLAYOUT(setting_camera));
			}
			else if (get_pwd_str == 6)
			{
				strncpy(user_data_get()->camera1.pwd, txt, 16);
				user_data_get()->camera1.pwd[15] = '\0';
				goto_layout(pLAYOUT(setting_camera));
			}
			else if (get_pwd_str == 7)
			{
				strncpy(user_data_get()->camera2.pwd, txt, 16);
				user_data_get()->camera2.pwd[15] = '\0';
				goto_layout(pLAYOUT(setting_camera));
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

// 对键盘进行设置布局
void password_keyboard_layout(lv_obj_t *obj)
{
	static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
	lv_keyboard_img_set(obj, 11, &info11);

	static rom_bin_info info22 = rom_bin_info_get(ROM_RES_KB_ENTER_PNG);
	lv_keyboard_img_set(obj, 22, &info22);

	static rom_bin_info info35 = rom_bin_info_get(ROM_RES_KB_CANCEL_PNG);
	lv_keyboard_img_set(obj, 35, &info35);

	static rom_bin_info info36 = rom_bin_info_get(ROM_RES_KB_LEFT_PNG);
	lv_keyboard_img_set(obj, 36, &info36);

	static rom_bin_info info37 = rom_bin_info_get(ROM_RES_KB_SPACE_PNG);
	lv_keyboard_img_set(obj, 37, &info37);

	static rom_bin_info info38 = rom_bin_info_get(ROM_RES_KB_RIGHT_PNG);
	lv_keyboard_img_set(obj, 38, &info38);

	static rom_bin_info info39 = rom_bin_info_get(ROM_RES_KB_APPLY_PNG);
	lv_keyboard_img_set(obj, 39, &info39);
}

// 创建键盘
static lv_obj_t *keybord_create(lv_obj_t *parent, lv_obj_t *ta)
{
	lv_obj_t *kb = lv_keyboard_create(parent, NULL);

	set_location(kb, 0, 200, 1024, 400);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
	lv_keyboard_set_textarea(kb, ta);
	lv_keyboard_set_cursor_manage(kb, false);
	lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x060606));
	lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

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

	password_keyboard_layout(kb);

	static btn_data kb_data = btn_data_up_create(lv_keyboard_event_cb);
	kb->user_data = &kb_data;
	btn_touch_event_listen(kb);

	return kb;
}

static lv_obj_t *pwd_ta = NULL;

// 改变密码模式
static void change_pwd_mode(lv_obj_t *obj)
{

	rom_bin_info info_pwd = rom_bin_info_get(ROM_RES_SETWIFI_PWD_PNG);
	rom_bin_info info_inv = rom_bin_info_get(ROM_RES_SETWIFI_VISIBLE_PNG);

	if (lv_textarea_get_pwd_mode(pwd_ta))
	{
		lv_img_set_src(obj, &info_inv);
		lv_textarea_set_pwd_mode(pwd_ta, false);
	}
	else
	{
		lv_img_set_src(obj, &info_pwd);
		lv_textarea_set_pwd_mode(pwd_ta, true);
	}
}

static void password_input_page_create(lv_obj_t *parent)
{
	// 如果是免费wifi

	lv_obj_t *password_cont = lv_cont_create(lv_scr_act(), NULL);
	set_location(password_cont, 0, 100, 1024, 500);
	lv_cont_set_layout(password_cont, LV_LAYOUT_OFF);
	lv_obj_set_style_local_bg_color(password_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00090E));
	lv_obj_set_style_local_bg_opa(password_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	// 创建一个输入框

	printf("get_pwd_str ===================>>>>%d\n\r", get_pwd_str);
	pwd_ta = input_textarea_create(password_cont, 40, 0, 942, 77, ((get_pwd_str == 1 || get_pwd_str == 0) ? 6 : PASSWORD_LEN), true, get_pwd_str < 1 ? text_str(STR_PLEASE_INPUT_PASSWORD) : get_pwd_str < 2 ? text_str(STR_PLEASE_ENTER_NEW_PASSWORD)
																																														 : get_pwd_str < 4	 ? text_str(STR_ENTER_CAMERA_IP_ADDR)
																																														 : get_pwd_str < 6	 ? text_str(STR_ENTER_CAMERA_ACCOUNT)
																																																			 : text_str(STR_ENTER_CAMERA_PASSWORD));
	// 创建密码模式切换 密码模式进去一定是不可见的 可见只是当时可见 退出再进任然不可见

	lv_obj_t *pwd_mode = lv_img_create(password_cont, NULL);
	// pwd_mode->user_data = ta;
	set_location(pwd_mode, 932, 15, 48, 48);
	static rom_bin_info pwd_info = rom_bin_info_get(ROM_RES_SETWIFI_PWD_PNG);
	static rom_bin_info pwd_info1 = rom_bin_info_get(ROM_RES_SETWIFI_VISIBLE_PNG);
	if (get_pwd_str == 2 || get_pwd_str == 3 || get_pwd_str == 4 || get_pwd_str == 5)
	{
		lv_textarea_set_pwd_mode(pwd_ta, false);
		lv_img_set_src(pwd_mode, &pwd_info1);
	}
	else
	{
		lv_img_set_src(pwd_mode, &pwd_info);
	}
	// lv_obj_set_event_cb(pwd_mode, change_pwd_mode);
	lv_obj_set_click(pwd_mode, true);
	lv_obj_set_ext_click_area(pwd_mode, 15, 15, 15, 15);

	static btn_data img_data = btn_data_up_create(change_pwd_mode);
	pwd_mode->user_data = &img_data;
	btn_touch_event_listen(pwd_mode);

	// 创建一个键盘 并且绑定这个输入框

	lv_obj_t *kb = keybord_create(password_cont, pwd_ta);

	lv_obj_set_pos(kb, 0, 100);
	lv_obj_t *img = lv_img_create(password_cont, NULL);
	lv_obj_set_pos(img, 0, 98);
	lv_obj_set_size(img, 1024, 4);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE3_PNG);
	lv_img_set_src(img, &info1);
}

extern void memory_print(void);

static void LAYOUT_ENETER_FUNC(password_input)
{
	// home_bg_display();
	system_bg_fill_color(0xff00090E, 0, 0, 1024, 600);
	Password_head_label_create(); // 创建标题
	password_input_back_btn_create();

	password_input_page_create(lv_scr_act());
}

static void LAYOUT_QUIT_FUNC(password_input)
{
	get_pwd_str = 0;
}

CREATE_LAYOUT(password_input);
