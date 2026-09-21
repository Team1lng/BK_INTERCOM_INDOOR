/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-01-02 08:35:23
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-05-21 09:59:00
 * @FilePath: /two-wire-indoor/src/layout/layout_monitor2.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifdef INTERCOM_VERSION

#include "layout_define.h"
#include "leo_api.h"

#define RING_CLOSE_TIME 5
#define MONITOR_DURATION 30
#define MONITOR_LAYER1_COORDINATE_INIT { \
    {262, 483, 100, 100},                \
    {462, 483, 100, 100},                \
    {662, 483, 100, 100},                \
};
#define MONITOR_LAYER2_COORDINATE_INIT { \
    {372, 483, 100, 100},                \
    {552, 483, 100, 100},                \
    {900, 120, 100, 100},                \
    {900, 220, 100, 100},                \
    {900, 320, 100, 100},                \
    {900, 420, 100, 100},                \
};
#define MONITOR_CCTV_LAYER_COORDINATE_INIT { \
    {262, 483, 100, 100},                    \
    {392, 483, 100, 100},                    \
    {522, 483, 100, 100},                    \
    {662, 483, 100, 100},                    \
};

typedef enum monitor_btn_list
{
    NONE_BTN,
    LOCK_BTN,
    HANDUP_BTN,
    ANSWER_BTN,
    SWITCH_BTN,
    SANP_BTN,
    SET_BTN,
    GATE_BTN,
    VIDEO_BTN,
    SETTING_BTN,
    TOTAL_BTN
} monitor_btn_list;

static void (*monitor_door_layer_ptr)(void) = NULL;
static MONITOR_CH montior_call_ch = MON_CH_NONE;
static int monitor_wait_time = 0;
static int monitor_ring_close_time = 0;
static int monitor_timeout = MONITOR_DURATION;
static bool monitor_setting_window_flag;
static lv_task_t *monitor_chime_ptask = NULL;
static lv_task_t *auto_shoot_task_t = NULL;
static lv_task_t *unlock_task_t = NULL;
static lv_task_t *ungate_task_t = NULL;
static lv_task_t *montior_talk_task_t = NULL;
static lv_task_t *monitor_timer_task_p = NULL;
static lv_obj_t *time_label = NULL;
static lv_obj_t *channel_label = NULL;
static lv_obj_t *countdown_label = NULL;
static void monitor_channel_display(void);
static void monitor_all_btn_destroy(void);
static void monitor_door_layer1_btn_create(void);
static void monitor_door_layer2_btn_create(void);
static void monitor_cctv_layer_btn_create(void);
extern bool get_video_data_display_state(void);

static void monitor_timer_set(__int32_t time)
{
    monitor_timeout = time == 0 ? MONITOR_DURATION : time;
}

static int get_monitor_brightness(void)
{
    switch (monitor_channel_get())
    {
    case MON_CH_DOOR_1:

        return user_data_get()->door1.brightness;
    case MON_CH_DOOR_2:

        return user_data_get()->door2.brightness;
    case MON_CH_CCTV_1:

        return user_data_get()->camera1.brightness;
    case MON_CH_CCTV_2:

        return user_data_get()->camera2.brightness;

    default:
        return user_data_get()->other.brightness;
    }
}

static void monitor_indoor_cmd_func(unsigned long arg1, unsigned long arg2)
{
    char cmd = (arg1 >> 8) & 0xFF;
    network_device out_device = arg2 & 0xFF;

    MONITOR_CH ch = monitor_channel_get();
    MONITOR_CH out_ch = out_device == DEVICE_OUTDOOR_1 ? MON_CH_DOOR_1 : MON_CH_DOOR_2;

    printf("======net_common_outdoor_hand_func= %d ,%d======>>>\n\n", cmd, out_ch);
    fflush(stdout);
    switch (cmd)
    {
    case 1: /* in device 与 out_device 通话 */
        if (((out_ch != ch)) || monitor_enter_way_get() == MONITOR_ENTER_TUYA)
        {
            /*不做任何处理*/
        }
        else
        {
            goto_layout(pLAYOUT(standby));
        }
        break;
    default:
        printf("Parameter error :%d\n\r", cmd);
        break;
    }
}

extern void tuya_switch_camera(char channel);

static void tuya_event_extern_proc(unsigned long arg1, unsigned long arg2)
{
    tuya_event ev = (tuya_event)arg1;
    switch (ev)
    {
    /*切换监控*/
    case TUYA_EVENT_MONITOR_SWAP:
        // tuya_ipc_ring_buffer_video_release_data();
        // tuya_switch_camera(arg2);
        //  tuya_ipc_ring_buffer_video_release_data();
        break;
        /*开锁*/
    case TUYA_EVENT_OPEN_LOCK:
        // start_door_unlock();
        break;
    case TUYA_EVENT_WORK_MODE:
        user_data_get()->other.model = arg2 == 0 ? 2 : arg2 - 1;
        if (current_layout_get() == &layout_home)
        {
            extern void home_Model_btn_switch(void);
            home_Model_btn_switch();
        }
        Debug("TUYA_EVENT_WORK_MODE ====================+>>>>%d\n\n\n", user_data_get()->other.model);
        break;
        /*通话*/
    case TUYA_EVENT_TALK:
        send_monitor_talk_cmd(true);
        break;
        /*进入监控*/
    case TUYA_EVENT_MONITOR_ENTER:
    {
        monitor_channel_set(MON_CH_DOOR_1);
        for (int ch = DEVICE_OUTDOOR_1; ch < DEVICE_END; ch++)
        {
            if (device_online_state_get(ch))
            {
                monitor_channel_set(MON_CH_DOOR_1 + ch - DEVICE_OUTDOOR_1);
                break;
            }
        }
        Debug("TUYA_EVENT_MONITOR_ENTER ====================+>>>>%d\n\n\n", monitor_channel_get());
        tuya_channel_valid_report();
        monitor_enter_way_set(MONITOR_ENTER_TUYA);
        goto_layout(pLAYOUT(monitor));

        monitor_all_btn_destroy();
        MONITOR_CH monitor_ch = monitor_channel_get();
        Debug("TUYA_EVENT_MONITOR_ENTER ====================+>>>>%d\n\n\n", monitor_ch);
        network_device devcie = monitor_ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
        audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
        audio_talk_open(ctrl);

        fb_video_mode_enable(false);
        // send_monitor_talk_cmd(true);
    }
    break;
        /*退出监控*/
    case TUYA_EVENT_MONITOR_QUIT:
        /*Don't do anything*/
        break;
    /* 截屏 */
    case TUYA_EVENT_SCREENSHOT:
    {
        extern int tuya_dp_236_response_screenshot(BOOL_T state);
        extern void screen_capture();
        screen_capture();
        tuya_dp_236_response_screenshot(false);
    }
    case TUYA_EVENT_MONITOR_ING:
    {
        if (current_layout_get() != &layout_standby && monitor_enter_way_get() != MONITOR_ENTER_TUYA)
        {
            goto_layout(pLAYOUT(standby));
        }
    }
    break;
    default:
        /*Don't do anything*/
        break;
    }
    return;
}

