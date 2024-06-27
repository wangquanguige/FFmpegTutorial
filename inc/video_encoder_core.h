#ifndef VIDEO_ENCODER_CORE_H
#define VIDEO_ENCODER_CORE_H
#include <stdint.h>

int32_t init_video_encoder(const char *codec_name);

void destory_video_encoder();

int32_t encoding();

#endif