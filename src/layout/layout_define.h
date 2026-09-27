#ifndef LAYOUT_DEFINE_H
#define LAYOUT_DEFINE_H
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "ak_mem.h"

#include "../lvgl/lvgl.h"
// #include "resource/icon_font/output/font_icon_def.h"
#include "resource/rom.h"

#include "user_data.h"
#include "../lv_drivers/lv_port_disp.h"
#include "../lv_qrcode/lv_qrcode.h"

#include "../../api/freetype/lv_freetype.h"
#include "../api/network/network_common.h"
#include "../api/tcp_network/tcp_network_cmd.h"

#include "file_api.h"
#include "leo_api.h"
#include "../../api/gpio/leo_gpio.h"
#include "user_data.h"
#include "layout_interphone.h"
#include "layout_monitor.h"
#include "../api/audio/audio_play.h"
#include "../api/common/leo_audio_play.h"
#include "../api/wlan/wlan.h"
#include "../api/video/video_decode.h"
#include "lang_xls.h"

#include "tuya_sdk.h"

#include <math.h>
#include "user_obj.h"

#define CAMERA_MODULE_ENABLE
#define Hardware_version "Ver 1.0"
#define Firmware_version "Ver 1.0"

#if defined(_10_IPS_SCREEN)
#define Software_version "Ver10_" IPC_APP_VERSION
#elif defined(_8_IPS_SCREEN)
#define Software_version "Ver8_" IPC_APP_VERSION
#else
#define Software_version "Ver7_" IPC_APP_VERSION
#endif

#define Aelesse_date "2022-04-06"

typedef void (*event_pro_callback)(unsigned long arg1, unsigned long arg2);
typedef void (*tcp_event_pro_callback)(unsigned long arg1, unsigned long arg2, unsigned long arg3);

#define Debug (printf("\n\033[0;32;40m[***%s***]:%u\033[0m \t", __PRETTY_FUNCTION__, __LINE__), printf)

typedef struct
{
    void (*enter)(void);
    void (*quit)(void *target_layout);
} layout;

#define CREATE_LAYOUT(x) layout layout_##x = {            \
                             .enter = layout_##x##_enter, \
                             .quit = layout_##x##_quit};

#define pLAYOUT(x) &layout_##x

#define DEFINE_LAYOUT(x) extern layout layout_##x;

#define LAYOUT_ENETER_FUNC(x) layout_##x##_enter(void)

#define LAYOUT_QUIT_FUNC(x) layout_##x##_quit(void *target_layout)

#define BTN_PRESS_COLOR LV_COLOR_RED

#define LIST_BORDER_COLOR lv_color_hex(0X00959e)

#ifdef PUBLIC_VERSION
#if defined(INTERCOM_VERSION)
#define LANGUAGE_EN_GROUP { \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
};
#else
#define LANGUAGE_EN_GROUP { \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
    true,                   \
};
#endif
#else
#define LANGUAGE_EN_GROUP { \
    true,                   \
    false,                  \
    true,                   \
    false,                  \
    true,                   \
    false,                  \
    false,                  \
    false,                  \
    false,                  \
    false,                  \
};
#endif

typedef enum
{
    ENGLISH = 0,
    CHINESE = 1,
    GERMANY,
    HEBREW,
    POLISH,
    PORTUGUESE,
    SPANISH,
    FRENCH,
    JAPANESE,
    ITALIAN,
    DUTCH,
    LANGUAGE_TOTAL,
} language_index;

enum btn_string_id
{
    /* language type */
    STR_LANGUAGE_TYPE,
    /* layout_home */
    STR_INTERCOM,
    STR_MONITOR,
    STR_VIDEO,
    STR_EVENT,
    STR_MEDIA,
    STR_SETTING,
    STR_AT_HOME,
    STR_NOT_AT_HOME,
    STR_DORMANT,
    STR_UNLOCK,
    STR_LOCK,
    STR_GATE,
    STR_GATE1,
    STR_GATE2,
    STR_STANDBY,
    STR_PLEASE_INSERT_SD,
    STR_NO_SD_CARD,
    STR_INSET_SD_SUCCEE,
    STR_PLEASE_FORMAT_SD,
    STR_YES,
    STR_NO,
    STR_DEVICE_CONFLICT,

