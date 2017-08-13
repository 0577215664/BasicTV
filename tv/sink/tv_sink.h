#ifndef TV_SINK_H
#define TV_SINK_H
/*
  TV sinks are designed for real-time playback to software, like playing
  audio to a speaker, RTSP streaming, or any situation where this program
  initiates sending and managing the stream

  TODO: should probably be renamed to TV_SINK_STREAM, since TV_SINK_METADATA
  also exists, and has a seperate interface
*/

#include "../../id/id.h"
#include "../../util.h"

// referenced via ID through tv_window_t, so we need ID integration
struct tv_sink_state_t{
private:
	uint8_t medium = 0;
	uint8_t frame_type = 0;
	// right now, mapping isn't used for simplicity reasons, add it later
	std::vector<uint8_t> mapping; // Vorbis I channel mappings, multiple snumerical feeds, etc.
	std::vector<id_t_> push_history;
	std::vector<id_t_> pull_history;
	
	// tv_audio_prop_t, tv_video_prop_t, etc
	void *prop_ptr = nullptr;
	void *state_ptr = nullptr;

	uint8_t flow_direction = 0;
public:
	data_id_t id;
	tv_sink_state_t();
	~tv_sink_state_t();
	GET_SET(medium, uint8_t);
	GET_SET(frame_type, uint8_t);
	GET_SET(mapping, std::vector<uint8_t>);

	ADD_DEL_VECTOR(push_history, id_t_);
	GET_SET(push_history, std::vector<id_t_>);
	ADD_DEL_VECTOR(pull_history, id_t_);
	GET_SET(pull_history, std::vector<id_t_>);

	GET_SET(prop_ptr, void*);
	GET_SET(state_ptr, void*);

	GET_SET(flow_direction, uint8_t);
};

#define TV_SINK_MEDIUM_AUDIO_HARDWARE 1
#define TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT 2

#define TV_SINK_MEDIUM_FLOW_DIRECTION_IN 1
#define TV_SINK_MEDIUM_FLOW_DIRECTION_OUT 2
#define TV_SINK_MEDIUM_FLOW_DIRECTION_BOTH 3

#define TV_SINK_MEDIUM_INIT(medium) tv_sink_state_t *tv_sink_##medium##_init(uint8_t flow_direction)
#define TV_SINK_MEDIUM_CLOSE(medium) void tv_sink_##medium##_close(tv_sink_state_t* state_ptr)
#define TV_SINK_MEDIUM_PULL(medium) std::vector<id_t_> tv_sink_##medium##_pull(tv_sink_state_t* state_ptr, uint8_t mapping)
#define TV_SINK_MEDIUM_PUSH(medium) void tv_sink_##medium##_push(tv_sink_state_t *state_ptr, int64_t window_offset_micro_s, std::vector<id_t_> frames)

// TODO: bind the mappings to this once things start working

struct tv_sink_medium_t{
public:
	tv_sink_medium_t(
		uint8_t flow_direction_,
		uint8_t medium_,
		tv_sink_state_t* (*init_)(
			uint8_t flow_direction),
		void (*close_)(
			tv_sink_state_t *state_ptr),
		std::vector<id_t_> (*pull_)(
			tv_sink_state_t *state_ptr,
			uint8_t mapping), 
		void (*push_)(
			tv_sink_state_t *state_ptr,
			int64_t window_offset_micro_s,
			std::vector<id_t_> frames)){
		flow_direction = flow_direction_;
		medium = medium_;
		init = init_;
		close = close_;
		pull = pull_;
		push = push_;
	}
	uint8_t flow_direction = 0;
	uint8_t medium = 0;

	tv_sink_state_t* (*init)(
		uint8_t flow_direction) = nullptr;
	void (*close)(
		tv_sink_state_t *state_ptr) = nullptr;

	std::vector<id_t_> (*pull)(
		tv_sink_state_t *state_ptr,
		uint8_t mapping) = nullptr;
	void (*push)(
		tv_sink_state_t *state_ptr,
		int64_t window_offset_micro_s,
		std::vector<id_t_> frames) = nullptr;
};

tv_sink_medium_t tv_sink_get_medium(uint8_t medium);

namespace tv_sink{
	namespace stream{
		namespace ogg_bitstream{
			
		};
	};
};

#endif
