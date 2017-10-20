#ifndef TV_SINK_AUDIO_TOX_H
#define TV_SINK_AUDIO_TOX_H

/*
  From what I can gather, the reason why Tox group chats "suck" is because
  each individual person on the network has their own dedicated encrypted
  Opus stream, which if you care that much about performance, is a problem.

  HOWEVER, as it pertains to BasicTV and the model of multiple audio streams,
  we can effectively use it as multiple encrypted live input sinks over the
  internet, with a nice PCM (maybe Opus?) interface later on.

  For simplicity sake, this is how it works:
  1. The sink only records group chats right now
  2. The sink ONLY initiates friend requests, there is no auto-accepting
  3. The sink auto-accepts invites to groups

  Friend requests are initiated through a telnet console (should probably
  upgrade, but this is fine for now).
 */

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