    /* layout_call */
    STR_LOCAL_DEVICE_ID,
    STR_CALLED_DEVICE_ID,
    STR_CALL_TIMEOUT,
    STR_WAIT_TIMEOUT,
    STR_CALL_TIMED_OUT,

    /*layout_monitor_1  */
    STR_DOOR1,
    STR_DOOR2,
    STR_CAMERA1,
    STR_CAMERA2,

    /* layout_video_list、layout_photo_list */
    STR_OPEN,
    STR_DELETE,
    STR_ALL_DELETE,
    STR_CLOSE,
    STR_FILE_CORRUPTED,
    STR_VIDEO_LIST,

    /* layout_setting */
    STR_SYSTEM_SET,
    STR_DOOR_SET,
    STR_CAMERA_SET,
    STR_NETWORK_SET,
    STR_SCENE_SET,
    STR_SENIOR_SET,
    STR_SYSTEM_INFO,

    /* layout_monitor */
    STR_APP_PREVIEW,
    STR_REC,
    STR_SNAPSHOT,
    STR_DOOR1_CALL,
    STR_DOOR2_CALL,
    STR_DEVICE_BUSY,
    STR_SD_NO_MEMORY,

    /* layout_media */
    STR_MOVIE,
    STR_MUSIC,
    STR_PHOTO,
    STR_FILES,

    /* layout_media */
    STR_CALL_RECORD,
    STR_MESSAGE_RECORD,
    STR_MOTION_RECORD,
    STR_ALARM_RECORD,

    /* layout_setting_sys */
    STR_DEVICE_ID,
    STR_TIME,
    STR_DATE_FORMAT,
    STR_CLOCK,
    STR_LANGUAGE,
    STR_KEYTONE,
    STR_DETECTION_PREVIEW,
    STR_RINGBACK,
    STR_INDOOR_UNLOCK_TIME,
    STR_UNLOCK_HINT,
    STR_ADMIN_SETTING,
    STR_YY_MM_DD,
    STR_MM_DD_YY,
    STR_DD_MM_YY,
    STR_ON,
    STR_OFF,
    STR_S,
    STR_ONLY_DEVICE_1_SET,
    STR_SLEEP_CANNOT_SET,

    /* layout_setting_admin */
    STR_ROOM_NUMBER,
    STR_ROUTER_ADDRESS,
    STR_CHANGE_PASSWORD,
    STR_RESTOR,
    STR_ADMINISTRATORS,
    STR_RESTORE_ADMIN,
    STR_CONFIRM,
    STR_CANCEL,

    /* layout_setting_door */
    STR_STATE,
    STR_UNLOCK_TIME,
    STR_UNGATE_TIME,
    STR_RECORD_MODE,
    STR_MOTION_DETECTION,
    STR_MOTION_DETECT_SENSITIVITY,
    STR_MOTION_DETECT_RECORD_MODE,
    STR_MOTION_DETECTION_DURATION,
    STR_MESSAGE,
    STR_MESSAGE_TIME,
    STR_RING_SETTING,
    STR_CLOSE_SCENE_MODE,
    STR_IN_NOT_AT_HOME,
    STR_LOW,
    STR_MEDIUM,
    STR_HIGH,
    STR_SNAP,

    /* layout_setting_camera */
    STR_CAMERA_MODEL,
    STR_DAHUA,
    STR_HIKVISION,
    STR_CAMERA_IP_ADDRESS,
    STR_ACCOUNT_NUMBER,
    STR_PASSWORD,

    /* layout_setting_ring */
    STR_RING1,
    STR_RING2,
    STR_RING3,
    STR_RING1_DOOR1,
    STR_RING2_DOOR1,
    STR_RING3_DOOR1,
    STR_RING1_DOOR2,
    STR_RING2_DOOR2,
    STR_RING3_DOOR2,
    STR_SCHEDULE,
    STR_RING_TIME,
    STR_RING_MODE,
    STR_RING_SELECT,
    STR_RING_VOLUME,
    STR_STANDARD,
    STR_CUSTOMIZED,

