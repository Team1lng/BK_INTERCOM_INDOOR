/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-04-16 16:09:00
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-05-21 11:56:49
 * @FilePath: /two-wire-indoor/src/layout/tcp_upgrade_windows.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "tcp_upgrade.h"
#include "layout_define.h"
#include "network_common.h"

#define OUTDOOR_IPADDR "192.168.37."
#define UPGRADE_FILE_MAX 20
#define UPGRADE_DEVICE_NUM 2

static void ListFlush(lv_obj_t *obj, int type);

static char *path_group[] =
    {
        "/app/app/", "/mnt/tf/"};

static char *model_group[] =
    {
        "330L-82227-EPC", "330L-2CD-ME2NP"};

typedef struct
{
    char name[32];
    union
    {
        char path[128];
        char ip[16];
    };

} list_info;

struct list
{
    list_info info[UPGRADE_FILE_MAX];
    int num;
} upgrade_list, device_list;

static int list_type = 0;

static int SearchUpgradeFile(char *path, char *prefix)
{
    DIR *dir;
    struct dirent *entry;

    // Open directory
    dir = opendir(path);
    if (dir == NULL)
    {
        perror("Unable to open directory");
        return 1;
    }
    // Read directory entries
    while ((entry = readdir(dir)) != NULL)
    {
        // Check if entry is a regular file and starts with prefix
        if (entry->d_type == DT_REG && strncmp(entry->d_name, prefix, strlen(prefix)) == 0)
        {
            // Allocate memory for file name and copy it
            sprintf(upgrade_list.info[upgrade_list.num].path, "%s", path);
            sprintf(upgrade_list.info[upgrade_list.num++].name, "%s", entry->d_name);
            // Break loop if maximum number of files reached
            if (upgrade_list.num >= UPGRADE_FILE_MAX)
            {
                break;
            }
        }
    }
    closedir(dir);
    return upgrade_list.num;
}

static int RetrieveAllUpgradePath(void)
{
    int path_num = sizeof(path_group) / sizeof(char *);
    int model_num = sizeof(model_group) / sizeof(char *);
    memset(&upgrade_list, 0, sizeof(upgrade_list));
    for (int i = 0; i < path_num; i++)
    {
        for (int j = 0; j < model_num; j++)
        {
            SearchUpgradeFile(path_group[i], model_group[j]);
        }
    }
    // Debug("%d upgrade files were retrieved\n", upgrade_list.num);
    for (int x = 0; x < upgrade_list.num; x++)
    {
        Debug("%d. path:%s%s\n", x, upgrade_list.info[x].path, upgrade_list.info[x].name);
    }
    return upgrade_list.num;
}

static int SearchUpgradeDevice(void)
{
    memset(&device_list, 0, sizeof(device_list));
    char *string[] = {"Door", "门口机", "Türstation ", "דלת  ", "Drzwi ", "Porta", "Puerta", "Platine ", "玄関ドアフォン", "Porta"};
    for (int i = 0; i < DEVICE_CCTV_1 - DEVICE_OUTDOOR_1; i++)
    {
        // if (device_online_state_get(i + DEVICE_OUTDOOR_1))
        {
            sprintf(device_list.info[device_list.num].name, "%s%d", string[user_data_get()->other.language], i + 1);
            sprintf(device_list.info[device_list.num].ip, "%s%d", OUTDOOR_IPADDR, i + 7);
            Debug("%d.%s %s\n", device_list.num, device_list.info[device_list.num].name, device_list.info[device_list.num].ip);
            device_list.num++;
        }
    }
    Debug("device_list.num:%d\n", device_list.num);
    return device_list.num;
}

static void ConfirmUp(lv_obj_t *obj)
{
    lv_obj_t *cont = ((btn_data *)(obj->user_data))->user_data;
    if (cont && cont->parent)
    {
        lv_obj_del(cont->parent);
    }
}

