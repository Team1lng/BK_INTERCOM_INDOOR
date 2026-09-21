/**
* @file      tuya_ipc_log_seq.h
* @brief     ipc log seq upload api
* @version   1.0
* @date      2021-11-19
*
* copyright  Copyright (c) tuya.inc 2021
*/
#ifndef __IPC_LOG_SEQ_H__
#define __IPC_LOG_SEQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_iot_adapter.h"

#define IPC_INSERT_LOG_SEQ_NULL(ls,seq)           tuya_iot_adapter_log_seq_insert_log(ls, seq, LDT_NULL, NULL)
#define IPC_INSERT_LOG_SEQ_DEC(ls,seq,dec)        tuya_iot_adapter_log_seq_insert_log(ls, seq, LDT_DEC, (VOID_T*)dec)
#define IPC_INSERT_LOG_SEQ_HEX(ls,seq,hex)        tuya_iot_adapter_log_seq_insert_log(ls, seq, LDT_HEX, (VOID_T*)hex)
#define IPC_INSERT_LOG_SEQ_TM(ls,seq,tm)          tuya_iot_adapter_log_seq_insert_log(ls, seq, LDT_TIMESTAMP, (VOID_T*)tm)
//string max length is STR_DATA_MAX_LENGTH
#define IPC_INSERT_LOG_SEQ_STR(ls,seq,str)        tuya_iot_adapter_log_seq_insert_log(ls, seq, LDT_STRING, (VOID_T*)str)
#define IPC_INSERT_ERROR_LOG_SEQ_NULL(ls,seq)     tuya_iot_adapter_log_seq_insert_error_log(ls, seq, LDT_NULL, NULL)
#define IPC_INSERT_ERROR_LOG_SEQ_DEC(ls,seq,dec)  tuya_iot_adapter_log_seq_insert_error_log(ls, seq, LDT_DEC, (VOID_T*)dec)
#define IPC_INSERT_ERROR_LOG_SEQ_HEX(ls,seq,hex)  tuya_iot_adapter_log_seq_insert_error_log(ls, seq, LDT_HEX, (VOID_T*)hex)
#define IPC_INSERT_ERROR_LOG_SEQ_TM(ls,seq,tm)    tuya_iot_adapter_log_seq_insert_error_log(ls, seq, LDT_TIMESTAMP, (VOID_T*)tm)
//string max length is STR_DATA_MAX_LENGTH
#define IPC_INSERT_ERROR_LOG_SEQ_STR(ls,seq,str)  tuya_iot_adapter_log_seq_insert_error_log(ls, seq, LDT_STRING, (VOID_T*)str)
#define IPC_INSERT_REPORT_LOG_SEQ_NULL(ls,seq)    tuya_iot_adapter_log_seq_insert_report_log(ls, seq, LDT_NULL, NULL)
#define IPC_INSERT_REPORT_LOG_SEQ_DEC(ls,seq,dec) tuya_iot_adapter_log_seq_insert_report_log(ls, seq, LDT_DEC, (VOID_T*)dec)
#define IPC_INSERT_REPORT_LOG_SEQ_HEX(ls,seq,hex) tuya_iot_adapter_log_seq_insert_report_log(ls, seq, LDT_HEX, (VOID_T*)hex)
#define IPC_INSERT_REPORT_LOG_SEQ_TM(ls,seq,tm)   tuya_iot_adapter_log_seq_insert_report_log(ls, seq, LDT_TIMESTAMP, (VOID_T*)tm)
//string max length is STR_DATA_MAX_LENGTH
#define IPC_INSERT_REPORT_LOG_SEQ_STR(ls,seq,str) tuya_iot_adapter_log_seq_insert_report_log(ls, seq, LDT_STRING, (VOID_T*)str)

#ifdef __cplusplus
}
#endif

#endif

