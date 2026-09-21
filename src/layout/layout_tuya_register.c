#include "layout_define.h"
#include "tuya_uuid_and_key.h"
extern bool tuya_sdk_inited;

static char *src[4] = {"Please enter key number", "Please enter key number agian", "The two inputs do not match.Please enter a new key number", "The key number was not found. Please enter a new key number"};
static bool text_flag = false;
#if 1
void keyboard_layout_num1(lv_obj_t *obj)
{
    static rom_bin_info info11 = rom_bin_info_get(ROM_RES_KB_DELETS_PNG);
    lv_keyboard_img_set(obj, 11, &info11);

    static rom_bin_info info39 = rom_bin_info_get(ROM_RES_KB_APPLY_PNG);
    lv_keyboard_img_set(obj, 9, &info39);
}

static int frist_text = 0;
static int second_text = 0;

static void lv_keyboard_event_cb1(lv_obj_t *kb)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

    printf("lv_keyboard_event_cb1\n");
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
            printf("ROM_RES_KB_APPLY_PNG\n");
            if (strlen(lv_textarea_get_text(ext->ta)) == 0)
            {
                printf("ta is null\n");
                return;
            }
            if (text_flag == false)
            {
                text_flag = true;
                printf("***%s***\n", src[text_flag]);
                // lv_label_set_text(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),11), 11), src[text_flag]);
                // lv_obj_align(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),11), 11), kb, LV_ALIGN_OUT_TOP_MID, 0, -15);
                // 获取文本框的文字

                frist_text = atoi(lv_textarea_get_text(ext->ta));
                lv_textarea_set_text(ext->ta, "");
                lv_textarea_set_placeholder_text(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(), 11), 11), src[text_flag]);
            }
            else if (text_flag == true)
            {
                text_flag = false;
                second_text = atoi(lv_textarea_get_text(ext->ta));

                if (second_text != frist_text)
                {
                    printf("The two inputs do not match.Please enter a new key number\n");
                    text_flag = false;
                    lv_textarea_set_placeholder_text(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(), 11), 11), src[2]);
                    // lv_obj_align(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),11), 11), kb, LV_ALIGN_OUT_TOP_MID, 0, -15);
                    frist_text = 0;
                    second_text = 0;
                    lv_textarea_set_text(ext->ta, "");
                }
                else if (second_text != 0)
                {
                    int ret = tuya_uuid_and_key_get(second_text, user_data_get()->tuya_info.tuya_uuid, user_data_get()->tuya_info.tuya_key);
                    printf("!!!!!!tuya register index :%d,result:%d  !!!!!!!!\n", second_text, ret);
                    if (ret == TUYA_ACTION_OK)
                    {
                        // tuya_uuid_and_key_read((unsigned char*)user_data_get()->tuya_info.tuya_uuid,(unsigned char*)user_data_get()->tuya_info.tuya_key);
                        printf("tuya_uuid :%s           tuya_key:%s  \n", user_data_get()->tuya_info.tuya_uuid, user_data_get()->tuya_info.tuya_key);
                        user_data_get()->tuya_info.index = second_text;
                        tuya_wifi_sdk_init(IPC_APP_PID, user_data_get()->tuya_info.tuya_uuid, user_data_get()->tuya_info.tuya_key);
                        goto_layout(pLAYOUT(home));
                    }
                    else
                    {
                        printf("The key number was not found. Please enter a new key number 1\n");
                        text_flag = false;
                        lv_textarea_set_placeholder_text(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(), 11), 11), src[3]);
                        // lv_obj_align(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),11), 11), kb, LV_ALIGN_OUT_TOP_MID, 0, -15);
                        frist_text = 0;
                        second_text = 0;
                        lv_textarea_set_text(ext->ta, "");
                    }
                }
                else
                {
                    printf("The key number was not found. Please enter a new key number 2\n");
                    text_flag = false;
                    lv_textarea_set_placeholder_text(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(), 11), 11), src[3]);
                    // lv_obj_align(lv_obj_get_child_form_id(lv_obj_get_child_form_id(lv_scr_act(),11), 11), kb, LV_ALIGN_OUT_TOP_MID, 0, -15);
                    frist_text = 0;
                    second_text = 0;
                    lv_textarea_set_text(ext->ta, "");
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
        printf("num txt : %s\n", txt);
        lv_textarea_add_text(ext->ta, txt);
    }
}