    /* layout_setting_wifi */
    STR_NET_PAIRING,
    STR_WLAN_PAIRING,
    STR_WIRED_PAIRING,
    STR_STATIC_ALLOC,
    STR_UDHCPC_ALLOC,
    STR_WLAN,
    STR_NETWORK_INFO,
    STR_QR_CODE_NETWORK,
    STR_ADD_MANUALLY,
    STR_DISCONNECT_WIFI,
    STR_IP_ADDRESS,
    STR_MAC,

    /* layout_add_wifi */
    STR_WIFI_ACCOUNT,
    STR_WIFI_PASSWD,

    /* layout_setting_scene */
    STR_DIGITAL_PHOTO,
    STR_DIGITAL_PHOTO_SWITCH_TIME,
    STR_BACKGROUND_MUSIC,
    STR_BACKGROUND_MUSIC_VOLUME,
    STR_MOTION_DETECTION_CLOSE,

    /* layout_setting_senior */
    STR_FORMAT_SD,
    STR_FACTORY_RESET,
    STR_FACTORY_OUTDOOR,
    STR_SOFTWARE_UPDATE,
    STR_RESTART_SYSTEM,
    STR_APP_UNLOCK_SETTINGS,
    STR_STANDBY_MODE,
    STR_WANT_FORMAT_SD,
    STR_FORMATING,
    STR_SD_SCAN,
    STR_FORMAT_SUCCE,
    STR_WANT_FACTORY_RESET,
    STR_WANT_FACTORY_OUTDOOR,
    STR_FACTORY_RESET_ING,
    STR_WANT_SOFTWARE_UPDATE,
    STR_CHECK_OUTDOOR_STATUS,
    STR_DEVICE_OFFLINE,
    STR_NO_UPGRAD_FW,
    STR_SOFTWARE_UPDATE_ING,
    STR_UPDATE_SUCCEE,
    STR_WANT_RESTART_SYSTEM,

    /* layout_setting_info */
    STR_HARDWARE_VERSION,
    STR_SOFTWARE_VERSION,
    STR_FIRMWARE_VERSION,
    STR_DOOR_1_VERSION,
    STR_DOOR_2_VERSION,
    STR_DOOR_1_COMPILE,
    STR_DOOR_2_COMPILE,
    STR_RELEASE_DATE,
    STR_SD_SIZE,
    STR_TUYA_PID,
    STR_GET_TUYA_ID,
    STR_REPLACE_IT,
    STR_NO_TUYA_FILE,
    STR_NO_UUID,
    STR_ERROR_FILE_TYPE,
    STR_FILE_RW_ERROR,

    /* layout_password_input */
    STR_PASSWORD_INPUT,
    STR_PLEASE_ENTER,
    STR_PLEASE_INPUT_PASSWORD,
    STR_PLEASE_ENTER_NEW_PASSWORD,
    STR_ENTER_CAMERA_IP_ADDR,
    STR_ENTER_CAMERA_PASSWORD,
    STR_ENTER_CAMERA_ACCOUNT,
    STR_ADD_LOCK_CARD,
    STR_ADD_GATE1_CARD,
    STR_DEL_LOCK_CARD,
    STR_DEL_GATE1_CARD,

    /* layout_add_wifi */
    STR_CONNECTING,
    STR_CONNECTION_SUCCESS,
    STR_CONNECTION_FAIL,
    STR_WIFI_NAME_EMPTY,
    STR_WIFI_PASSW_SHOT,
    STR_NOT_FIND,
    STR_PASSW_SHOT,
    STR_PASSW_LONG,
    STR_PASSW_ERROR,
    STR_INPUT_WIFI_NAME,

    /* tuya */
    STR_TUYA_CURR_DOOR1,
    STR_TUYA_DOOR1,
    STR_TUYA_CURR_DOOR2,
    STR_TUYA_DOOR2,
    STR_TUYA_CURR_CAMERA1,
    STR_TUYA_CAMERA1,
    STR_TUYA_CURR_CAMERA2,
    STR_TUYA_CAMERA2,

