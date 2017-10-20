#ifndef TV_SINK_AUDIO_TOX_H
#define TV_SINK_AUDIO_TOX_H

/* #ifdef TOXLIBS_FOUND */
#if 1

#pragma message("compiling with the Tox input audio ink")

#include "../tv_sink.h"

#include <tox/tox.h>
#include <tox/toxav.h>
#include <tox/toxencryptsave.h>

struct tv_sink_audio_tox_state_t{
        Tox *tox = nullptr;
	ToxAV *tox_av = nullptr;
	
	uint32_t group_id = 0;
        uint64_t last_iter_time = 0;
	std::vector<uint8_t> tox_profile;

        std::vector<std::pair<std::array<uint8_t, TOX_PUBLIC_KEY_SIZE>, uint64_t> > key_stream_binding;
};

extern TV_SINK_MEDIUM_INIT(audio_tox);
extern TV_SINK_MEDIUM_CLOSE(audio_tox);
extern TV_SINK_MEDIUM_PUSH(audio_tox);
extern TV_SINK_MEDIUM_PULL(audio_tox);

#endif
#endif
