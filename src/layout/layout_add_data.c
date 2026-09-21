#include "layout_define.h"
#include "leo_api.h"
#include <stdlib.h>
#include <time.h>

extern data_info_t data_info;

static lv_anim_t a1;

#define ADD_TIMEOUT 20

static lv_task_t *add_timeout_task_t = NULL;
static int add_timeout_time = 0;

tcp_device curr_tcp_dev_id_get(void);

static void add_timeout_task(lv_task_t *task_t)
{
    add_timeout_time++;
    if (add_timeout_time > ADD_TIMEOUT)
    {
        goto_layout(pLAYOUT(data_list));
        if (add_timeout_task_t != NULL) // 计时任务未退出
        {
            lv_task_del(add_timeout_task_t); // 退出计时任务
            add_timeout_task_t = NULL;       // 置空指针
            add_timeout_time = 0;
        }
    }
}

static void tcp_network_data_handler_callback(unsigned long arg1, unsigned long arg2, unsigned long arg3);

void add_data_event_handler(char *buff)
{
    printf("=================>>>> 接收卡号:[%02x %02x %02x %02x %02x]\n", buff[0], buff[1], buff[2], buff[3], buff[4]);
    if (data_info.total >= DATA_NUM_MAX)
        return;
    lv_anim_start(&a1);
    for (int i = 0; i < DATA_NUM_MAX; i++)
    {
        if (data_info.data[i].lock_state == 0)
        {
            data_info.total++;
            data_info.data[i].lock_state = 0x03;
            memcpy(data_info.data[i].number, buff, DATA_NUMBER_SIZE);
            break;
        }
    }

    // printf("============>> data total : [%d] \n", data_info.total);

    // for (int i = 0; i < DATA_NUM_MAX; i++)
    // {
    // 	if(data_info.data[i].serial_number != 0)
    // 		printf("===NO.[%d]=====data num:[%02x %02x %02x %02x %02x]\n",
    //         data_info.data[i].serial_number,
    //         data_info.data[i].number[0],
    //         data_info.data[i].number[1],
    //         data_info.data[i].number[2],
    //         data_info.data[i].number[3],
    //         data_info.data[i].number[4]);
    // }
    // printf("===================>>> 加卡成功\n");
}

static void add_data_text_anim_cb(void *var, lv_anim_value_t value)
{
    lv_obj_set_style_local_value_opa(var, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, value);
}

// static void add_data_test_btn_up(lv_obj_t * obj)
// {
//     srand((unsigned int)time(NULL));
//     for (int i = 0; i < 151; i++)
//     {
//         int ret = rand() % 100;
//         char data[5] = {ret, ret, ret, ret, ret};
//         add_data_event_handler(data);
//     }
// }

