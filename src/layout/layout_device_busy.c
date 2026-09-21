/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-10-26 20:45:10
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-01 10:36:09
 * @FilePath: /two-wire-indoor/src/layout/layout_device_busy.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include"layout_define.h"
extern bool get_outdoor_talk_state(MONITOR_CH ch);

static lv_task_t *motion_busy_detect_task_t = NULL;
static lv_obj_t *msg_dev_busy_t = NULL;
static void motion_busy_detect_task(lv_task_t *task_t)
{
    if(!get_outdoor_talk_state(MON_CH_DOOR_1) && !get_outdoor_talk_state(MON_CH_DOOR_2) && !tuya_monitor_state_get() && (tuya_online_clinet_num_get() == 0))
    {
        Debug("\n\n\n\n");
        goto_layout(pLAYOUT(standby));
    }
}

static void LAYOUT_ENETER_FUNC(dev_busy)
{
    backlight_open(true,false,user_data_get()->other.brightness);
    if(motion_busy_detect_task_t != NULL)
    {
        lv_task_del(motion_busy_detect_task_t);
    }
	motion_busy_detect_task_t = lv_task_create(motion_busy_detect_task, 1000, LV_TASK_PRIO_HIGH, NULL);

	if(msg_dev_busy_t == NULL)
    {
		msg_dev_busy_t = msg_window_create(text_str(STR_SYSYEM_BUSY), true);
    }
}

static void LAYOUT_QUIT_FUNC(dev_busy)
{
    if(motion_busy_detect_task_t != NULL)
    {
        lv_task_del(motion_busy_detect_task_t);
        motion_busy_detect_task_t = NULL;
    }
	if(msg_dev_busy_t)
    {
		lv_obj_del_reload(&(msg_dev_busy_t));        /* !!! 一定要删除loading 弹窗的父对象 */
        msg_dev_busy_t = NULL;
    }
        
}

CREATE_LAYOUT(dev_busy);