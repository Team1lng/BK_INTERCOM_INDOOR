#ifndef _TUYA_STREAM_KEEPALIVE_POLICY_H_
#define _TUYA_STREAM_KEEPALIVE_POLICY_H_

#include <stdbool.h>

static inline bool tuya_stream_keepalive_due(bool tuya_client_active,
											  unsigned long long now_ms,
											  unsigned long long last_send_ms,
											  unsigned long long interval_ms)
{
	if (!tuya_client_active)
	{
		return false;
	}

	return last_send_ms == 0 || now_ms - last_send_ms >= interval_ms;
}

#endif