static void tuya_event_inside_proc(unsigned long arg1, unsigned long arg2)
{
    tuya_event ev = (tuya_event)arg1;
    switch (ev)
    {
    /*切换监控*/
    case TUYA_EVENT_MONITOR_SWAP:
    {
        tuya_switch_camera(arg2);
        break;
    }
        /*开锁*/
    case TUYA_EVENT_OPEN_LOCK:
    {
        tuya_unlock_start();
        break;
    }

    /* 开锁2*/
    case TUYA_EVENT_OPEN_GATE1:
    {
        if (user_data_get()->tuya_info.lock_id == false)
        {
            tuya_ungate1_start();
        }
        else
        {
            tuya_ungate2_start();
        }
        break;
    }
    case TUYA_EVENT_OPEN_GATE2:
    {
        tuya_ungate2_start();
        break;
    }
        /*通话*/
    case TUYA_EVENT_TALK:
        if (arg2 == true)
        {
            Debug("TUYA_EVENT_TALK:%d\n", monitor_channel_get());
            MONITOR_CH monitor_ch = monitor_channel_get();
            if (monitor_ch < MON_CH_CCTV_1)
            {
                network_device devcie = monitor_ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
                audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
                audio_talk_open(ctrl);
                send_monitor_talk_cmd(true);
            }
        }
        break;
        /*进入监控*/
    case TUYA_EVENT_MONITOR_ENTER:
        /*Don't do anything*/
        {
            if (device_online_state_get(monitor_channel_get() + DEVICE_INDOOR_ID6) == false)
            {
                audio_talk_close(false);
                for (int ch = DEVICE_OUTDOOR_1; ch < DEVICE_END; ch++)
                {
                    if (device_online_state_get(ch))
                    {

                        monitor_channel_set(MON_CH_DOOR_1 + ch - DEVICE_OUTDOOR_1);
                        break;
                    }
                }
                monitor_switch(); // monitor_open(true);//
            }

            monitor_enter_way_set(MONITOR_ENTER_TUYA);
            record_video_stop(0x00);
            tuya_channel_valid_report();
            monitor_all_btn_destroy();

            MONITOR_CH monitor_ch = monitor_channel_get();
            network_device devcie = monitor_ch + DEVICE_INDOOR_ID6;
            if (monitor_ch != MON_CH_CCTV_1 && monitor_ch != MON_CH_CCTV_2)
            {
                audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
                audio_talk_open(ctrl);
            }
            audio_play_stop_set();
            while (video_decode_data_status() != true)
            {
                if (get_video_decode_state() == false)
                    break;
            };
            video_raw_clear();
            fb_video_mode_enable(false);
        }
        break;
        /*退出监控*/
    case TUYA_EVENT_MONITOR_QUIT:
    {
        send_monitor_hang_cmd();
        monitor_channel_set(MON_CH_NONE);
        goto_layout(pLAYOUT(standby));
    }
    break;
    /* 截屏 */
    case TUYA_EVENT_SCREENSHOT:
    {
        extern int tuya_dp_236_response_screenshot(BOOL_T state);
        extern void screen_capture();
        screen_capture();
        tuya_dp_236_response_screenshot(false);
    }
    break;

    case TUYA_EVENT_MONITOR_ING:
    {
        if (current_layout_get() != &layout_standby && monitor_enter_way_get() != MONITOR_ENTER_TUYA)
        {
            goto_layout(pLAYOUT(standby));
        }
    }
    break;
    default:
        /*Don't do anything*/
        break;
    }
    return;
}

static void monitor_chime_task(struct _lv_task_t *task_t)
{
    if (monitor_chime_ptask != NULL)
    {
        lv_task_del(monitor_chime_ptask);
        monitor_chime_ptask = NULL;
    }
    chime_gpio_disable();
}

static void monitor_auto_shoot_task(struct _lv_task_t *task_t)
{
    if (get_video_data_display_state())
    {
        door_info *door = monitor_channel_get() == MON_CH_DOOR_1 ? &user_data_get()->door1 : &user_data_get()->door2;
        if (door->record_mode)
        {
            if (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED)
            {
                extern bool send_tuya_record(char record_mode);
                send_tuya_record(REC_MODE_TUYA);
            }
        }
        else if (is_sdcard_insert())
        {
            int free_space = sd_free_space_insufficient();
            if (free_space < 500)
            {
                if (sdcard_insert_msg_box == NULL)
                {
                    sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
                }
            }
            if (free_space > 200)
            {
                extern void jpg_push_to_tuya(int type);
                jpg_push_to_tuya(1);
                record_pictrue_start(REC_MODE_AUTO, monitor_channel_get());
            }
            else if (free_space < 200)
            {
                extern void detect_sd_free_space(void);
                detect_sd_free_space();
            }
        }
        if (auto_shoot_task_t != NULL)
        {
            lv_task_del(auto_shoot_task_t);
            auto_shoot_task_t = NULL;
        }
    }
}