static void add_data_bg_img_text_display(void)
{
    // setting_bg_display();
    lv_obj_t *bg = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg, 1024, 600);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_set_style_local_bg_color(bg, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00131D));
    lv_obj_set_style_local_bg_opa(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    lv_obj_t *img_data = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(img_data, 381, 386);
    lv_obj_align(img_data, NULL, LV_ALIGN_CENTER, 0, 0);
    static rom_bin_info info_data = rom_bin_info_get(ROM_RES_SETTING_ADD_CARD_PNG);
    static rom_bin_info info_data1 = rom_bin_info_get(ROM_RES_SETTING_ADD_FINGER_PNG);
    lv_obj_set_style_local_pattern_image(img_data, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, data_manage_type_get() == 0 ? &info_data : &info_data1);
    // lv_obj_set_style_local_pattern_recolor(img_data,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x0000FF));

    // static char str[30] = "Add Success\0";
    lv_obj_set_style_local_value_str(img_data, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_ADD_SUCCESS));
    lv_obj_set_style_local_value_color(img_data, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_align(img_data, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
    lv_obj_set_style_local_value_ofs_y(img_data, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, -30);
    lv_obj_set_style_local_value_font(img_data, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(36));
    lv_obj_set_style_local_value_opa(img_data, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

    lv_anim_path_t path;
    lv_anim_path_init(&path);
    lv_anim_path_set_cb(&path, lv_anim_path_linear);

    lv_anim_init(&a1);
    /* 在动画中设置路径 */
    lv_anim_set_path(&a1, &path);
    lv_anim_set_exec_cb(&a1, add_data_text_anim_cb);
    lv_anim_set_var(&a1, img_data);
    lv_anim_set_time(&a1, 1000);
    lv_anim_set_values(&a1, LV_OPA_0, LV_OPA_100);
    lv_anim_set_delay(&a1, 0);
    lv_anim_set_playback_time(&a1, 1000);
    lv_anim_set_playback_delay(&a1, 200);
    lv_anim_set_repeat_count(&a1, 1);
    // lv_anim_start(&a1);

    // lv_obj_t *btn = lv_obj_create(lv_scr_act(), NULL);
    // lv_obj_set_pos(btn, 200, 200);
    // lv_obj_set_size(btn, 100, 50);
    // lv_obj_set_style_local_bg_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    // lv_obj_set_style_local_radius(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    // lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xAAAAAA));
    // lv_obj_set_style_local_bg_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x3BD741));

    // static btn_data btn_data = btn_data_create(NULL, add_data_test_btn_up, NULL);
    // btn->user_data = &btn_data;
    // btn_touch_event_listen(btn);

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xBDBDBD));
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(36));
    if (data_manage_type_get() == 0)
    {
        lv_label_set_text(label, text_str(STR_PLEASE_ADD_CARD));
    }
    else
    {
        lv_label_set_text(label, text_str(STR_PLEASE_ADD_FINGER));
    }

    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -70);
}

static void add_data_back_btn_up(lv_obj_t *obj)
{
    lv_obj_set_state(obj, LV_STATE_DEFAULT);
    goto_layout(pLAYOUT(data_list));
}

static void LAYOUT_ENETER_FUNC(add_data)
{
    standby_timer_close();
    tcp_network_cmd_add_data_send(curr_tcp_dev_id_get(), 1, 2);
    add_data_bg_img_text_display();
    home_back_btn_create(add_data_back_btn_up, NULL);
    tcp_network_event_register(tcp_network_data_handler_callback);
    add_timeout_task_t = lv_task_create(add_timeout_task, 1000, LV_TASK_PRIO_HIGH, NULL);
}

static void LAYOUT_QUIT_FUNC(add_data)
{
    standby_timer_open(-1, NULL);
    lv_anim_del(&a1, add_data_text_anim_cb);
    user_data_save();

    tcp_network_event_register(NULL);
    tcp_network_cmd_exit_send(curr_tcp_dev_id_get());

    if (target_layout != pLAYOUT(data_list) && target_layout != pLAYOUT(add_data))
        tcp_management_close();

    if (add_timeout_task_t != NULL) // 计时任务未退出
    {
        lv_task_del(add_timeout_task_t); // 退出计时任务
        add_timeout_task_t = NULL;       // 置空指针
        add_timeout_time = 0;
    }
}

CREATE_LAYOUT(add_data);

static void tcp_network_data_handler_callback(unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    switch (arg1)
    {
    case NET_COMMON_CMD_ADD_FINGER:
        add_timeout_time = 0;
        lv_obj_set_style_local_value_str(a1.var, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, arg2 != FAIL_RET ? text_str(STR_ADD_SUCCESS) : text_str(STR_ADD_FAILED));
        lv_anim_start(&a1);
        break;

    case NET_COMMON_CMD_ADD_CARD:
        add_timeout_time = 0;
        lv_obj_set_style_local_value_str(a1.var, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, arg2 != FAIL_RET ? text_str(STR_ADD_SUCCESS) : text_str(STR_ADD_FAILED));
        lv_anim_start(&a1);
        break;

    case NET_COMMON_CMD_EXIT_FINGER:
        goto_layout(pLAYOUT(standby));
        break;

    case NET_COMMON_CMD_EXIT_CARD:
        goto_layout(pLAYOUT(standby));
        break;

    default:
        break;
    }
}