static void ConfirmWindowCreate(lv_obj_t *parent, char *str)
{
    lv_obj_t *obj = lv_cont_create(parent, NULL);
    lv_obj_set_size(obj, 400, 300);
    lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x1c, 0x1c, 0x1c));
    lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0, 0x13, 0x1d));
    lv_obj_set_style_local_bg_opa(obj, LV_LIST_PART_BG, LV_STATE_DEFAULT, LV_OPA_80);
    lv_obj_set_style_local_radius(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);

    /* 创建标题 */
    lv_obj_t *label = lv_label_create(obj, NULL);
    lv_label_set_text(label, "Upgrade Result");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);
    // lv_obj_t *img1 = lv_img_create(obj, NULL);
    // lv_obj_set_size(img1, 220, 2);
    // lv_obj_align(img1, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);
    // static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE5_PNG);
    // lv_img_set_src(img1, &info1);

    /* 创建弹窗内容 */
    lv_obj_t *cont_parent = lv_cont_create(obj, NULL);
    lv_obj_set_size(cont_parent, 360, 180);
    lv_obj_align(cont_parent, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_local_bg_color(cont_parent, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x30, 0x30, 0x30));
    lv_obj_set_style_local_bg_opa(cont_parent, LV_LIST_PART_BG, LV_STATE_DEFAULT, LV_OPA_90);
    lv_obj_t *content = lv_label_create(cont_parent, NULL);
    lv_label_set_text(content, str);
    lv_obj_align(content, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_color(content, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0x00, 0x00));
    lv_obj_set_style_local_radius(content, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);

    /* 创建确认键 */
    // lv_obj_t *img2 = lv_img_create(obj, NULL);
    // lv_obj_set_size(img2, 220, 2);
    // lv_obj_align(img2, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -60);
    // static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_LINE2_PNG);
    // lv_img_set_src(img2, &info2);
    lv_obj_t *btn = lv_btn_create(obj, NULL);
    lv_obj_set_size(btn, 220, 50);
    lv_obj_align(btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0, 0));
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_MAKE(0xFF, 0, 0));
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_80);
    lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CONFIRM));
    lv_obj_set_style_local_text_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0x00, 0x00));
    lv_obj_set_style_local_radius(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    static btn_data btn_data1 = btn_data_create(NULL, ConfirmUp, NULL);
    btn_data1.user_data = parent;
    btn->user_data = &btn_data1;
    btn_touch_event_listen(btn);
    // lv_obj_t *img3 = lv_img_create(obj, NULL);
    // lv_obj_set_size(img3, 220, 2);
    // lv_obj_align(img3, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);
    // lv_img_set_src(img3, &info2);
}

static void WindowsDisplayTask(struct _lv_task_t *task_t)
{
    lv_obj_t *cont = (lv_obj_t *)(task_t->user_data);
    lv_obj_t *label = (lv_obj_t *)(cont->user_data);
    char *str[] = {"Ready Upgrade", "Start Uploading...", "Version Consistent", "Upload Fail!!!", "Start Upgrade...", "Upgrade Finish!!!", "Upgrade Fail!!!"};
    static upgrade_status status = NoneUpgrade;
    if (GetUpgradeProgress() != status)
    {
        status = GetUpgradeProgress();
    }
    lv_obj_set_style_local_value_str(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str[status]);
    Debug("status:%s\n", str[status]);
    if (status == UploadFail || status == UpgradeFail || status == UpgradeFinish || status == VersionConsistent)
    {
        if (task_t)
            lv_task_del(task_t); /* !!! 一定要删除loading 弹窗的父对象 */
        if (label)
            lv_obj_del(label);
        ConfirmWindowCreate(cont, str[status]);
    }
}

static lv_obj_t *UpgradeWindowCreate(lv_obj_t *parent, char *str, bool is_loading)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, LV_HOR_RES_MAX, LV_VER_RES_MAX); // 一整个屏幕大小
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);

    /* 有消息弹窗时, 除了弹窗外,其他区域变为深色, 有那个 聚焦的效果 */
    // lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    // lv_obj_set_style_local_bg_color(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

    lv_obj_t *obj = lv_cont_create(cont, NULL);
    lv_obj_set_size(obj, 600, 360);
    lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(obj, true);

    if (str != NULL)
    {
        lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str);
    }
    lv_obj_set_style_local_value_font(obj, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, FONT_SIZE_L(31));
    lv_obj_set_style_local_value_align(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_90);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1C1C1C));

    /* 灰色边框 */
    lv_obj_set_style_local_border_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_border_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5C5C5C));
    lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_border_side(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
    lv_obj_set_style_local_border_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_100);
    lv_obj_set_style_local_border_color(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x5C5C5C));

    if (is_loading)
    {
        lv_obj_t *preload = lv_spinner_create(obj, NULL);
        lv_obj_set_size(preload, 120, 120);
        lv_obj_align(preload, preload->parent, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_local_line_color(preload, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0X2183EF));
        lv_obj_set_style_local_line_color(preload, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0XFFFFFF));
        lv_obj_set_style_local_line_width(preload, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, 12);
        lv_obj_set_style_local_line_width(preload, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, 12);

        /* 如果有加载框， 需要把文字向下偏移 */
        lv_obj_set_style_local_value_ofs_y(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 100);
    }

    cont->user_data = obj;
    lv_task_create(WindowsDisplayTask, 1000, LV_TASK_PRIO_HIGH, cont);
    return cont;
}