static void monitor_call_ring_play_finsih_callback(void)
{
    printf("%s==================>>>>>>>>>>>>>>\n\r", __func__);
    if ((monitor_ring_close_time) && is_audio_play_ing() == false)
    {
        door_ring_info *door = monitor_channel_get() == MON_CH_DOOR_1 ? &ring_attr.door1 : &ring_attr.door2;
        {
            if (door->ring_mode)
            {
                media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, door->custom_ring);
                custom_music_play(info->file_name, door->ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_ring_play_finsih_callback);
            }
            else
                door_ring_play(door->ring, door->ring_val, user_data_get()->audio.ringback && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_ring_play_finsih_callback);
        }
    }
}

static void monitor_call_inside_func(unsigned long arg1, unsigned long arg2)
{
    if (monitor_enter_way_get() == MONITOR_ENTER_TUYA)
    {
        return;
    }
    if (arg1 == DEVICE_OUTDOOR_2 && user_data_get()->door2.enable_sw == false)
    {
        return;
    }
    // if (monitor_enter_way_get() == MONITOR_ENTER_CALL)
    // {
    //     return;
    // }

    MONITOR_CH curr_ch = monitor_channel_get();
    MONITOR_CH call_ch = arg1 == DEVICE_OUTDOOR_1 ? MON_CH_DOOR_1 : MON_CH_DOOR_2;

    // Debug("%d,%d\n\n\n", monitor_channel_get(), call_ch);
    if (curr_ch != call_ch)
    {
        if (user_data_get()->other.model != MUTE_PATTERN)
        {
            chime_gpio_enable();
            if (monitor_chime_ptask == NULL)
            {
                monitor_chime_ptask = lv_task_create(monitor_chime_task, 5000, LV_TASK_PRIO_HIGH, NULL);
            }
        }
        montior_call_ch = call_ch;
        monitor_enter_way_set(MONITOR_ENTER_CALL);
    }

    if (!is_sdcard_insert())
    {
        if (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED)
        {
            extern bool send_tuya_record(char record_mode);
            send_tuya_record(REC_MODE_TUYA);
        }
    }
}

static void monitor_call_extern_action(unsigned long arg1, unsigned long arg2)
{
    door_ring_info *door_ring = arg1 == DEVICE_OUTDOOR_1 ? &ring_attr.door1 : &ring_attr.door2;
    Debug("%d\n\n\n\n", door_ring->ring_val);
    door_info *door = arg1 == DEVICE_OUTDOOR_1 ? &user_data_get()->door1 : &user_data_get()->door2;
    if (user_data_get()->other.model == AT_HOME_PATTERN)
    {
        if (door_ring->ring_mode)
        {
            media_info *info = media_info_get(FILE_TYPE_SD_MUSIC, door_ring->custom_ring);
            custom_music_play(info->file_name, door_ring->ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_ring_play_finsih_callback);
        }
        else
        {
            door_ring_play(door_ring->ring, door_ring->ring_val, user_data_get()->audio.ringback && !(arg2 & CALL_REINGBACK_DISABLE) && (monitor_enter_way_get() != MONITOR_ENTER_ALARM), NULL, monitor_call_ring_play_finsih_callback);
        }
        monitor_wait_time = door_ring->ring_time;
        monitor_ring_close_time = RING_CLOSE_TIME;
    }

    {
        monitor_wait_time = door_ring->ring_time;
        if (door->record_mode)
        {
            Debug("\n\n\n\n");
            if (is_sdcard_insert())
            {
                Debug("\n\n\n\n");
                int free_space = sd_free_space_insufficient();
                if (free_space < 500)
                {
                    if (sdcard_insert_msg_box == NULL)
                    {
                        sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
                    }
                }
                if (free_space > 200)
                {
                    record_video_start(user_data_get()->other.model == NOT_AT_HOME_PATTERN ? REC_MODE_MESSAGE : REC_MODE_AUTO, true, monitor_channel_get());
                }
                else if (free_space < 200)
                {
                    extern void detect_sd_free_space(void);
                    detect_sd_free_space();
                }
            }
        }
    }
}

static void monitor_call_extern_func(unsigned long arg1, unsigned long arg2)
{
    extern bool get_outdoor_talk_state(MONITOR_CH ch);
    if (format_sd_card_status() ||
        tuya_monitor_state_get() ||
        get_outdoor_talk_state(MON_CH_DOOR_1) ||
        get_outdoor_talk_state(MON_CH_DOOR_2) ||
        monitor_enter_way_get() == MONITOR_ENTER_TUYA)
    {
        Debug("outdoor tlaking...,return %d %d %d %d\n\r", get_outdoor_talk_state(MON_CH_DOOR_1), get_outdoor_talk_state(MON_CH_DOOR_2), tuya_monitor_state_get(), monitor_enter_way_get());
        return;
    }
    network_device device = (network_device)arg1; // 确定呼叫的门口机设备
    if (device == DEVICE_OUTDOOR_2 && user_data_get()->door2.enable_sw == false)
    {
        return;
    }
    monitor_channel_set(device == DEVICE_OUTDOOR_1 ? MON_CH_DOOR_1 : MON_CH_DOOR_2); // 选择通道
    monitor_enter_way_set(MONITOR_ENTER_CALL);
    goto_layout(pLAYOUT(monitor));

    if (user_data_get()->other.model != MUTE_PATTERN)
    {
        chime_gpio_enable();
        if (monitor_chime_ptask == NULL)
        {
            monitor_chime_ptask = lv_task_create(monitor_chime_task, 5000, LV_TASK_PRIO_HIGH, NULL);
        }
    }
    monitor_call_extern_action(arg1, arg2);

    if (auto_shoot_task_t == NULL)
    {
        auto_shoot_task_t = lv_task_create(monitor_auto_shoot_task, 500, LV_TASK_PRIO_HIGH, NULL);
    }
    network_device devcie = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
    audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, monitor_channel_get() == MON_CH_DOOR_1 ? ring_attr.door1.ring_val * 5 + 46 : ring_attr.door2.ring_val * 5 + 46};
    audio_talk_open(ctrl);
}

