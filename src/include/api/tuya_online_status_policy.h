#ifndef TUYA_ONLINE_STATUS_POLICY_H
#define TUYA_ONLINE_STATUS_POLICY_H

#include <stdbool.h>

static inline bool tuya_online_status_from_callback(int mqtt_status)
{
	return mqtt_status != 0;
}

#endif
