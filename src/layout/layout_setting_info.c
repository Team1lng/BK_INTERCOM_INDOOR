
#include "layout_define.h"
#include "leo_api.h"
#include "tuya_uuid_and_key.h"
typedef enum info_module_list
{
	HARDWARE_MODULE,
	FIRMWARE_MODULE,
	SOFTWARE_MODULE,
	DOOR1_VER_MODULE,
	DOOR2_VER_MODULE,
#ifdef DOOR_COMPILE_TIME
	DOOR1_COMPILE_MODULE,
	DOOR2_COMPILE_MODULE,
#endif
	RELESE_DATE_MODULE,
	SD_SPACE_MODULE,
	TUYA_PID_MODULE,
	TUYA_UUID_MODULE,
	TOTAL_MODULE
} info_module_list;

#define INFO_MODULE_COORDINATE_INIT { \
	{199, 75, 700, 52},               \
	{199, 127, 700, 52},              \
	{199, 179, 700, 52},              \
	{199, 231, 700, 52},              \
	{199, 285, 700, 52},              \
	{199, 337, 700, 52},              \
	{199, 389, 700, 52},              \
	{199, 441, 700, 52},              \
	{199, 493, 700, 52},              \
	{199, 545, 700, 52},              \
};

static void info_img_text_display(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);

	lv_obj_set_pos(img, 44, 218);
	lv_obj_set_size(img, 102, 102);
	static rom_bin_info info = rom_bin_info_get(ROM_RES_SETTING_INFO_UNFOCUS_PNG);
	lv_img_set_src(img, &info);

	lv_obj_set_style_local_value_str(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_SYSTEM_INFO));
	lv_obj_set_style_local_value_color(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_align(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 15);
	lv_obj_set_style_local_value_font(img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
}

static void info_Hardware_version_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);

	sys_setting_btn_create(**coordinate, Hardware_version, text_str(STR_HARDWARE_VERSION), &btn_data3, NULL, NULL);
	(*coordinate)++;
}

int get_mtd_dev_count(void)
{
	FILE *fp;
	char line[256];
	int count = 0;
	// 打开/proc/mounts文件
	fp = fopen("/proc/mtd", "r");
	if (fp == NULL)
	{
		printf("无法打开/proc/mtd 文件\n");
		return -1;
	}
	// 逐行读取文件内容，查找包含"mtd"关键字的行
	while (fgets(line, sizeof(line), fp))
	{
		if (strstr(line, "mtd") != NULL)
		{
			count++;
		}
	}
	// 关闭文件
	fclose(fp);
	printf("MTD个数:%d\n", count);
	return count;

	return count;
}
static void info_Firmware_version_btn_create(Controls_location **coordinate)
{

	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	static char firmware_ver[128] = {0};
	sprintf(firmware_ver, "Ver 1.%d", get_mtd_dev_count() > 9 ? 3 : 1);
	sys_setting_btn_create(**coordinate, firmware_ver, text_str(STR_FIRMWARE_VERSION), &btn_data3, NULL, NULL);
	(*coordinate)++;
}

static void info_Software_version_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);

	sys_setting_btn_create(**coordinate, Software_version, text_str(STR_SOFTWARE_VERSION), &btn_data3, NULL, NULL);
	(*coordinate)++;
}

static void info_Outdoor_version_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(NULL, NULL, NULL);
	static char str1[32] = {0};
	int ver = 0;
	int model = 0;
	if (get_outdoor_model_version(DEVICE_OUTDOOR_1, &model, &ver) == 0)
	{
		if (model == OLD_OUTDOOR_MODEL)
		{
			sprintf(str1, "Ver %02d.%02d", ver / 100, ver % 100);
		}
		else
		{
			sprintf(str1, "Ver %d.%d.%d", ver / 10000, ver % 10000 / 100, ver % 100);
		}

		sys_setting_btn_create(**coordinate, str1, text_str(STR_DOOR_1_VERSION), &btn_data1, NULL, NULL);
		(*coordinate)++;
	}

	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	static char str4[32] = {0};
	if (get_outdoor_model_version(DEVICE_OUTDOOR_2, &model, &ver) == 0)
	{
		if (model == OLD_OUTDOOR_MODEL)
		{
			sprintf(str4, "Ver %02d.%02d", ver / 100, ver % 100);
		}
		else
		{
			sprintf(str4, "Ver %d.%d.%d", ver / 10000, ver % 10000 / 100, ver % 100);
		}

		sys_setting_btn_create(**coordinate, str4, text_str(STR_DOOR_2_VERSION), &btn_data3, NULL, NULL);
		(*coordinate)++;
	}
}