static lv_obj_t *keybord_create(lv_obj_t *parent, lv_obj_t *ta)
{
    lv_obj_t *kb = lv_keyboard_create(parent, NULL);

    set_location(kb, 300, 160, 424, 350);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
    lv_keyboard_set_textarea(kb, ta);
    lv_keyboard_set_cursor_manage(kb, false);

    lv_obj_set_style_local_border_side(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);

    lv_obj_set_style_local_bg_color(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x191919));
    lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BG, LV_STATE_PRESSED, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(kb, LV_KEYBOARD_PART_BTN, LV_STATE_PRESSED, LV_OPA_TRANSP);

    lv_obj_set_style_local_pad_top(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_bottom(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_left(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_right(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_inner(kb, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT, 30);

    lv_obj_set_style_local_text_color(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
    lv_obj_set_style_local_text_color(kb, LV_KEYBOARD_PART_BTN, LV_STATE_PRESSED, lv_color_hex(0x2a9ad2));

    lv_obj_set_style_local_text_font(kb, LV_KEYBOARD_PART_BTN, LV_STATE_DEFAULT, FONT_SIZE_L(42));

    keyboard_layout_num1(kb);

    static btn_data kb_data = btn_data_up_create(lv_keyboard_event_cb1);
    kb->user_data = &kb_data;
    btn_touch_event_listen(kb);

    return kb;
}
#endif

lv_obj_t *input_textarea_create1(lv_obj_t *parent, int w, int h)
{

    lv_obj_t *textarea = lv_textarea_create(parent, NULL);

    lv_obj_set_size(textarea, w, h);
    lv_obj_set_pos(textarea, 0, 10);

    lv_textarea_set_text(textarea, "");

    lv_textarea_set_max_length(textarea, 6);
    lv_textarea_set_one_line(textarea, true);
    lv_textarea_set_text_align(textarea, LV_LABEL_ALIGN_CENTER);
    lv_textarea_set_scrollbar_mode(textarea, LV_SCROLLBAR_MODE_OFF);

    lv_textarea_set_cursor_hidden(textarea, true);

    lv_textarea_set_pwd_mode(textarea, false);

    lv_textarea_set_placeholder_text(textarea, src[text_flag]);
    // lv_obj_set_style_local_bg_color(textarea, LV_TEXTAREA_PART_SCROLLBAR, LV_STATE_DEFAULT,  lv_color_hex(0x000000));
    // lv_obj_set_style_local_bg_opa(textarea, LV_TEXTAREA_PART_SCROLLBAR, LV_STATE_DEFAULT,LV_OPA_COVER);

    // lv_obj_set_style_local_pad_top(textarea, LV_TEXTAREA_PART_SCROLLBAR, LV_STATE_DEFAULT,10);
    // lv_obj_set_style_local_pad_bottom(textarea, LV_TEXTAREA_PART_SCROLLBAR, LV_STATE_DEFAULT,10);
    // lv_obj_set_style_local_pad_left(textarea, LV_TEXTAREA_PART_SCROLLBAR, LV_STATE_DEFAULT,10);
    // lv_obj_set_style_local_pad_right(textarea, LV_TEXTAREA_PART_SCROLLBAR, LV_STATE_DEFAULT,10);

    lv_obj_set_style_local_text_font(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FONT_SIZE_L(28));
    lv_obj_set_style_local_text_letter_space(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 5); // 字母间隔

    lv_obj_set_style_local_border_side(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
    lv_obj_set_style_local_border_color(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x4F4E4E));
    lv_obj_set_style_local_border_width(textarea, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_border_side(textarea, LV_TEXTAREA_PART_BG, LV_STATE_FOCUSED, LV_BORDER_SIDE_BOTTOM);
    lv_obj_set_style_local_border_color(textarea, LV_TEXTAREA_PART_BG, LV_STATE_FOCUSED, lv_color_hex(0x4F4E4E));
    lv_obj_set_style_local_border_width(textarea, LV_TEXTAREA_PART_BG, LV_STATE_FOCUSED, 2);

    lv_obj_set_style_local_text_color(textarea, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, lv_color_hex(0x424542));
    lv_obj_set_style_local_text_font(textarea, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, FONT_SIZE_L(28));
    lv_obj_set_style_local_pad_left(textarea, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, 40);
    lv_obj_set_style_local_text_letter_space(textarea, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, 2);

    return textarea;
}

void create_tuya_register_page(void)
{
    // 创建容器
    lv_obj_t *tuya_cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_id(tuya_cont, 11);
    set_location(tuya_cont, 0, 0, 1024, 600);
    lv_obj_set_style_local_bg_opa(tuya_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_bg_color(tuya_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

    lv_obj_t *ta = input_textarea_create1(tuya_cont, 1024, 160);
    lv_obj_set_id(ta, 11);

    keybord_create(tuya_cont, ta);

    // lv_obj_t *t_lable = lv_label_create(tuya_cont, NULL);
    // lv_obj_set_style_local_text_color(t_lable, LV_KEYBOARD_PART_BG, LV_STATE_DEFAULT,  lv_color_hex(0x4F4E4E));
    // lv_obj_set_style_local_text_font(t_lable, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, ));
    // lv_obj_set_id(t_lable,11);

    // lv_label_set_text(t_lable, src[text_flag]);
    // lv_obj_align(t_lable, kb, LV_ALIGN_OUT_TOP_MID, 0, -25);
}

static void LAYOUT_ENETER_FUNC(tuya_register)
{
    printf("======tuya register!!!!!\n");
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    system_bg_enable_set(false); // 系统背景使能设置
    backlight_open(true, false, user_data_get()->other.brightness);
    create_tuya_register_page();
}

static void LAYOUT_QUIT_FUNC(tuya_register)
{
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    system_bg_enable_set(true);

    standby_timer_open(-1, NULL);
}

CREATE_LAYOUT(tuya_register);