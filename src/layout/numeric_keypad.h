#ifndef _NUMERIC_KEYPAD_H_
#define _NUMERIC_KEYPAD_H_

#define KEYBOARD_PASSWORD_LEN 4

void init_numeric_keyboard(void (*confirm_cb)(int password, int len));
#endif