#ifdef DOOR_COMPILE_TIME
static void info_Outdoor_compile_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data1 = btn_data_create(NULL, NULL, NULL);
	static char str1[32] = {0};
	if (get_outdoor_info(0) != NULL && get_outdoor_version(DEVICE_OUTDOOR_1) != 0)
	{
		sprintf(str1, "20%02d-%02d:%02d", get_outdoor_info(0)->Compile_year, get_outdoor_info(0)->Compile_mon, get_outdoor_info(0)->Compile_day);
		Debug("str :%s\n", str1);
		sys_setting_btn_create(**coordinate, str1, text_str(STR_DOOR_1_COMPILE), &btn_data1, NULL, NULL);
		(*coordinate)++;
	}

	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	static char str4[32] = {0};
	if (get_outdoor_info(1) != NULL && get_outdoor_version(DEVICE_OUTDOOR_2) != 0)
	{
		sprintf(str4, "20%02d-%02d:%02d", get_outdoor_info(1)->Compile_year, get_outdoor_info(1)->Compile_mon, get_outdoor_info(1)->Compile_day);

		sys_setting_btn_create(**coordinate, str4, text_str(STR_DOOR_2_COMPILE), &btn_data3, NULL, NULL);
		(*coordinate)++;
	}
}
#endif

void get_curr_relesse_date(int *day, int *month, int *year)
{
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
							"Sep", "Oct", "Nov", "Dec"};
	char temp[] = __DATE__;
	*year = atoi(temp + 7);
	*(temp + 6) = 0;
	*day = atoi(temp + 4);
	*(temp + 3) = 0;
	for (int i = 0; i < 12; i++)
	{
		if (!strcmp(temp, months[i]))
		{
			*month = i + 1;
			break;
		}
	}

	Debug("Release Date : %d-%d-%d\n\r", *year, *month, *day);
}
static void info_Relesse_date_btn_create(Controls_location **coordinate)
{

	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	static char str1[32] = {0};

	int day;
	int month = 0;
	int year;

	get_curr_relesse_date(&day, &month, &year);
	sprintf(str1, "%d-%02d-%02d", year, month, day);
	sys_setting_btn_create(**coordinate, str1, text_str(STR_RELEASE_DATE), &btn_data3, NULL, NULL);
	(*coordinate)++;
}

static void info_SD_remain_space_btn_create(Controls_location **coordinate)
{

	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	static char str1[20] = {0};
	unsigned int p_total, p_user, p_free;
	tuya_sd_memory_query(&p_total, &p_user, &p_free);
	float tuya_remain = 0, tuya_all = 0;
	tuya_all = p_total / (float)(1024 * 1024);
	tuya_remain = p_free / (float)(1024 * 1024);
	// Debug("p_total:%f,p_free:%f\n", tuya_remain, tuya_all);

	unsigned long bavail, disk_all_space;
	get_SD_space(&bavail, &disk_all_space);
	float remain = 0, all = 0;
	remain = bavail / (float)1024;
	all = disk_all_space / (float)1024;
	if (!is_sdcard_insert())
	{
		remain = 0;
		all = 0;
	}

	sprintf(str1, "%.2fG/%0.2fG", remain + tuya_remain, all + tuya_all);

	lv_obj_t *btn = sys_setting_btn_create(**coordinate, str1, text_str(STR_SD_SIZE), &btn_data3, NULL, NULL);
	(*coordinate)++;
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
}

static void info_Tuya_pid_btn_create(Controls_location **coordinate)
{
	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	lv_obj_t *btn = sys_setting_btn_create(**coordinate, IPC_APP_PID, text_str(STR_TUYA_PID), &btn_data3, NULL, NULL);
	(*coordinate)++;
	lv_obj_set_base_dir(btn, LV_BIDI_DIR_LTR);
}

static void window_no_btn_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 789);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
}
static lv_task_t *msgbox_task_t = NULL;
static void tuya_switch_msgbox_task(lv_task_t *task_t)
{
	lv_obj_t *msgbox_cont = lv_obj_get_child_form_id(lv_scr_act(), 456);
	if (msgbox_cont != NULL)
	{
		lv_obj_del(msgbox_cont);
	}
	if (msgbox_task_t != NULL)
	{
		lv_task_del(msgbox_task_t);
		msgbox_task_t = NULL;
	}
}
void tuya_switch_fail_msgbox_create(char *str)
{
	printf("%s ===================+>>%s\n\r", __func__, str);
	lv_obj_t *msgbox_cont1 = lv_obj_get_child_form_id(lv_scr_act(), 456);
	if (msgbox_cont1 != NULL)
	{
		ak_sleep_ms(400);
		lv_obj_del(msgbox_cont1);
	}
	if (msgbox_task_t != NULL)
	{
		lv_task_del(msgbox_task_t);
		msgbox_task_t = NULL;
	}
	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	lv_obj_set_id(window_cont, 456);
	lv_obj_t *msgbox_cont = lv_cont_create(window_cont, NULL);
	lv_obj_set_pos(msgbox_cont, 228, 103);
	lv_obj_set_size(msgbox_cont, 648, 441);
	lv_obj_set_style_local_bg_color(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00131D));
	lv_obj_set_style_local_bg_opa(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 18);
	lv_obj_t *img = lv_img_create(msgbox_cont, NULL);
	lv_obj_set_pos(img, 0, 82);
	lv_obj_set_size(img, 648, 3);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_MSGBOX_LINE_PNG);
	lv_img_set_src(img, &info1);
	lv_obj_t *window_head_label = lv_label_create(msgbox_cont, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(window_head_label, str);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_align(window_head_label, msgbox_cont, LV_ALIGN_CENTER, 0, -30);
	msgbox_task_t = lv_task_create(tuya_switch_msgbox_task, 2000, LV_TASK_PRIO_HIGH, NULL);
}

