/*******************************************************************
 * @Descripttion   : 
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-08-27 13:36
 * @LastEditTime   : 2022-08-30 19:59
*******************************************************************/
#ifndef _LANG_XLS_H_
#define _LANG_XLS_H_

#include <stdbool.h>
//xls文件的路径
#define XLS_PATH /*"/mnt/share/LB_22_8_5/project/456.xls"*/ TTF_XLS_PATH
//xls文件的编码格式
#define CODE "UTF-8"

char *** lang_xls_init(int sheet_num);

bool lang_xls_file_state_get(void);

int lang_xls_null_str_num_get(void);

int lang_xls_language_num_get(void);

int lang_xls_str_num_get(void);

char * lang_xls_str_get(int str_num, int lang_type);

char ***lang_xls_a_row_str_get(int index);

#endif // _LANG_XLS_H_
