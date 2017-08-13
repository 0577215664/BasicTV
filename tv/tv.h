#include "../id/id.h"
#ifndef TV_H
#define TV_H
#include <utility>
#include <chrono>

#include <ogg/ogg.h> // technically audio and video

#define TV_FRAME_TYPE_AUDIO 1
#define TV_FRAME_TYPE_VIDEO 2
#define TV_FRAME_TYPE_TEXT 3
#define TV_FRAME_TYPE_NUMERICAL 4

extern void tv_init();
extern void tv_loop();
extern void tv_close();

#include "tv_api.h"
#include "sink/tv_sink.h"
#include "sink/audio/tv_sink_audio_hardware.h"
#endif