static void tuya_switch_yes_up(lv_obj_t *obj)
{
	lv_obj_t *window_cont = lv_obj_get_child_form_id(lv_scr_act(), 789);
	if (window_cont != NULL)
	{
		lv_obj_del(window_cont);
	}
	extern void watchdog_open(void);
	extern void watch_dog_close(void);
	int result;
	watch_dog_close();
	if ((result = tuya_key_and_uuid_check(user_data_get()->tuya_info.tuya_uuid, user_data_get()->tuya_info.tuya_key, &(user_data_get()->tuya_info.index))) != TUYA_ACTION_OK)
	{
		printf("tuya_key_and_uuid_check :%d\n\r", result);
		tuya_switch_fail_msgbox_create(text_str(STR_NO_TUYA_FILE + result));
		watchdog_open();
		return;
	}
	watchdog_open();
	printf("tuya_key_and_uuid_check :succeed  uuis :%s       key:%s\n\r", user_data_get()->tuya_info.tuya_uuid, user_data_get()->tuya_info.tuya_key);
	lv_obj_t *tuya_btn = lv_obj_get_child_form_id(lv_scr_act(), 555);

	static char uuid[128] = {0};
	memset(uuid, 0, sizeof(uuid));
	sprintf(uuid, "%s", user_data_get()->tuya_info.tuya_uuid);
	Debug("uuid:%s\n", uuid);
	lv_obj_set_style_local_value_str(tuya_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, uuid);

	{
		char buf[64] = {0};
		sprintf(buf, "rm -rf  %stuya_*.db\n", TUYA_DATA_PATH);
		system(buf);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "rm -rf  %stuya_*.db_bak\n", TUYA_DATA_PATH);
		system("buf");
	}
}
static void tuya_switch_msgbox_create(void (*window_yse_btn_up)(lv_obj_t *), char *str)
{
	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	lv_obj_set_id(window_cont, 789);
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
	static btn_data btn_data1 = {0};
	btn_data1.OPS_UP = window_yse_btn_up;
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_YES));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_YES));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
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
static void info_Tuya_uuid_btn_up(lv_obj_t *obj)
{
	tuya_switch_msgbox_create(tuya_switch_yes_up, text_str(STR_REPLACE_IT));
}
static void info_Tuya_uuid_btn_create(Controls_location **coordinate)
{

	static btn_data btn_data3 = btn_data_create(NULL, NULL, NULL);
	btn_data3.OPS_UP = info_Tuya_uuid_btn_up;
	static char uuid[128] = {0};
	memset(uuid, 0, sizeof(uuid));
	sprintf(uuid, "%s", user_data_get()->tuya_info.tuya_uuid);
	lv_obj_t *btn = sys_setting_btn_create(**coordinate, uuid, text_str(STR_GET_TUYA_ID), &btn_data3, NULL, NULL);
	(*coordinate)++;
	lv_obj_set_id(btn, 555);
}

static void info_setting_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}

static void info_setting_display(void)
{
	// extern unsigned long long os_get_ms(void);
	// unsigned long long x = os_get_ms();
	Controls_location module_coordinate[] = INFO_MODULE_COORDINATE_INIT;
	Controls_location *module_p = &module_coordinate[0];
	info_img_text_display();
	info_Hardware_version_btn_create(&module_p);
	info_Firmware_version_btn_create(&module_p);
	info_Software_version_btn_create(&module_p);
	info_Outdoor_version_btn_create(&module_p);

#ifdef DOOR_COMPILE_TIME
	info_Outdoor_compile_btn_create(&module_p);
#endif

	info_Relesse_date_btn_create(&module_p);
	info_SD_remain_space_btn_create(&module_p);
	info_Tuya_pid_btn_create(&module_p);
	if (wifi_usb_module_enable())
		info_Tuya_uuid_btn_create(&module_p);

	home_back_btn_create(info_setting_back_btn_up, NULL);
}

static void LAYOUT_ENETER_FUNC(setting_info)
{
	Debug("================================\n\r");
	setting_bg_display();
	info_setting_display();
}

static void LAYOUT_QUIT_FUNC(setting_info)
{
	Debug("================================\n\r");
	if (msgbox_task_t != NULL)
	{
		lv_task_del(msgbox_task_t);
		msgbox_task_t = NULL;
	}
	user_data_save();
}

CREATE_LAYOUT(setting_info);
