#include "../id/id.h"
#ifndef TV_H
#define TV_H
#include <utility>
#include <chrono>

#define TV_FRAME_TYPE_AUDIO 1
#define TV_FRAME_TYPE_VIDEO 2
#define TV_FRAME_TYPE_TEXT 3
#define TV_FRAME_TYPE_NUMERICAL 4

extern void tv_init();
extern void tv_loop();
extern void tv_close();

#include "tv_api.h"
#endif
