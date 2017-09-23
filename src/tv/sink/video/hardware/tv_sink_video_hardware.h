#ifndef TV_SINK_VIDEO_HARDWARE_H
#define TV_SINK_VIDEO_HARDWARE_H

#include "../../tv_sink.h"

struct tv_sink_video_hardware_state_t{
public:
};

extern TV_SINK_MEDIUM_INIT(video_hardware);
extern TV_SINK_MEDIUM_CLOSE(video_hardware);
extern TV_SINK_MEDIUM_PUSH(video_hardware);
extern TV_SINK_MEDIUM_PULL(video_hardware);


#endif
