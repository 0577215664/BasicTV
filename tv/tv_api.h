#include "../encrypt/encrypt.h"
#include "../encrypt/encrypt_rsa.h"
#ifndef TV_API_H
#define TV_API_H

namespace tv{
	namespace chan{
		id_t_ next_id(id_t_ id, uint64_t flags = 0);
		id_t_ prev_id(id_t_ id, uint64_t flags = 0);
		id_t_ rand_id(uint64_t flags = 0);
	};
	namespace window{
		namespace bind{
			void active_stream_playback_state(
				id_t_ window_id,
				id_t_ playback_id);
		};
		namespace unbind{
			void active_stream_playback_state(
				id_t_ window_id,
				id_t_ playback_id);
		};
	};
	// layout is defined through tv_window_t
};

#endif
