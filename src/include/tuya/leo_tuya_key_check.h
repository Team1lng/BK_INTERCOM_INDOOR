#ifndef TUYA_KEY_CHECK_H
#define TUYA_KEY_CHECK_H
#include <stdbool.h>

bool tuya_key_uuid_check(void);
bool tuya_product_seral_write(int number);

bool tuya_uuid_and_key_get(char* uuid,char* key);
bool tuya_serial_number_get(char* serial);

#endif