    /* layout_add_data & layout_data_list */
    STR_NO_NUM,
    STR_DOOR1_CARD_MANAGE,
    STR_DOOR2_CARD_MANAGE,
    STR_DOOR1_FINGER_MANAGE,
    STR_DOOR2_FINGER_MANAGE,
    STR_DOOR1_PASSW_MANAGE,
    STR_DOOR2_PASSW_MANAGE,
    STR_CARD_MANAGE,
    STR_FINGER_MANAGE,
    STR_PASSW_MANAGE,
    STR_ADD_SUCCESS,
    STR_ADD_FAILED,
    STR_PASSW_ENOUGH_BITS,
    STR_CARD_NUMBER,
    STR_FINGER_NUMBER,
    STR_PASSWORD_NUMBER,
    STR_PLEASE_ADD_FINGER,
    STR_PLEASE_DELETE_FINGER,
    STR_PLEASE_ADD_CARD,
    STR_PLEASE_DELETE_CARD,
    STR_PLEASE_ADD_PASSW,
    STR_PLEASE_DELETE_PASSW,
    STR_ILLEGAL_OPERATION,
    STR_NUM_PASS_FULL,
    STR_EXISTING_EQUIPMENT_ONLINE,

    /* weather */
    STR_REAVY_RAIN,
    STR_THUNDERSTORM,
    STR_SANDSTORM,
    STR_LIGHT_SNOW,
    STR_SNOW,
    STR_POGONIP,
    STR_RAINSTORM,
    STR_ISOLATED_SHOWERS,
    STR_FLOATING_DUST,
    STR_LIGHTNING,
    STR_LIGHT_SHOWERS,
    STR_RAIN,
    STR_RAIN_AND_SNOW,
    STR_DUST_TORNADO,
    STR_ICE_PELLET,
    STR_STRONG_SANDSTORM,
    STR_BLOWING_SAND,
    STR_LIGHT_TO_MODERATE_RAIN,
    STR_MOSTLY_CLEAR,
    STR_SUNNY,
    STR_FOG,
    STR_SHOWER,
    STR_HEAVY_SHOWERS,
    STR_HEAVY_SNOW,
    STR_TORRENTIAL_RAIN,
    STR_BLIZZARD,
    STR_HAIL,
    STR_LIGHT_TO_MODERATE_SNOW,
    STR_PARTLY_CLOUDY,
    STR_LIGHT_SNOW_SHOWERS,
    STR_MODERATE_SNOW,
    STR_OVERCAST,
    STR_HOARFROST,
    STR_HEAVY_RAIN,
    STR_THUNDERSTORMS_WITH_HAIL,
    STR_FREEZING_RAIN,
    STR_SNOW_SHOWERS,
    STR_LIGHT_RAIN,
    STR_SMOG,
    STR_MODERATE_RAIN,
    STR_CLOUDY,
    STR_THUNDER_SHOWER,
    STR_MODERATE_TO_HEAVY_RAIN,
    STR_HEAVY_TO_RAINSTORM,
    STR_SUNDAY,

    STR_WEATHER_SWITCH,
    STR_WEATHER_SCHEDULE,
    STR_PRESSURE,
    STR_HUMIDITY,
    STR_PM10,
    STR_PM25,
    STR_WEEK_SUN,
    STR_WEEK_MON,
    STR_WEEK_TUE,
    STR_WEEK_WED,
    STR_WEEK_THU,
    STR_WEEK_FRI,
    STR_WEEK_SAT,

    // #ifdef DHCP_IPCAMERA
    /******* IPCamera *******/
    STR_REGISTERED,
    STR_AVA_DEV,
    STR_REPLACE_DEV,
    STR_CONFIRM_CANCELLATION,
    STR_LINK_DEVICE_FAILED,
    STR_REG_SAME_DEVICE,
    STR_REGISTER,
    STR_CANCELLATION,
    STR_ENTER_IPC_USER,
    STR_ENTER_IPC_PWD,
    STR_WEBCAM,
    // #endif