static void monitor_time_display(void)
{
    struct ak_date date;
    static bool colon = false;
    ak_get_localdate(&date);
    lv_obj_set_style_local_text_font(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
    lv_label_set_text_fmt(time_label, "%02d/%02d  %02d%s%02d", date.month + 1, date.day + 1, date.hour, (colon = !colon) ? ":" : " ", date.minute);
    lv_obj_align(time_label, time_label->parent, LV_ALIGN_CENTER, -150, 0);
}

static void monitor_channel_display(void)
{
    lv_label_set_text(channel_label, text_str(STR_DOOR1 + monitor_channel_get() - MON_CH_DOOR_1));
}

static void monitor_countdown_display(void)
{
    static char *str = NULL;
    if (monitor_enter_way_get() == MONITOR_ENTER_TUYA)
    {
        lv_label_set_text_fmt(countdown_label, "%s", text_str(STR_APP_PREVIEW));
        lv_obj_align(countdown_label, countdown_label->parent, LV_ALIGN_IN_TOP_RIGHT, -40, 20);
        return;
    }
    else if (is_jpg_record_ing())
    {
        str = text_str(STR_SNAPSHOT);
    }
    else if (monitor_enter_way_get() == MONITOR_ENTER_CALL && montior_call_ch != MON_CH_NONE)
    {
        str = (montior_call_ch - 1) ? text_str(STR_DOOR2_CALL) : text_str(STR_DOOR1_CALL);
    }
    else if (is_video_recording() && record_video_type() == REC_MODE_MESSAGE)
    {
        str = text_str(STR_MESSAGE);
    }
    else if (is_video_recording())
    {
        str = text_str(STR_REC);
    }
    else
    {
        str = "";
    }
    lv_label_set_text_fmt(countdown_label, "%s %02d", str, monitor_timeout < 0 ? 0 : monitor_timeout);
    lv_obj_align(countdown_label, countdown_label->parent, LV_ALIGN_IN_TOP_RIGHT, -40, 20);
}

static void monitor_infobar_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_cont_set_fit(cont, LV_FIT_NONE);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_size(cont, 1024, 60);
    lv_obj_set_style_local_bg_opa(cont, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
    lv_obj_set_style_local_bg_color(cont, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
    lv_obj_set_auto_realign(cont, true);

    time_label = lv_label_create(cont, NULL);
    lv_label_set_long_mode(time_label, LV_LABEL_LONG_EXPAND);
    lv_label_set_align(time_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(time_label, cont, LV_ALIGN_CENTER, 0, 0);
    monitor_time_display();

    channel_label = lv_label_create(cont, NULL);
    lv_obj_set_size(channel_label, 120, 60);
    lv_label_set_align(channel_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_text_font(channel_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
    lv_obj_align(channel_label, cont, LV_ALIGN_IN_LEFT_MID, 30, 0);
    monitor_channel_display();

    countdown_label = lv_label_create(cont, NULL);
    lv_label_set_align(countdown_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(countdown_label, LV_LABEL_LONG_EXPAND);
    lv_obj_set_style_local_text_color(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xdd3d3d));
    lv_obj_set_style_local_text_font(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(28));
    lv_obj_align(countdown_label, cont, LV_ALIGN_IN_TOP_RIGHT, -40, 20);
    monitor_countdown_display();
    return;
}

static void monitor_click_up(lv_obj_t *obj)
{
    if (monitor_setting_window_flag)
    {
        lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
        if (window_cont != NULL)
        {
            lv_obj_del(window_cont);
            monitor_setting_window_flag = 0;
        }
    }
}

static void monitor_lock_task(lv_task_t *task_t)
{
    if (unlock_task_t)
    {
        lv_task_del(unlock_task_t);
        unlock_task_t = NULL;
    }
    if (monitor_channel_get() > MON_CH_DOOR_2)
    {
        return;
    }
    lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), LOCK_BTN);
    if (obj)
    {
        static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LOCK1_FOCUS_PNG);
        static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_LOCK1_PNG);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
        lv_obj_clear_state(obj, LV_STATE_FOCUSED);
    }
    tuya_dp_148_response_accessory_lock(false);
}
static void monitor_lock_btn_up(lv_obj_t *obj)
{
    monitor_click_up(NULL);
    if (unlock_task_t == NULL)
    {
        audio_play_stop_set();
        static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK1_FOCUS_PNG);
        static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK1_PNG);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
        lv_obj_add_state(obj, LV_STATE_FOCUSED);

        network_cmd_data data;
        data.device = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : monitor_channel_get() == MON_CH_DOOR_2 ? DEVICE_OUTDOOR_2
                                                                                                                         : DEVICE_UNKONW;
        data.cmd = NET_COMMON_CMD_UNLOCK;
        data.arg1 = monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.unlock_delay : user_data_get()->door1.unlock_delay;
        data.arg2 = 1 | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
        network_send_cmd_data(&data);
        tuya_dp_148_response_accessory_lock(true);
        unlock_task_t = lv_task_create(monitor_lock_task, (monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.unlock_delay : user_data_get()->door1.unlock_delay) * 1000, LV_TASK_PRIO_HIGH, NULL);
    }
}
static void monitor_lock_btn_create(Controls_location **coordinate)
{
    static btn_data btn_data = btn_data_create(NULL, monitor_lock_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_LOCK1_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_LOCK1_FOCUS_PNG);
    static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK1_FOCUS_PNG);
    static rom_bin_info info3 = rom_bin_info_get(ROM_RES_MONITOR_UNLOCK1_PNG);
    lv_obj_t *btn = home_btn_create_1(**coordinate, NULL, &btn_data, unlock_task_t ? &info3 : &info, unlock_task_t ? &info2 : &info1);
    lv_obj_set_id(btn, LOCK_BTN);
    (*coordinate)++;
}

