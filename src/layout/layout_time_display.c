/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-04-16 08:18:07
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-04-17 16:20:55
 * @FilePath: /two-wire-indoor/src/layout/layout_time_display.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <time.h>
#include "layout_define.h"

extern void gui_raw_clear(void);
extern bool backlight_status_get(void);
extern void screen_force_refresh(void);

#define TIME_DISPLAY_DURATION (2 * 60 * 1000)

static lv_task_t *time_display_task_t = NULL;
static lv_obj_t *label_time = NULL;
static lv_obj_t *label_date = NULL;

/* 更新时间的函数 */
void update_time_task(void)
{
    time_t now;
    struct tm *timeinfo;
    static bool colon_visible = true; /* 切换冒号的显示状态（每秒交替显示或隐藏） */
    char time_buffer[9];              /* 用于存储时间字符串，例如 "14 23 45" */
    char date_buffer[11];             /* 用于存储日期字符串，例如 "11.04.2024" */

    // 获取当前时间
    time(&now);
    timeinfo = localtime(&now);

    // 格式化时间为 "HH MM SS"
    strftime(time_buffer, sizeof(time_buffer), "%H %M", timeinfo);
    time_buffer[2] = ((colon_visible = !colon_visible)) ? ' ' : ':';
    // 将时间设置为 Label 的文本
    lv_label_set_text(label_time, time_buffer);
    lv_obj_align(label_time, NULL, LV_ALIGN_CENTER, 0, -30); // 将 Label 居中

    if (user_data_get()->other.date_format == 0)
    {
        strftime(date_buffer, sizeof(date_buffer), "%Y.%m.%d", timeinfo);
    }
    else if (user_data_get()->other.date_format == 1)
    {
        strftime(date_buffer, sizeof(date_buffer), "%m.%d.%Y", timeinfo);
    }
    else if (user_data_get()->other.date_format == 2)
    {
        strftime(date_buffer, sizeof(date_buffer), "%d.%m.%Y", timeinfo);
    }
    // 将日期设置为日期 Label 的文本
    char Data[128] = {0};
    sprintf(Data, "%s %s", timeinfo->tm_wday == 7 ? text_str(STR_WEEK_SUN) : text_str(STR_WEEK_SUN + timeinfo->tm_wday), date_buffer);
    lv_label_set_text(label_date, Data);
    lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 100); // 将 Label 居中

    lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE_L(160));
    lv_obj_set_style_local_text_font(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE_L(60));
}

static void time_display_click_up(lv_obj_t *obj)
{
    Debug("\n\n\n");
    goto_layout(pLAYOUT(standby));
}

static void update_time_create(void)
{

    label_time = lv_label_create(lv_scr_act(), NULL);

    label_date = lv_label_create(lv_scr_act(), NULL);
    // 初始化时间显示
    update_time_task();

    lv_obj_t *obj = lv_scr_act();
    static btn_data btn_data = btn_data_up_create(time_display_click_up);
    obj->user_data = &btn_data;
    btn_touch_event_listen(obj);
    lv_obj_set_click(obj, true);
    lv_obj_set_style_local_bg_opa(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}

static void time_display_task(lv_task_t *task_t)
{

    struct timespec start_time = *((struct timespec *)(task_t->user_data));
    struct timespec curr_time;
    clock_gettime(CLOCK_MONOTONIC, &curr_time);
    unsigned long long diff = (curr_time.tv_sec - start_time.tv_sec) * 1000 + (curr_time.tv_nsec - start_time.tv_nsec) / 1000000;

    if (diff >= 1000)
    {
        if (backlight_status_get() == false)
        {
            backlight_open(true, false, user_data_get()->other.brightness);
        }
    }
    if (diff >= TIME_DISPLAY_DURATION)
    {
        if (time_display_task_t != NULL)
        {
            lv_task_del(time_display_task_t);
        }

        if (backlight_status_get() == true)
        {
            gui_raw_clear();
            screen_force_refresh();
            backlight_open(false, false, user_data_get()->other.brightness);
        }
    }
    else
    {
        update_time_task();
    }
}
static void LAYOUT_ENETER_FUNC(time_display)
{
    Debug("\n\n\n");
    static struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    update_time_create();
    sdcard_event_register(NULL);
    system_bg_fill_color(0x00, 0, 0, 1024, 600);
    backlight_open(false, false, user_data_get()->other.brightness);
    if (time_display_task_t != NULL)
    {
        lv_task_del(time_display_task_t);
    }
    time_display_task_t = lv_task_create(time_display_task, 500, LV_TASK_PRIO_HIGH, &time);
}

static void LAYOUT_QUIT_FUNC(time_display)
{
    if (time_display_task_t != NULL)
    {
        lv_task_del(time_display_task_t);
        time_display_task_t = NULL;
    }
}

CREATE_LAYOUT(time_display);