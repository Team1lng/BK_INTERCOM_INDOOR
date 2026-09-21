#ifndef TUYA_VIDEO_FRAME_TYPE_H
#define TUYA_VIDEO_FRAME_TYPE_H

#include "tuya_ipc_media.h"

extern int h264_is_keyframe(const unsigned char *buffer, int len);

static inline MEDIA_FRAME_TYPE_E tuya_h264_frame_type_get(const unsigned char *data, int size)
{
    if (data == NULL || size <= 4)
    {
        return E_VIDEO_PB_FRAME;
    }

    return h264_is_keyframe(data + 4, size - 4) ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME;
}

#endif
