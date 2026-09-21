/*******************************************************************
 * @Descripttion   : 
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-08-27 13:35
 * @LastEditTime   : 2022-10-25 09:57
*******************************************************************/
#include <xls.h>
#include <stdlib.h>
#include <string.h>
#include "lang_xls.h"

typedef struct
{
    bool xls_is_exist;//xls文件存在标志位
    int xls_null_str_num;//xls文件 在有效行列中 存在空字符的数量
    int row_total;//xls文档的有效总行数
    int col_total;//xls文档的有效总列数
}xls_info_t;



xls_info_t xls_info = {false, 0, 0, 0};

char *** buffer = NULL;
/*******************************************************************
 * @brief  : 初始化xls文件，加载语言字符串至内存
 * @return  {char ***} NULL：初始化失败  buffer地址：初始化成功，返回动态申请的三维字符数组
 * @param {int} sheet_num excel表格中的工作表序号，从零开始
*******************************************************************/
char *** lang_xls_init(int sheet_num)
{
    xlsWorkBook *pWb = NULL;
    xlsWorkSheet *pWs = NULL;

    pWb = xls_open(XLS_PATH, CODE);

    if (NULL==pWb){
        printf("File open error!\n");
        return NULL;
    }

    pWs = xls_getWorkSheet(pWb, sheet_num);//pWs指向第 sheet_num 个 Sheet
    xls_parseWorkSheet(pWs);

    xls_info.row_total = pWs->rows.lastrow + 1;
    xls_info.col_total = pWs->rows.lastcol;

    printf("===========================>>> 行数: %d   列数: %d \n", xls_info.row_total, xls_info.col_total);
    
    buffer = (char ***)malloc(sizeof(char ***) * xls_info.row_total);
    if(buffer == NULL) return NULL;

    for(int i = 0; i < xls_info.row_total; i++)
    {
        buffer[i] = (char **)malloc(sizeof(char **) * xls_info.col_total);
        if(buffer[i] == NULL) return NULL;

        for(int j = 0; j < xls_info.col_total; j++)
        {
            if((&(pWs->rows.row[i]))->cells.cell[j].str == NULL)
            {
                buffer[i][j] = (char *)malloc(5);
                if(buffer[i][j] == NULL) return NULL;

                sprintf(buffer[i][j], "NULL");
                printf("AAAAAAAAAAAAAAAAAAAAAAAAa%d,%d\t",i,j);
                xls_info.xls_null_str_num++;
            }
            else
            {
                buffer[i][j] = (char *)malloc(strlen((char *)(&(pWs->rows.row[i]))->cells.cell[j].str) + 1);
                if(buffer[i][j] == NULL) return NULL;

                strcpy(buffer[i][j], (char *)((&(pWs->rows.row[i]))->cells.cell[j].str));
            }
            // printf("%s\t", (char *)((&(pWs->rows.row[i]))->cells.cell[j].str));
        }
        // printf("\n");
    }

    printf("===========================>>> 行数: %d   列数: %d \n", xls_info.row_total, xls_info.col_total);
    xls_close_WS(pWs);
    xls_close_WB(pWb);

    xls_info.xls_is_exist = true;

    return buffer;
}
/*******************************************************************
 * @brief  : 获取xls文件状态，查看文件 是否存在 并 初始化成功
 * @return  {bool} false：文件不存在  true：文件存在
*******************************************************************/
bool lang_xls_file_state_get(void)
{
    return xls_info.xls_is_exist;
}
/*******************************************************************
 * @brief  : 获取xls文件中 在有效行列中 含有的空字符数量
 * @return  {int}
*******************************************************************/
int lang_xls_null_str_num_get(void)
{
    return xls_info.xls_null_str_num;
}
/*******************************************************************
 * @brief  : 获取xls文件中语言的数量（即列数）
 * @return  {int}
*******************************************************************/
int lang_xls_language_num_get(void)
{
    return xls_info.col_total;
}
/*******************************************************************
 * @brief  : 获取xls文件中单个字符串的数量（即行数）
 * @return  {int}
*******************************************************************/
int lang_xls_str_num_get(void)
{
    return xls_info.row_total;
}
/*******************************************************************
 * @brief  : 获取xls文件中指定单元格的字符串
 * @return  {char *}
 * @param {int} str_num 字符串的序号（行号） 从0起
 * @param {int} lang_type 字符串的语言类型（列号） 从0起
*******************************************************************/
char * lang_xls_str_get(int str_num, int lang_type)
{
    return buffer[str_num][lang_type];
}
/*******************************************************************
 * @brief  : 获取xls文件中指定行字符串的地址
 * @return  {char *}
 * @param {int} index 字符串的序号（行号） 从0起
*******************************************************************/
char ***lang_xls_a_row_str_get(int index)
{
    return &buffer[index];
}


