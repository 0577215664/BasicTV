#ifndef TV_AUDIO_H
#define TV_AUDIO_H
#include "../../id/id.h"
#include "../../util.h"
#include "../tv_prop.h"
#include <algorithm> // std::reverse

// 48000 is the default for Opus
#define TV_AUDIO_DEFAULT_SAMPLING_RATE 48000
#define TV_AUDIO_DEFAULT_BIT_DEPTH 16
#define TV_AUDIO_DEFAULT_CHANNEL_COUNT 1

#define TV_AUDIO_FORMAT_UNDEFINED 0
#define TV_AUDIO_FORMAT_WAV 1
#define TV_AUDIO_FORMAT_WAVE TV_AUDIO_FORMAT_WAV
#define TV_AUDIO_FORMAT_OPUS 2
#define TV_AUDIO_FORMAT_FLAC 3
#define TV_AUDIO_FORMAT_MP3 4

#define TV_AUDIO_PROP_FORMAT_ONLY TV_PROP_FORMAT_ONLY

struct tv_audio_prop_t : public tv_prop_t{
private:
	uint32_t sampling_freq = 0;
	uint8_t channel_count = 0;
	uint8_t bit_depth = 0;

	data_id_t *id = nullptr;
public:
	bool operator==(const tv_audio_prop_t& rhs){
		return bit_depth == rhs.bit_depth &&
		sampling_freq == rhs.sampling_freq &&
		channel_count == rhs.channel_count;
	}
	bool operator!=(const tv_audio_prop_t &rhs){
		return !(*this == rhs);
	}
	void list_virtual_data(data_id_t *id_);
	tv_audio_prop_t();
	~tv_audio_prop_t();
	GET_SET_V(sampling_freq, uint32_t);
	GET_SET_V(bit_depth, uint8_t);
	GET_SET_V(channel_count, uint8_t);
};

#define PRINT_AUDIO_PROP(audio_prop) P_V((audio_prop).get_sampling_freq(), P_VAR); \
	P_V((audio_prop).get_bit_depth(), P_VAR);			\
	P_V((audio_prop).get_channel_count(), P_VAR);			\
	
#endif
