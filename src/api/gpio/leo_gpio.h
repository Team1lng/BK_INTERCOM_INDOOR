/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-04 10:33:17
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-12-12 15:52:42
 * @FilePath: /two-wire-indoor/src/api/gpio/leo_gpio.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef LEO_GPIO_H
#define LEO_GPIO_H
#include <stdbool.h>
#include "ak_thread.h"

enum audio_open
{

    AUDIO_DOOR1 = 0X01,
    AUDIO_DOOR2,
    AUDIO_INTERCOM,
    AUDIO_CLOSE_ALL
};

void speak_enable_set(bool enable);
int check_speak_static(void); // 高电平：功放开

void unlock_gpio_set(bool enable);

void talk_led_gpio_control(bool en);

void unlock_led_gpio_control(bool en);

void msg_gpio_set(bool enable);

void backlight_open(bool enable, bool is_monitor, int brightness);

void chime_gpio_enable(void);
void chime_gpio_disable(void);
#endif
