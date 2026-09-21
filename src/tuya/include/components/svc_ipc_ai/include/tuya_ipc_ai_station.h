#ifndef __TUYA_IPC_AI_STATION_H_
#define __TUYA_IPC_AI_STATION_H_

#include "tuya_ai_client.h"
#include "tuya_ipc_media.h"

typedef enum {
    E_AI_CMD_CHAT_START   = 1,
    E_AI_CMD_CHAT_FIN     = 2,
    E_AI_CMD_SERVER_VAD   = 3,
    E_AI_CMD_MOVE_AROUND  = 4,
    E_AI_CMD_MOVE_LEFT    = 5,
    E_AI_CMD_MOVE_RIGHT   = 6,
    E_AI_CMD_MOVE_UP      = 7,
    E_AI_CMD_MOVE_DOWN    = 8,
    E_AI_CMD_ASR_TEXT     = 9,    //text of automatic speech recognition
    E_AI_CMD_NLG_TEXT     = 0x0a, //text of natural language generation
    E_AI_CMD_EMOTION      = 0x0b, //emotion recognition
    E_AI_CMD_SESSION_CREATE = 0x0c, //the session has been successfully created, and you can now proceed with the conversation
    E_AI_CMD_SESSION_CLOSE = 0x0d, //the session has been closed, and the conversation has ended.
    E_AI_CMD_CUSTOM_CMD   = 0x0e, //user-defined commands
}TUYA_AI_CMD_TYPE_E;

typedef enum {
    E_UPSTREAM_AUDIO_START   = 1,
    E_UPSTREAM_AUDIO_UPLOADING = 2,
    E_UPSTREAM_AUDIO_STOP     = 3,
}TUYA_AI_UPSTREAM_AUDIO_STATE_E;

typedef struct {
    TUYA_CODEC_ID_E codec_type;
    UINT_T sample_rate; // unit: Hz
    TUYA_AUDIO_CHANNEL_E channels;
    USHORT_T bit_depth;
} TUYA_AI_AUDIO_ATTR_T;

typedef struct {
    CHAR_T id[64]; //角色id
    CHAR_T unique_code[64]; //角色唯一标识符，用于角色背景图下载
    CHAR_T name[128]; //角色名称
    CHAR_T img_url[256]; //角色图像下载地址
    CHAR_T use_lang_code[32]; //角色使用语言code
    CHAR_T use_timbre_id[64]; //角色使用音色
    CHAR_T use_timbre_support_langs[256]; //角色支持的音色语言,多个用逗号分隔
    BOOL_T default_role;//默认角色标志
} AI_ROLE_INFO_T;

/**
 * @brief         callback of ai cmd
 * @param[in]     cmd. specifically refer to TUYA_AI_CMD_TYPE_E
 * @param[in]     args. additional data for cmd
 * @return        VOID
 */
typedef VOID (*TUYA_IPC_AI_CMD_CB)(TUYA_AI_CMD_TYPE_E cmd, CONST PVOID_T args);

/**
 * @brief         callback of audio play
 * @param[in]     audio_data. if the AI voice ends, audio_data will be equal to NULL
 * @param[in]     len. len of audio data. if the AI voice ends, len will be equal to 0
 * @return        VOID
 */
typedef VOID (*TUYA_IPC_AI_AUDIO_PLAY_CB)(CONST CHAR_T *audio_data, INT_T len);

/**
 * @brief         callback of chat audio monitor cb
 * @param[in]     data: upstream audio data
 * @param[in]     len: upstream audio data length
 * @param[in]     state:please refer to TUYA_AI_UPSTREAM_AUDIO_STATE_E
 * @return        VOID
 */
typedef OPERATE_RET(*AI_CHAT_MONITOR_CB)(UCHAR_T *data, UINT_T len, TUYA_AI_UPSTREAM_AUDIO_STATE_E state);

/**
 * @brief         set callback of chat audio monitor cb for debug
 * @param[in]     cb:upsteam audio monitor cb
 * @return        VOID
 */
VOID_T tuya_ipc_ai_set_chat_monitor_cb(AI_CHAT_MONITOR_CB cb);

/**
 * @brief init ai station

 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_station_init();


/**
 * @brief uninit ai station

 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_station_uninit();

/**
 * @brief init ai chat
 * 
 * @param[in] event_cb: ai event cb
 * @param[in] audio_cb: audio play cb
 * @param[in] cloud_vad_flag. TRUE: enable cloud vad; FALSE: disable cloud vad
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_chat_init(TUYA_IPC_AI_CMD_CB event_cb, TUYA_IPC_AI_AUDIO_PLAY_CB audio_cb, BOOL_T cloud_vad_flag);

/**
 * @brief uninit ai chat
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_chat_uninit();

/**
 * @brief start a conversation
 * 
 * @param VOID 
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_station_start_act();

/**
 * @brief stop a conversation
 * 
 * @param VOID
 * 
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h 
 */
OPERATE_RET tuya_ipc_ai_station_stop_act();

/**
 * @brief break a conversation
 *
 * @param VOID
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_ipc_ai_station_break_act();

/**
 * @brief set chat pre time duration
 *
 * @param[in] chat pre time
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tuya_ipc_ai_set_chat_pre_time(INT_T pre_time_ms);

/**
 * @brief set tts format.
 *
 * @param[in] format:tts foramt.only support mp3 now.Please fill in "mp3" as the input parameter.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tuya_ipc_ai_set_tts_format(CHAR_T *format);

/**
 * @brief Initialize the binding relationship between roles and AI agents.Should be called after each reboot
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tuya_ipc_ai_initialize_role_binding();


/**
 * @brief Query the AI agent role template list
 *
 * @param[out] template_list Pointer to an array of role template information
 *
 * @param[out] template_count Pointer to store the number of templates
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tuya_ipc_ai_get_role_template_list(OUT AI_ROLE_INFO_T **template_list, OUT UINT_T *template_count);

/**
 * @brief Release Role Template List Memory
 * 
 * @param[in] template_list Role Template List Pointer 
 *
 * @return VOID_T
 */
VOID_T tuya_ipc_ai_release_role_template_list(IN AI_ROLE_INFO_T *template_list);

/**
 * @brief Bind AI agent with a specific role
 *
 * @param[in] role_id The unique identifier of the role. This parameter is optional and can be NULL.
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tuya_ipc_ai_bind_agent_role(IN CONST CHAR_T *role_id);

/**
 * @brief Query the role bound to the AI agent
 * 
 * @param[out] role_info Pointer to store the bound role information
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tuya_ipc_ai_get_bind_role(OUT AI_ROLE_INFO_T *role_info);

#endif