static void monitor_handup_btn_up(lv_obj_t *obj)
{
    send_monitor_hang_cmd();
    outdoor_order_set(NET_COMMON_CMD_NONE);
    goto_layout(pLAYOUT(standby)); // 页面跳转
}
static void monitor_handup_btn_create(Controls_location **coordinate)
{
    static btn_data btn_data = btn_data_create(NULL, monitor_handup_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_HANDUP_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_HANDUP_FOCUS_PNG);
    static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_HOLDUP_UNFOCUS_PNG);
    static rom_bin_info info3 = rom_bin_info_get(ROM_RES_MONITOR_HOLDUP_FOCUS_PNG);
    rom_bin_info *info_ptr1 = monitor_channel_get() < MON_CH_CCTV_1 ? &info : &info2;
    rom_bin_info *info_ptr2 = monitor_channel_get() < MON_CH_CCTV_1 ? &info1 : &info3;
    lv_obj_t *btn = home_btn_create_1(**coordinate, NULL, &btn_data, info_ptr1, info_ptr2);
    lv_obj_set_id(btn, HANDUP_BTN);
    (*coordinate)++;
}

static void monitor_answer_task(lv_task_t *task)
{
    MONITOR_CH monitor_ch = monitor_channel_get();

    monitor_click_up(NULL);
    int vol = monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume : user_data_get()->door2.talk_volume;

    {
        network_device devcie = monitor_ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
        audio_talk_ctrl ctrl = {{devcie}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN) | OPERATION_OPTION(AUDIO_OUT_EN) | OPERATION_OPTION(AUDIO_IN_EN)), AI_AO_O, true, true, vol * 5 + 46};
        audio_talk_open(ctrl);
        send_monitor_talk_cmd(true);
        audio_output_volume_set(vol * 5 + 46);
    }

    if (montior_talk_task_t != NULL)
    {
        lv_task_del(montior_talk_task_t);
        montior_talk_task_t = NULL;
    }
}
static void monitor_answer_btn_up(lv_obj_t *obj)
{
    monitor_timer_set(MONITOR_DURATION * 4);
    monitor_door_layer2_btn_create();
    audio_play_stop_set();
    outdoor_order_set(NET_COMMON_PARAM_CAMERA_TALK);
    send_monitor_talk_cmd(true);
    extern void main_device_monitor_busy_func(unsigned long arg1, unsigned long arg2);
    device_monitor_busy_register(NULL);

    montior_talk_task_t = lv_task_create(monitor_answer_task, 100, LV_TASK_PRIO_MID, NULL);

    monitor_door_layer_ptr = monitor_door_layer2_btn_create;
}
static void monitor_answer_btn_create(Controls_location **coordinate)
{
    static btn_data btn_data = btn_data_create(NULL, monitor_answer_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_ANSWER_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_ANSWER_FOCUS_PNG);
    lv_obj_t *btn = home_btn_create_1(**coordinate, NULL, &btn_data, &info, &info1);
    lv_obj_set_id(btn, ANSWER_BTN);
    (*coordinate)++;
}

static void monitor_switch_btn_up(lv_obj_t *obj)
{

    void (*monitor_layer_ptr)(void) = monitor_door_layer_ptr;
    static struct ak_timeval tv1 = {0}, tv2;
    ak_get_ostime(&tv2);
    if (tv2.sec == tv1.sec)
    {
        return;
    }
    tv1 = tv2;

    record_video_stop(0x00);
    monitor_click_up(NULL);
    monitor_enter_way_set(MONITOR_ENTER_MANUAL);
    MONITOR_CH ch = monitor_channel_get();
    if (ch == MON_CH_DOOR_1)
    {
        ch = user_data_get()->camera1.enable && user_data_get()->camera1.url[0] == 'r' ? MON_CH_CCTV_1 : (user_data_get()->door2.enable_sw ? MON_CH_DOOR_2 : user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2
                                                                                                                                                                                                                                       : MON_CH_DOOR_1);
    }
    else if (ch == MON_CH_CCTV_1)
    {
        ch = user_data_get()->door2.enable_sw ? MON_CH_DOOR_2 : (user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2 : MON_CH_DOOR_1);
    }
    else if (ch == MON_CH_DOOR_2)
    {
        ch = user_data_get()->camera2.enable && user_data_get()->camera2.url[0] == 'r' ? MON_CH_CCTV_2 : MON_CH_DOOR_1;
    }
    else if (ch == MON_CH_CCTV_2)
    {
        ch = MON_CH_DOOR_1;
    }

    if (monitor_channel_get() < MON_CH_CCTV_1 && ch >= MON_CH_CCTV_1)
    {
        monitor_layer_ptr = monitor_cctv_layer_btn_create;
    }
    else if (monitor_channel_get() >= MON_CH_CCTV_1 && ch < MON_CH_CCTV_1)
    {
        monitor_layer_ptr = monitor_door_layer_ptr;
    }
    audio_talk_close(false);
    monitor_channel_set(ch);
    monitor_switch();
    backlight_open(true, true, get_monitor_brightness());
    outdoor_order_set(NET_COMMON_CMD_NONE);
    monitor_channel_display();
    monitor_layer_ptr();

    if (monitor_layer_ptr == monitor_door_layer2_btn_create)
    {
        audio_talk_ctrl ctrl = {{ch + DEVICE_INDOOR_ID6}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN) | OPERATION_OPTION(AUDIO_OUT_EN) | OPERATION_OPTION(AUDIO_IN_EN)), AI_AO_O, true, true, ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
        audio_talk_open(ctrl);
        send_monitor_talk_cmd(true);
    }
    else
    {
        audio_talk_ctrl ctrl = {{ch + DEVICE_INDOOR_ID6}, (OPERATION_OPTION(AUDIO_SEND_EN) | OPERATION_OPTION(AUDIO_RECEIVE_EN)), AI_AO_C, true, false, ch == MON_CH_DOOR_1 ? user_data_get()->door1.talk_volume * 5 + 46 : user_data_get()->door2.talk_volume * 5 + 46};
        audio_talk_open(ctrl);
    }
}
static void monitor_switch_btn_create(Controls_location **coordinate)
{
    static btn_data btn_data = btn_data_create(NULL, monitor_switch_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_MONITOR_UNFOCUS_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_MONITOR_FOCUS_PNG);
    lv_obj_t *btn = home_btn_create_1(**coordinate, NULL, &btn_data, &info, &info1);
    lv_obj_set_id(btn, SWITCH_BTN);
    (*coordinate)++;
}