#define LIST_BTN_HEIGHT 52      // 列表按键高度
#define LIST_ONCE_DISPLAY_MAX 6 // 列表一次显示最多
#define LIST_HEIGHT (LIST_BTN_HEIGHT * LIST_ONCE_DISPLAY_MAX)

static void ListBtnUp(lv_obj_t *obj)
{
    lv_obj_t *btn = lv_obj_get_child(obj, NULL);
    lv_obj_set_state(btn, LV_STATE_DEFAULT);
    lv_obj_t *list = ((btn_data *)(obj->user_data))->user_data;
    bool *list_slide = ((btn_data *)(list->user_data))->user_data;
    static int dev_id = 0;
    if (*list_slide == false)
    {

        switch (list_type)
        {
        case 0:
            dev_id = obj->obj_id; /* 先赋值，再刷新 */
            ListFlush(list, ++list_type);
            break;

        case 1:
        {
            char path[128] = {0};
            sprintf(path, "%s%s", upgrade_list.info[obj->obj_id].path, upgrade_list.info[obj->obj_id].name);
            Debug("%d.%s %s %s \n", obj->obj_id, device_list.info[dev_id].name, device_list.info[dev_id].ip, path);
            lv_obj_del(list->parent);
            // lv_obj_del(list->parent->parent);
            UpgradeWindowCreate(list->parent->parent, "Upgradeing ....", true);
            CreateUpgradeClientTask(device_list.info[dev_id].ip, path);
        }
        break;
        default:
            break;
        }
    }
}

static void ListBtnDown(lv_obj_t *obj)
{
    lv_obj_t *btn = lv_obj_get_child(obj, NULL);
    lv_obj_set_state(btn, LV_STATE_PRESSED);
}

static void ListBtnAnything(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_PRESS_LOST == event)
    {
        lv_obj_t *btn = lv_obj_get_child(obj, NULL);
        lv_obj_set_state(btn, LV_STATE_DEFAULT);
    }
}

static void FileListTouchAnything(lv_obj_t *obj, lv_event_t event)
{
    bool *list_slide = ((btn_data *)(obj->user_data))->user_data;
    if (LV_EVENT_DRAG_BEGIN == event)
    {
        *list_slide = true;
    }
    else if (LV_EVENT_DRAG_END == event)
    {
        *list_slide = false;
    }
}

static void UpgradeCancelUp(lv_obj_t *obj)
{
    lv_obj_t *cont = ((btn_data *)(obj->user_data))->user_data;
    lv_obj_del(cont->parent);
}

static lv_obj_t *ListCreateFunc(lv_obj_t *parent, int y, int h, int i, char *string)
{
    lv_obj_t *list_btn = lv_list_add_btn(parent, NULL, NULL);

    if (i >= 0)
    {
        lv_obj_set_id(list_btn, i);
    }

    lv_obj_set_style_local_pad_ver(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_hor(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
    lv_obj_set_style_local_border_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
    lv_obj_set_style_local_border_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(list_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);

    lv_obj_t *cont = lv_cont_create(list_btn, NULL);
    if (i >= 0)
        lv_obj_set_id(cont, i);
    lv_obj_set_click(cont, false);
    lv_obj_set_size(cont, 280, h);

    if (i < 0)
        return list_btn;

    lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x30, 0x30, 0x30));
    // lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xD0, 0xD0, 0xD0));
    lv_obj_set_style_local_bg_opa(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_90);
    lv_obj_set_style_local_bg_color(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x80, 0x0, 0x10));
    lv_obj_set_style_local_bg_opa(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_100);
    lv_obj_set_style_local_border_side(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
    lv_obj_set_style_local_border_width(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);

    lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0X00959e));
    lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0X00959e));
    lv_obj_set_style_local_border_color(cont, LV_BTN_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0X00959e));

    lv_obj_t *name_label = lv_label_create(cont, NULL);
    lv_label_set_text(name_label, string);
    lv_obj_align(name_label, cont, LV_ALIGN_IN_LEFT_MID, 5, 0);
    lv_label_set_long_mode(name_label, LV_LABEL_LONG_SROLL);
    lv_obj_set_width(name_label, 275); // 设置标签宽度
    lv_obj_set_height(name_label, 52); // 设置标签高度
    lv_obj_set_id(name_label, i);

    // 单独的回调函数
    static btn_data btn_data = btn_data_create(ListBtnDown, ListBtnUp, NULL);
    btn_data.OPS_ANYTHING = ListBtnAnything;
    btn_data.user_data = parent;
    list_btn->user_data = &btn_data;
    btn_touch_event_listen(list_btn);

    return list_btn;
}

