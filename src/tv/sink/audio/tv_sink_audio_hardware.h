#ifndef TV_SINK_AUDIO_HARDWARE_H
#define TV_SINK_AUDIO_HARDWARE_H

#include "../tv_sink.h"
#include <portaudio.h>

struct tv_sink_audio_hardware_state_t{
public:
};

extern TV_SINK_MEDIUM_INIT(audio_hardware);
extern TV_SINK_MEDIUM_CLOSE(audio_hardware);
extern TV_SINK_MEDIUM_PUSH(audio_hardware);
extern TV_SINK_MEDIUM_PULL(audio_hardware);

#endif