    // #ifdef MACHINE_CHIME
    STR_CHIME_TYPE,
    STR_MECHANICAL_CHIME,
    STR_ELECTRONIC_CHIME,
    // #endif
    STR_LOAD,
    STR_ALARM_1,
    STR_ALARM_2,
    STR_LIGTH_SWITCH,
    STR_SYSYEM_BUSY,
    STR_EXIT_BUTTON,

    STR_TUYA_UNBIND,
    STR_UNBIND_HINT,
    STR_TOTAL
};
// const char *multi_lingual[STR_TOTAL][LANGUAGE_TOTAL];
#ifdef BCOM_OID_VERSION
void *btn_str(enum btn_string_id str_id);
#endif

char *text_str(enum btn_string_id str_id);
int language_total_get(void);
char *data_fmt_string_get(struct tm *time);

typedef uint8_t language;

bool goto_layout(const layout *layout);
const layout *current_layout_get(void);
const layout *prev_layout_get(void);

lv_obj_t *sdcard_insert_msg_box;

#define btn_data_create(down_ex, up_ex, user_data_ex) {.obj_tone = true,          \
                                                       .OPS_DOWN = down_ex,       \
                                                       .OPS_UP = up_ex,           \
                                                       .user_data = user_data_ex, \
                                                       .OPS_ANYTHING = NULL};

#define btn_data_up_create(x) {.obj_tone = true,  \
                               .user_data = NULL, \
                               .OPS_DOWN = NULL,  \
                               .OPS_UP = x,       \
                               .OPS_ANYTHING = NULL};

#define btn_data_up_data_create(x, user_data_ex) {.obj_tone = true,          \
                                                  .user_data = user_data_ex, \
                                                  .OPS_DOWN = NULL,          \
                                                  .OPS_UP = x,               \
                                                  .OPS_ANYTHING = NULL};
#define btn_data_anything_create(x) {.obj_tone = true,  \
                                     .user_data = NULL, \
                                     .OPS_DOWN = NULL,  \
                                     .OPS_UP = NULL,    \
                                     .OPS_ANYTHING = x};

void btn_touch_event_listen(lv_obj_t *obj);

void record_jpeg_event_register(event_pro_callback handle);

void record_video_event_register(event_pro_callback handle);

lv_obj_t *sdcard_insert_msgbox_create(char *str);

void setting_sdcard_callback(unsigned long arg1, unsigned long arg2);

void sdcard_event_register(event_pro_callback handle);

event_pro_callback device_id_repeat_register(event_pro_callback handle);

void interphone_call_event_register(event_pro_callback handle);
void upgrade_event_register(event_pro_callback handle);

void outdoor_call_event_register(event_pro_callback handle);

void motion_detect_event_register(event_pro_callback handle);

void indoor_cmd_event_register(event_pro_callback handle);

void dev_info_status_event_register(event_pro_callback handle);

void weather_status_event_register(event_pro_callback handle);

void door_chime_event_register(event_pro_callback handle);

void mechanical_key_event_register(event_pro_callback handle);

void alarm_event_register(event_pro_callback handle);

void mailbox_status_event_register(event_pro_callback handle);

void tcp_network_event_register(tcp_event_pro_callback handle);

void device_monitor_busy_register(event_pro_callback handle);

void device_gate2_unlock_register(event_pro_callback handle);

void device_adc_key_register(event_pro_callback handle);

void adc_key_event_push(unsigned long arg1, unsigned long arg2);

void device_monitor_busy_push(unsigned long arg1, unsigned long arg2);

bool dev_info_status_event_push(unsigned long arg1, unsigned long arg2);

void default_gate2_unlock_callback(unsigned long arg1, unsigned long arg2);
void tuya_event_register(event_pro_callback handle);
bool tuya_monitor_swap_event(int ch);
bool tuya_enter_monitor_push(void);
bool tuya_monitor_talk_event(bool state);

bool tuya_monitor_absent_mode_event(bool state);
bool tuya_monitor_enter_event(void);
bool tuya_monitor_quit_event(void);