static void monitor_record_jpeg_callback(unsigned long arg1, unsigned long arg2)
{
    lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), SANP_BTN);
    if (btn == NULL)
    {
        return;
    }
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SNAP_UNFOCUS_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_SNAP_FOCUS_PNG);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
    lv_obj_clear_state(btn, LV_STATE_FOCUSED);
    lv_obj_set_click(btn, true);
}
static void monitor_snap_btn_up(lv_obj_t *obj)
{
    monitor_click_up(NULL);
    extern bool get_video_data_display_state(void);
    if (!get_video_data_display_state())
        return;

    if (is_sdcard_insert() == false)
    {
        if (sdcard_insert_msg_box == NULL)
        {
            sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_PLEASE_INSERT_SD));
        }
        return;
    }

    int free_space = sd_free_space_insufficient();
    if (free_space < 500)
    {
        if (sdcard_insert_msg_box == NULL)
        {
            sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
        }
    }

    if (free_space > 200 && record_pictrue_start(REC_MODE_MANUAL, monitor_channel_get()) == true) // 手动从当前通道拍照
    {
        lv_obj_t *btn = obj;
        Debug("%d\n", lv_obj_get_state(btn, LV_OBJ_PART_MAIN));
        static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SNAPING_UNFOCUS_PNG);
        static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_SNAPING_FOCUS_PNG);
        lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
        lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
        lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
        lv_obj_add_state(btn, LV_STATE_FOCUSED);
        lv_obj_set_click(btn, false);
    }
    else if (free_space < 200)
    {
        extern void detect_sd_free_space(void);
        detect_sd_free_space();
    }
}
static void monitor_snap_btn_create(Controls_location **coordinate)
{
    static btn_data btn_data = btn_data_create(NULL, monitor_snap_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_SNAP_UNFOCUS_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_SNAP_FOCUS_PNG);
    lv_obj_t *btn = home_btn_create_1(**coordinate, NULL, &btn_data, &info, &info1);
    lv_obj_set_id(btn, SANP_BTN);
    (*coordinate)++;
}

static void monitor_gate_task(lv_task_t *task_t)
{
    if (ungate_task_t)
    {
        lv_task_del(ungate_task_t);
        ungate_task_t = NULL;
    }
    if (monitor_channel_get() > MON_CH_DOOR_2)
    {
        return;
    }
    lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), GATE_BTN);
    if (obj)
    {
        static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_LOCK_FOCUS_PNG);
        static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_LOCK_UNFOCUS_PNG);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
        lv_obj_clear_state(obj, LV_STATE_FOCUSED);
    }
}
static void monitor_gate_btn_up(lv_obj_t *obj)
{
    monitor_click_up(NULL);
    if (ungate_task_t == NULL)
    {
        audio_play_stop_set();
        static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_UNLOCK_FOCUS_PNG);
        static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_UNLOCK_UNFOCUS_PNG);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
        lv_obj_add_state(obj, LV_STATE_FOCUSED);

        network_cmd_data data;
        data.device = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : monitor_channel_get() == MON_CH_DOOR_2 ? DEVICE_OUTDOOR_2
                                                                                                                         : DEVICE_UNKONW;
        data.cmd = NET_COMMON_CMD_UNLOCK;
        data.arg1 = monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.ungate1_delay : user_data_get()->door1.ungate1_delay;
        data.arg2 = 2 | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
        network_send_cmd_data(&data);

        ungate_task_t = lv_task_create(monitor_gate_task, (monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.ungate1_delay : user_data_get()->door1.ungate1_delay) * 1000, LV_TASK_PRIO_HIGH, obj);
    }
}
static void monitor_gate_btn_create(Controls_location **coordinate)
{
    static btn_data btn_data = btn_data_create(NULL, monitor_gate_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_LOCK_UNFOCUS_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_LOCK_FOCUS_PNG);
    static rom_bin_info info2 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_UNLOCK_FOCUS_PNG);
    static rom_bin_info info3 = rom_bin_info_get(ROM_RES_MONITOR_INDOOR_UNLOCK_UNFOCUS_PNG);
    lv_obj_t *btn = home_btn_create_1(**coordinate, NULL, &btn_data, ungate_task_t ? &info3 : &info, ungate_task_t ? &info2 : &info1);
    lv_obj_set_id(btn, GATE_BTN);
    (*coordinate)++;
}

