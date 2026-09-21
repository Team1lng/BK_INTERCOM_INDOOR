 /**
 * @file tkl_video_in.c
 * @brief video input
 * @version 0.1
 * @date 2021-11-04
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 * Video input configuration: for sensor, ISP or VI attribute configuration, 
 * such as image flip, anti flicker, compensation mode, profile, etc.
 */

#include "tkl_video_in.h"

/**
* @brief vi init
* 
* @param[in] pconfig: vi config
* @param[in] count: count of pconfig
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_init(TKL_VI_CONFIG_T *pconfig, INT32_T count)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi set mirror and flip
* 
* @param[in] chn: vi chn
* @param[in] flag: mirror and flip value
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_set_mirror_flip(TKL_VI_CHN_E chn, TKL_VI_MIRROR_FLIP_E flag)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi get mirror and flip
* 
* @param[in] chn: vi chn
* @param[out] flag: mirror and flip value
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_get_mirror_flip(TKL_VI_CHN_E chn, TKL_VI_MIRROR_FLIP_E *flag)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi uninit
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_uninit(VOID)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief  set sensor reg value
* 
* @param[in] chn: vi chn
* @param[in] pparam: reg info
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_sensor_reg_set( TKL_VI_CHN_E chn, TKL_VI_SENSOR_REG_CONFIG_T *parg)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief  get sensor reg value
* 
* @param[in] chn: vi chn
* @param[in] pparam: reg info
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_sensor_reg_get( TKL_VI_CHN_E chn, TKL_VI_SENSOR_REG_CONFIG_T *parg)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief detect init
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @param[in] pconfig: config
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_init(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type, TKL_VI_DETECT_CONFIG_T *p_config)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief detect start
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_start(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief detect stop
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_stop(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief get detection results
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @param[out] presult: detection results
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_get_result(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type, TKL_VI_DETECT_RESULT_T *presult)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief set detection param
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
* @param[in] pparam: detection param
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_set(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type, TKL_VI_DETECT_PARAM_T *pparam)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief detect uninit
* 
* @param[in] chn: vi chn
* @param[in] type: detect type
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_detect_uninit(TKL_VI_CHN_E chn, TKL_MEDIA_DETECT_TYPE_E type)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}