lv_obj_t *home_back_btn_create(void (*up)(lv_obj_t *), void (*down)(lv_obj_t *));
lv_obj_t *home_btn_create_1(Controls_location coordinate, char *string, btn_data *btn_pdata, const void *img_src1, const void *img_src2);
lv_obj_t *home_btn_create_2(Controls_location coordinate, btn_data *btn_pdata, const void *img_src1, const void *img_src2);
lv_obj_t *sys_setting_btn_create(Controls_location coordinate, char *string1, char *string_lable, btn_data *btn_pdata, btn_data *btn_pdata1, btn_data *btn_pdata2);
void set_location(lv_obj_t *obj, int x, int y, int w, int h);

void setting_bg_display(void);
void home_bg_display(void);

void layout_call_init(void);

void device_id_repeat_func(unsigned long arg1, unsigned long arg2);

void dev_info_status_callback(unsigned long arg1, unsigned long arg2);

lv_obj_t *prompt_window;
lv_obj_t *prompt_window_create(char *str, void (*up)(lv_obj_t *obj));

int getmac(char *mac, char *device);

int net_util_get_ipaddr(char *dev, char *ipaddr);

void get_network_time(void);

void tuya_ungate2_start(void);
void tuya_ungate1_start(void);
void tuya_unlock_start(void);

void send_monitor_talk_cmd(bool talk_en);
void send_monitor_hang_cmd(void);

void lv_obj_del_reload(lv_obj_t **obj);

void door_chime_func(unsigned long arg1, unsigned long arg2);

int get_curr_data_week(void);

lv_obj_t *msg_loading_t;
lv_obj_t *msg_window_create(char *str, bool is_loading);

lv_obj_t *msgbox_animat_create(char *str, int ms);
#ifdef MACHINE_CHIME
void Mechanical_Chime_Enable(void);
void Mechanical_Chime_Disable(void);
#endif

void network_devices_enable_init(void);

DEFINE_LAYOUT(home);
DEFINE_LAYOUT(standby);
DEFINE_LAYOUT(transfer);
DEFINE_LAYOUT(setting);
DEFINE_LAYOUT(setting_sys);
DEFINE_LAYOUT(setting_door);
DEFINE_LAYOUT(setting_camera);
DEFINE_LAYOUT(setting_scene);
DEFINE_LAYOUT(setting_senior);
DEFINE_LAYOUT(setting_info);
DEFINE_LAYOUT(setting_wifi);
DEFINE_LAYOUT(setting_admin);
DEFINE_LAYOUT(setting_ring);
DEFINE_LAYOUT(video_list);
DEFINE_LAYOUT(photo_list);
DEFINE_LAYOUT(monitor_1);
DEFINE_LAYOUT(call);
DEFINE_LAYOUT(password_input);
DEFINE_LAYOUT(video);

DEFINE_LAYOUT(monitor);
DEFINE_LAYOUT(playback);
DEFINE_LAYOUT(photo);
DEFINE_LAYOUT(interphone);
DEFINE_LAYOUT(cctv);
DEFINE_LAYOUT(set_wifi);
DEFINE_LAYOUT(add_wifi);
DEFINE_LAYOUT(connect_wifi);
DEFINE_LAYOUT(set_cctv);
DEFINE_LAYOUT(tuya_register);
DEFINE_LAYOUT(media);
DEFINE_LAYOUT(event);
DEFINE_LAYOUT(record_list);
DEFINE_LAYOUT(music_list);
DEFINE_LAYOUT(media_photo);
DEFINE_LAYOUT(media_music);
DEFINE_LAYOUT(data_list);
DEFINE_LAYOUT(add_data);
DEFINE_LAYOUT(setting_ipc);
DEFINE_LAYOUT(file_list);
DEFINE_LAYOUT(family_transfer);
DEFINE_LAYOUT(device_transfer);
DEFINE_LAYOUT(dev_busy);
DEFINE_LAYOUT(time_display);

extern user_ring_info ring_attr;

// #define LEO_FUNC_TEST
#endif
