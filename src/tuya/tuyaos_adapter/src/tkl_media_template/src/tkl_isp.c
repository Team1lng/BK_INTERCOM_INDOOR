/**
 * @file tkl_isp.c
 * @brief isp
 * @version 1.0.0
 * @date 2021-11-08
 *
 * @copyright Copyright 2018-2021 Tuya Inc. All Rights Reserved.
 *
 */

#include "tkl_isp.h"

/**
* @brief vi load isp file
* 
* @param[in] irmode: DN mode, 0-AUTO_MODE, 1-DAY_MODE, 2-NIGHT_MODE
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_isp_load_isp_file(TKL_ISP_DN_MODE_E irmode)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi isp get antiflicker
* 
* @param[in] chn: vi chn
* @param[out] *eAntiFlicker: antiflicker mode, 0-disable,1-50hz,2-60hz,3-auto
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_isp_get_antiflicker(TKL_VI_CHN_E chn, TKL_ISP_FLICKER_TYPE_E *eAntiFlicker)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi isp set antiflicker
* 
* @param[in] chn: vi chn
* @param[in] eAntiFlicker: antiflicker mode, 0-disable,1-50hz,2-60hz,3-auto
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_isp_set_antiflicker(TKL_VI_CHN_E chn, TKL_ISP_FLICKER_TYPE_E eAntiFlicker)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi isp set color to gray
* 
* @param[in] chn: vi chn
* @param[in] gray_flag: gray_flag, 0-color video mode, 1-gray video mode
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_isp_set_color_to_gray(TKL_VI_CHN_E chn, int gray_flag)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi dnswitch get illumin
* 
* @param[in] chn: vi chn
* @param[in] irmode: DN mode, 0-AUTO_MODE, 1-DAY_MODE, 2-NIGHT_MODE
* @param[out] *illumin_result: Illumination result
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_dnswitch_get_illumin(TKL_VI_CHN_E chn, TKL_ISP_DN_MODE_E irmode, INT32_T *illumin_result)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi isp get sensor fps
* 
* @param[in] chn: vi chn
* @param[out] *sensor_fps: sensor fps (non codec fps),this fps can update sensor max exposure
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_isp_get_sensor_fps(TKL_VI_CHN_E chn, INT32_T *sensor_fps)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi isp set sensor fps
* 
* @param[in] chn: vi chn
* @param[in] sensor_fps: sensor fps (non codec fps),this interface can update sensor max exposure
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_isp_set_sensor_fps(TKL_VI_CHN_E chn, INT32_T sensor_fps)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
* @brief vi isp get param
* 
* @param[in] chn: vi chn
* @param[out] *pstISPParam: ISP param info
*
* @return OPRT_OK on success. Others on error, please refer to tkl_error_code.h
*/
OPERATE_RET tkl_vi_isp_get_param(TKL_VI_CHN_E chn, TKL_ISP_LOG_INFO_T *pstISPParam)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}
