#ifndef TV_AUDIO_PLAYBACK_H
#define TV_AUDIO_PLAYBACK_H
#include "../../../id/id.h"
#include "../../../util.h"

/*
  Generic interface for playback to devices and systems

  Each active stream in tv_window_t is bound to a playback (i.e. a sink).
  Each sink has a set number of active streams of a specific type that
  can be bound to it.

  frame_limits is a list of each frame type and how many can be allocated to
  the state at any given time.

  frame_list is a pair of a mapping number and the latest tv_frame_*_t. 
  This mapping number is a nice way of controlled multiplexing of any
  stream type. For audio, this would be the Vorbis I output channel mappings
  (5.1, 7.1, and higher). Video multiplexing doesn't make much sense outside
  of a software POV, which isn't a concern. 

  AO and SDL are both just playing to the audio speakers. Only one of these
  should exist in the program at any time, and subsequent inits should fail
  with an error. These don't have clearly-defined upper limits on the binding,
  but in reality can have infinite bindings.

  
 */

#define TV_AUDIO_PLAYBACK_AO 0
#define TV_AUDIO_PLAYBACK_SDL 1

struct tv_audio_playback_state_ao_t{
};

struct tv_audio_playback_state_sdl_t{
};

struct tv_audio_playback_state_rtmp_t{
private:
	id_t_ socket_id = ID_BLANK_ID;
	std::vector<id_t_> client_socket_vector;
public:
	tv_audio_playback_state_rtmp_t();
	~tv_audio_playback_state_rtmp_t();
	
};

struct tv_audio_playback_state_t{
private:
	std::vector<std::pair<uint8_t, uint64_t> > frame_limits;
	std::vector<uint64_t, id_t_> frame_list;
public:
	data_id_t id;
	GET_SET(
};

struct tv_audio_playback_medium_t{
public:
	tv_audio_playback_state_t* (*init)() = nullptr;
	void (*close)(tv_audio_playback_state_t*) = nullptr;
	
	std::vector<uint8_t>(*get_decompressed)(tv_frame_audio_t*);
	void (*play_uncompressed)(std::vector<uint8_t>);
};

struct tv_audio_playback_ao_struct_t{
	ao_sample_format ao_format;
	ao_device *ao_device_ptr = nullptr;
};

#define TV_AUDIO_PLAYBACK_INIT(medium) tv_audio_playback_state_t *


#endif
