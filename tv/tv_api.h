#ifndef TV_API_H
#define TV_API_H
#include "../encrypt/encrypt.h"
#include "../encrypt/encrypt_rsa.h"

#include "sink/audio/tv_sink_audio_hardware.h"
#include "sink/tv_sink.h"

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
	namespace sink{
		namespace state{
			tv_sink_state_t *init(
				uint8_t medium,
				uint8_t flow_direction);
			void close(id_t_ state_id);
			void close(tv_sink_state_t *state_ptr);
			std::vector<id_t_> pull(
				id_t_ state_id,
				uint8_t mapping);
			std::vector<id_t_> pull(
				tv_sink_state_t *state_ptr,
				uint8_t mapping);
			void push(
				id_t_ state_id,
				int64_t window_offset_micro_s,
				std::vector<id_t_> data);
			void push(
				tv_sink_state_t *state_ptr,
				int64_t window_offset_micro_s,
				std::vector<id_t_> data);
		};
	};
	namespace request{
		id_t_ all_ids_of_channel(
			id_t_ channel_id);
	}; // specific requests with a TV-based nomenclature
	// layout is defined through tv_window_t
};

#endif