static void monitor_video_btn_up(lv_obj_t *obj)
{
    if (!get_video_data_display_state())
        return;

    if (is_sdcard_insert() == false)
    {
        if (sdcard_insert_msg_box == NULL)
        {
            sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_PLEASE_INSERT_SD));
        }
        return;
    }

    int free_space = sd_free_space_insufficient();
    if (free_space < 500)
    {
        if (sdcard_insert_msg_box == NULL)
        {
            sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
        }
    }

    if (is_video_recording() == false && free_space > 200) // 如果已经未开始录像
    {
        if (record_video_start(REC_MODE_MANUAL, true, monitor_channel_get()) == true) // 那么开始录像
        {
            static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORDING_FOCUS_PNG);
            static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORDING_UNFOCUS_PNG);
            lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
            lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
            lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
            lv_obj_add_state(obj, LV_STATE_FOCUSED);
        }
    }
    else
    {
        if (free_space < 200)
        {
            extern void detect_sd_free_space(void);
            detect_sd_free_space();
        }
        static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORD_UNFOCUS_PNG);
        static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORD_FOCUS_PNG);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, &info1);
        lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info1);
        record_video_stop(0x00); // 停止录像
    }
}
static void monitor_video_btn_create(Controls_location **coordinate)
{
    static btn_data btn_data = btn_data_create(NULL, monitor_video_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_RECORD_UNFOCUS_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_RECORD_FOCUS_PNG);
    lv_obj_t *btn = home_btn_create_1(**coordinate, NULL, &btn_data, &info, &info1);
    lv_obj_set_id(btn, VIDEO_BTN);
    (*coordinate)++;
}

static void monitor_set_btn_up(lv_obj_t *obj)
{
    if (monitor_setting_window_flag)
    {
        lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 888);
        if (window_cont != NULL)
        {
            lv_obj_del(window_cont);
            monitor_setting_window_flag = 0;
        }
    }
    else
    {
        extern void monitor_setting_window_create(void);
        monitor_setting_window_create();
        monitor_setting_window_flag = 1;
    }
}
static void monitor_set_btn_create(Controls_location **coordinate)
{
    static btn_data btn_data = btn_data_create(NULL, monitor_set_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MONITOR_COLOR_SET_UNFOCUS_PNG);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MONITOR_COLOR_SET_FOCUS_PNG);
    lv_obj_t *btn = home_btn_create_1(**coordinate, NULL, &btn_data, &info, &info1);
    lv_obj_set_id(btn, SETTING_BTN);
    (*coordinate)++;
}

static void monitor_btn_destroy(int id)
{
    lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), id);
    if (obj != NULL)
    {
        lv_obj_del(obj);
    }
}

static void monitor_all_btn_destroy(void)
{
    monitor_btn_destroy(LOCK_BTN);
    monitor_btn_destroy(HANDUP_BTN);
    monitor_btn_destroy(ANSWER_BTN);
    monitor_btn_destroy(SWITCH_BTN);
    monitor_btn_destroy(SANP_BTN);
    monitor_btn_destroy(GATE_BTN);
    monitor_btn_destroy(VIDEO_BTN);
    monitor_btn_destroy(SETTING_BTN);
}

static void monitor_door_layer1_btn_create(void)
{
    monitor_all_btn_destroy();
    Controls_location coordinate[] = MONITOR_LAYER1_COORDINATE_INIT;
    Controls_location *module_p = &coordinate[0];
    monitor_answer_btn_create(&module_p);
    monitor_lock_btn_create(&module_p);
    monitor_handup_btn_create(&module_p);
}

static void monitor_door_layer2_btn_create(void)
{
    monitor_all_btn_destroy();
    Controls_location coordinate[] = MONITOR_LAYER2_COORDINATE_INIT;
    Controls_location *module_p = &coordinate[0];
    monitor_lock_btn_create(&module_p);
    monitor_handup_btn_create(&module_p);
    monitor_switch_btn_create(&module_p);
    monitor_snap_btn_create(&module_p);
    monitor_set_btn_create(&module_p);
    monitor_gate_btn_create(&module_p);
}

static void monitor_cctv_layer_btn_create(void)
{
    monitor_all_btn_destroy();
    Controls_location coordinate[] = MONITOR_CCTV_LAYER_COORDINATE_INIT;
    Controls_location *module_p = &coordinate[0];
    monitor_switch_btn_create(&module_p);
    monitor_video_btn_create(&module_p);
    monitor_snap_btn_create(&module_p);
    // monitor_set_btn_create(&module_p);
    monitor_handup_btn_create(&module_p);
}

static void monitor_timer_task(struct _lv_task_t *task_t)
{
    if (monitor_timeout == -1)
    {
        goto_layout(pLAYOUT(standby));
        return;
    }

    static struct ak_timeval tv1, tv2, tv3;
    ak_get_ostime(&tv3);
    if (tv3.usec - tv1.usec > 5 * 100 * 1000)
    {
        tv1 = tv3;
        monitor_time_display();
    }

    if (tv2.sec != tv3.sec)
    {
        tv2 = tv3;

        monitor_countdown_display();

        if (monitor_enter_way_get() == MONITOR_ENTER_TUYA)
        {
            return;
        }

        monitor_timeout--;

        if (monitor_ring_close_time)
        {
            if (!(--monitor_ring_close_time))
                audio_play_stop_set();
        }
        if (monitor_wait_time)
        {
            --monitor_wait_time;
            // Debug("monitor_wait_time:%d,record_video_type:%d\n", monitor_wait_time, record_video_type());

            if (!(monitor_wait_time))
            {
                door_info curr_ch = monitor_channel_get() == MON_CH_DOOR_1 ? user_data_get()->door1 : user_data_get()->door2;
                if (is_sdcard_insert() && curr_ch.message_sw && record_video_type() != REC_MODE_MESSAGE && monitor_enter_way_get() == MONITOR_ENTER_CALL) // 门口机呼叫监控时间结束且未接听则进入录制留言操作
                {
                    Debug("start record message !!!!!!\n");
                    int free_space = sd_free_space_insufficient();
                    if (free_space > 200)
                    {
                        record_video_stop(0x00);
                        if (record_video_start(REC_MODE_MESSAGE, true, monitor_channel_get()) == false)
                        {
                            monitor_wait_time = 1;
                        }
                    }
                    else if (free_space < 200)
                    {
                        extern void detect_sd_free_space(void);
                        detect_sd_free_space();
                    }
                    if (free_space < 500)
                    {
                        if (sdcard_insert_msg_box == NULL)
                        {
                            sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_SD_NO_MEMORY));
                        }
                    }
                    return;
                }
            }
        }
    }
}
static void monitor_timer_task_create(void)
{
    if (monitor_timer_task_p == NULL)
    {
        monitor_timer_task_p = lv_task_create(monitor_timer_task, 200, LV_TASK_PRIO_HIGHEST, NULL);
        monitor_timer_task(monitor_timer_task_p);
    }
}

