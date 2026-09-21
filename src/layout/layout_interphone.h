#ifndef _LAYOUT_INTERPHONE_H_
#define _LAYOUT_INTERPHONE_H_

typedef enum
{
	INTERPHONE_STATUS_IDLE,
	/*已经呼出，等待应答*/
	INTERPHONE_STATUS_PUBLISH,
	INTERPHONE_STATUS_OUT,
	INTERPHONE_STATUS_IN,
	INTERPHONE_STATUS_TALK
}interphone_status_enum;

extern interphone_status_enum interphone_status;

void layout_interphone_init(void);
#endif

