#ifndef TV_SINK_H
#define TV_SINK_H
/*
  Standard interface for reading to and writing from any source of
  any type.
*/

#include "../../id/id.h"
#include "../../util.h"


// referenced via ID through tv_window_t, so we need ID integration
struct tv_sink_state_t{
private:
	uint8_t medium = 0;
	uint8_t frame_type = 0;
	std::vector<uint8_t> mapping; // Vorbis I channel mappings, etc.
	std::vector<id_t_> push_history;
	std::vector<id_t_> pull_history;

	
	// tv_audio_prop_t, tv_video_prop_t, etc
	void *prop_ptr = nullptr;
	void *state_ptr = nullptr;
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
};

#define TV_SINK_MEDIUM_AUDIO_HARDWARE 1
#define TV_SINK_MEDIUM_AUDIO_RTSP 2
#define TV_SINK_MEDIUM_AUDIO_FILE 3

#define TV_SINK_MEDIUM_VIDEO_HARDWARE 4
#define TV_SINK_MEDIUM_VIDEO_RTSP 5
#define TV_SINK_MEDIUM_VIDEO_FILE 6

#define TV_SINK_MEDIUM_FLOW_DIRECTION_IN 1
#define TV_SINK_MEDIUM_FLOW_DIRECTION_OUT 2
#define TV_SINK_MEDIUM_FLOW_DIRECTION_BOTH 3

#define TV_SINK_MEDIUM_INIT(medium) tv_sink_state_t *tv_sink_##medium##_init()
#define TV_SINK_MEDIUM_CLOSE(medium) void tv_sink_##medium##_close(tv_sink_state_t* state_ptr)
#define TV_SINK_MEDIUM_PULL(medium) std::vector<id_t_> tv_sink_##medium##_pull(tv_sink_state_t* state_ptr, uint8_t mapping)
#define TV_SINK_MEDIUM_PUSH(medium) void tv_sink_##medium##_push(tv_sink_state_t *state_ptr, std::vector<id_t_> frames)

// TODO: bind the mappings to this once things start working

struct tv_sink_medium_t{
public:
	uint8_t flow_direction = 0;
	uint8_t medium = 0;

	tv_sink_state_t* (*init)() = nullptr;
	void (*close)(
		tv_sink_state_t *state_ptr) = nullptr;

	std::vector<id_t_> (*pull)(
		tv_sink_state_t *state_ptr,
		uint8_t mapping) = nullptr;
	void (*push)(
		tv_sink_state_t *state_ptr,
		std::vector<id_t_> frames) = nullptr;
};

tv_sink_medium_t tv_sink_get_medium(uint8_t medium);

#endif