static void ListFlush(lv_obj_t *obj, int type)
{
    lv_list_clean(obj);

    int i = 0;
    struct list *List = type ? &upgrade_list : &device_list;
    int j = List->num;
    for (; i < UPGRADE_FILE_MAX; i++, j--)
    {
        if (j == 0)
        {
            break;
        }

        if (ListCreateFunc(obj, (i)*LIST_BTN_HEIGHT, LIST_BTN_HEIGHT, i, List->info[i].name) == NULL)
        {
            i--;
        }
    }

    /* 创建10个空表格，使屏幕哪怕列表未满10个也能做上下拉操作 */
    if (i < LIST_ONCE_DISPLAY_MAX)
    {
        ListCreateFunc(obj, (i)*LIST_BTN_HEIGHT, LIST_BTN_HEIGHT * (LIST_ONCE_DISPLAY_MAX - i), -1, List->info[i].name);
        ListCreateFunc(obj, (i)*LIST_BTN_HEIGHT, 10, -1, List->info[i].name);
    }
}

static void list_window_deleted_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_DELETE)
    {
        // 在这里执行删除 list_window 后需要执行的操作
        if (obj->user_data)
        {
            void (*finish_callback)(void) = obj->user_data;
            finish_callback();
        }
    }
}

int UpgradeListPageCreate(void (*finish_callback)(void))
{
    if (SearchUpgradeDevice() == 0)
    {
        return 0;
    }
    if (RetrieveAllUpgradePath() == 0)
    {
        return 0;
    }
    // 创建页面
    lv_obj_t *list_window = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_opa(list_window, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_pos(list_window, 0, 0);
    lv_obj_set_size(list_window, 1024, 600);
    list_window->user_data = finish_callback;
    lv_obj_set_event_cb(list_window, list_window_deleted_cb);
    Debug("list_window:%p\n", list_window);

    lv_obj_t *list_cont = lv_cont_create(list_window, NULL);
    set_location(list_cont, 199, 0, 320, LIST_HEIGHT + LIST_BTN_HEIGHT * 2 + 20);
    lv_obj_align(list_cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(list_cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x13, 0x1D));
    lv_obj_set_style_local_bg_opa(list_cont, LV_LIST_PART_BG, LV_STATE_DEFAULT, LV_OPA_80);
    lv_obj_set_style_local_radius(list_cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);

    /* 创建标题 */
    lv_obj_t *label = lv_label_create(list_cont, NULL);
    lv_label_set_text(label, "Upgrade list");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);
    lv_obj_set_style_local_value_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0xFF, 0x00, 0x00));
    lv_obj_t *img1 = lv_img_create(list_cont, NULL);
    lv_obj_set_size(img1, 220, 2);
    lv_obj_align(img1, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);
    static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE5_PNG);
    lv_img_set_src(img1, &info1);

    /* 创建取消按钮 */
    lv_obj_t *img2 = lv_img_create(list_cont, NULL);
    lv_obj_set_size(img2, 220, 2);
    lv_obj_align(img2, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -60);
    static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_LINE2_PNG);
    lv_img_set_src(img2, &info2);
    lv_obj_t *btn = lv_btn_create(list_cont, NULL);
    lv_obj_set_size(btn, 220, 50);
    lv_obj_align(btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
#if 1
    lv_obj_set_style_local_bg_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x00, 0x10, 0x0D));
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_40);
#else
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_bg_opa(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
#endif
    lv_obj_set_style_local_value_str(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CANCEL));
    lv_obj_set_style_local_value_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0xFF, 0x00, 0x00));
    static btn_data btn_data1 = btn_data_create(NULL, UpgradeCancelUp, NULL);
    btn_data1.user_data = list_cont;
    btn->user_data = &btn_data1;
    btn_touch_event_listen(btn);
    lv_obj_t *img3 = lv_img_create(list_cont, NULL);
    lv_obj_set_size(img3, 220, 2);
    lv_obj_align(img3, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);
    lv_img_set_src(img3, &info2);

    lv_obj_t *list = lv_list_create(list_cont, NULL); // 在当前活跃的屏幕上创建页面
    lv_list_set_edge_flash(list, true);
    set_location(list, 0, LIST_BTN_HEIGHT, 320, LIST_HEIGHT);
    lv_obj_set_style_local_pad_ver(list, LV_LIST_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_hor(list, LV_LIST_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_bg_color(list, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x13, 0x1D));
    lv_obj_set_style_local_bg_opa(list, LV_LIST_PART_BG, LV_STATE_DEFAULT, LV_OPA_80);

    static bool list_slide = false;
    static btn_data btn_data2 = btn_data_anything_create(FileListTouchAnything);
    btn_data2.user_data = &list_slide;
    list->user_data = &btn_data2;
    btn_touch_event_listen(list);

    list_type = 0;
    ListFlush(list, list_type);
    return 1;
}