static int get_monitor_time(void)
{
    int time = MONITOR_DURATION;
    if (monitor_enter_way_get() != MONITOR_ENTER_CALL)
    {
        time = MONITOR_DURATION;
    }
    else if (monitor_channel_get() == MON_CH_DOOR_1)
    {
        if (user_data_get()->other.model == NOT_AT_HOME_PATTERN)
        {
            time = user_data_get()->door1.message_time;
        }
        else
        {
            if (user_data_get()->door1.message_sw)
            {
                time = ring_attr.door1.ring_time + user_data_get()->door1.message_time;
            }
            else
            {
                time = ring_attr.door1.ring_time;
            }
        }
    }
    else if (monitor_channel_get() == MON_CH_DOOR_2)
    {
        if (user_data_get()->other.model == NOT_AT_HOME_PATTERN)
        {
            time = user_data_get()->door2.message_time;
        }
        else
        {
            if (user_data_get()->door2.message_sw)
            {
                time = ring_attr.door2.ring_time + user_data_get()->door2.message_time;
            }
            else
            {
                time = ring_attr.door2.ring_time;
            }
        }
    }

    return time;
}

static void layout_monitor_param_init(void)
{
    lv_area_t area[] = {
        {0, 0, 1024, 60},
        {900, 120, 1000, 520},
        {350, 187, 674, 413},
        {262, 483, 762, 583},
    };
    gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));
    monitor_timer_set(get_monitor_time());
    record_jpeg_event_register(monitor_record_jpeg_callback);
    tuya_event_register(tuya_event_inside_proc);
    outdoor_call_event_register(monitor_call_inside_func);
    indoor_cmd_event_register(monitor_indoor_cmd_func);
    monitor_door_layer_ptr = monitor_door_layer1_btn_create;
}

static void monitor_video_mode_open(bool fb_video_enable)
{
    monitor_switch(); // 打开指定通道的监控
    lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
    video_raw_clear();
    fb_video_mode_enable(fb_video_enable); // 打开
}

static void monitor_video_mode_close(void)
{
    Debug("==============monitor_video_mode_close====>>>>\n\n\n");
    fb_video_mode_enable(false);
    monitor_close();
    audio_play_stop_set();
    audio_talk_close(true); // 音频关闭
}

static void LAYOUT_ENETER_FUNC(monitor)
{
    printf("%s:%d\n", __func__, __LINE__);
    monitor_video_mode_open(true);
    layout_monitor_param_init();
    monitor_infobar_create();
    monitor_timer_task_create();

    standby_timer_open(-1, NULL); // 获取系统时间
    outdoor_order_set(NET_COMMON_CMD_NONE);
    backlight_open(true, true, get_monitor_brightness());

    if (monitor_channel_get() < MON_CH_CCTV_1)
        monitor_door_layer1_btn_create();
    else
        monitor_cctv_layer_btn_create();
}

static void LAYOUT_QUIT_FUNC(monitor)
{
    lv_area_t area[] = {{0, 0, 1024, 600}};
    gui_draw_area_set(area, sizeof(area) / sizeof(lv_area_t));

    record_video_stop(0x00);    // 停止录像
    monitor_video_mode_close(); // 监控和声音关闭

    extern void main_device_monitor_busy_func(unsigned long arg1, unsigned long arg2);
    device_monitor_busy_register(main_device_monitor_busy_func);
    record_jpeg_event_register(NULL);
    indoor_cmd_event_register(NULL);
    tuya_event_register(tuya_event_extern_proc); // 涂鸦
    outdoor_call_event_register(monitor_call_extern_func);

    outdoor_order_set(NET_COMMON_CMD_NONE);
    monitor_enter_way_set(MONITOR_ENTER_NONE);

    montior_call_ch = MON_CH_NONE;
    if (monitor_timer_task_p != NULL)
    {
        lv_task_del(monitor_timer_task_p);
        monitor_timer_task_p = NULL;
    }
    if (montior_talk_task_t != NULL)
    {
        lv_task_del(montior_talk_task_t);
        montior_talk_task_t = NULL;
    }
    if (unlock_task_t)
    {
        lv_task_del(unlock_task_t);
        unlock_task_t = NULL;
    }
    if (ungate_task_t)
    {
        lv_task_del(ungate_task_t);
        ungate_task_t = NULL;
    }
    if (auto_shoot_task_t != NULL)
    {
        lv_task_del(auto_shoot_task_t);
        auto_shoot_task_t = NULL;
    }
    monitor_setting_window_flag = 0;
}

void layout_monitor_init(void)
{
    outdoor_call_event_register(monitor_call_extern_func); // 户外机呼叫事件注册表 回调
    tuya_event_register(tuya_event_extern_proc);           // 涂鸦时间注册表
}

CREATE_LAYOUT(monitor);
#endif