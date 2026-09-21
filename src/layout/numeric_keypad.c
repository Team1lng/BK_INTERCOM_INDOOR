#include <stdio.h>
#include <stdlib.h>
#include "../lvgl/lvgl.h"
#include "numeric_keypad.h"
#include "layout_define.h"

#define KEY_PRESS_COLOR LV_COLOR_RED

// 创建父容器
lv_obj_t *keyboard_container;
lv_obj_t *text_area;

// 数字密码
struct
{
    int password;
    int passw_len;
} keypad_attr = {.password = 0, .passw_len = 0};

void (*keyboard_confirm_event_cb)(int password, int len) = NULL;
// 确认按钮回调函数
static void confirm_btn_event_cb(lv_obj_t *btn)
{
    lv_obj_del_reload(&keyboard_container);
    if (keyboard_confirm_event_cb != NULL)
    {
        keyboard_confirm_event_cb(keypad_attr.password, keypad_attr.passw_len);
    }
}

// 删除按钮回调函数
static void delete_btn_event_cb(lv_obj_t *btn)
{
    keypad_attr.password /= 10;
    if (keypad_attr.passw_len > 0)
    {
        keypad_attr.passw_len--;
        if (keypad_attr.passw_len == 0)
        {
            lv_textarea_set_text(text_area, "");
        }
        else
        {
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%d", keypad_attr.password);
            lv_textarea_set_text(text_area, buffer);
        }
    }
    else
    {
        lv_obj_del_reload(&keyboard_container);
    }
}

// 按键回调函数
void key_btn_event_cb(lv_obj_t *btn)
{
    if (keypad_attr.passw_len < KEYBOARD_PASSWORD_LEN)
    {
        keypad_attr.passw_len++;
        lv_obj_t *label = lv_obj_get_child(btn, NULL);
        const char *btn_label = lv_label_get_text(label);
        int key_num = atoi(btn_label);
        keypad_attr.password = keypad_attr.password * 10 + key_num;

        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%d", keypad_attr.password);
        lv_textarea_set_text(text_area, buffer);
    }
}

#ifdef MEIOU_SERSION
#define KEY_COLOR LV_COLOR_MAKE(0x20, 0x20, 0x20)
#define KEYCONT_COLOR LV_COLOR_MAKE(0x12, 0x11, 0x13)
#else
#define KEY_COLOR LV_COLOR_MAKE(0x00, 0x25, 0x38)
#define KEYCONT_COLOR LV_COLOR_MAKE(0x00, 0x13, 0x1D)
#endif

static void key_create(void)
{
    // 创建数字按键
    lv_obj_t *keys[12];
    static btn_data btn_data[12] = {{0}};
    char *key_labels[12] = {
        "1", "2", "3",
        "4", "5", "6",
        "7", "8", "9",
        "×", "0", "√"};

    lv_obj_t *key_container = lv_cont_create(keyboard_container, NULL);
    lv_obj_set_size(key_container, 326, 328);
    lv_obj_align(key_container, NULL, LV_ALIGN_IN_TOP_MID, 0, 121);

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            keys[row * 3 + col] = lv_btn_create(key_container, NULL);

            lv_obj_set_size(keys[row * 3 + col], 96, 70);
            lv_obj_set_pos(keys[row * 3 + col], col * 115, row * 86);
            lv_obj_set_style_local_bg_color(keys[row * 3 + col], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, KEY_COLOR);
            lv_obj_set_style_local_bg_color(keys[row * 3 + col], LV_OBJ_PART_MAIN, LV_STATE_PRESSED, KEY_PRESS_COLOR);
            // lv_obj_set_event_cb(keys[row * 3 + col], row*3+col == 9 ? delete_btn_event_cb : row*3+col == 11 ? confirm_btn_event_cb : key_btn_event_cb);

            btn_data[row * 3 + col].obj_tone = true;
            btn_data[row * 3 + col].OPS_UP = row * 3 + col == 9 ? delete_btn_event_cb : row * 3 + col == 11 ? confirm_btn_event_cb
                                                                                                            : key_btn_event_cb;
            keys[row * 3 + col]->user_data = &btn_data[row * 3 + col];
            btn_touch_event_listen(keys[row * 3 + col]);

            lv_obj_t *key_label = lv_label_create(keys[row * 3 + col], NULL);
            lv_label_set_text(key_label, key_labels[row * 3 + col]);
        }
    }
}

static void msgbox_event_cb(lv_obj_t *obj, lv_event_t event)
{
    lv_obj_t *main_cont = (lv_obj_t *)(obj->user_data);
    if (event == LV_EVENT_DELETE)
    {
        if (main_cont)
            lv_obj_del(main_cont);
    }
}
// 初始化键盘界面
void init_numeric_keyboard(void (*confirm_cb)(int password, int len))
{
    // 初始化参数
    keypad_attr.password = 0;
    keypad_attr.passw_len = 0;
    keyboard_confirm_event_cb = confirm_cb;

    lv_obj_t *main_cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(main_cont, 1024, 600);
    lv_obj_set_pos(main_cont, 0, 0);

    // 创建父容器
    keyboard_container = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(keyboard_container, 408, 484);
    lv_obj_align(keyboard_container, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_opa(keyboard_container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(keyboard_container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, KEYCONT_COLOR);
    lv_obj_set_style_local_radius(keyboard_container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_event_cb(keyboard_container, msgbox_event_cb);
    keyboard_container->user_data = main_cont;

    // // 创建文本显示区域
    text_area = lv_textarea_create(keyboard_container, NULL);
    lv_obj_set_size(text_area, 300, 60);
    lv_obj_align(text_area, NULL, LV_ALIGN_IN_TOP_MID, 0, 28);
    lv_textarea_set_text(text_area, "");
    lv_textarea_set_placeholder_text(text_area, text_str(STR_PLEASE_INPUT_PASSWORD));
    lv_obj_set_style_local_bg_opa(text_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_color(text_area, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_text_color(text_area, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_text_color(text_area, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_text_color(text_area, LV_TEXTAREA_PART_PLACEHOLDER, LV_STATE_FOCUSED, LV_COLOR_SILVER);
    lv_textarea_set_text_align(text_area, LV_LABEL_ALIGN_CENTER);
    lv_textarea_set_cursor_hidden(text_area, true);

    // 创建数字键按键
    key_create();
}
