#ifndef TV_AUDIO_H
#define TV_AUDIO_H
#include "../../id/id.h"
#include "../../util.h"
#include <algorithm> // std::reverse

// 48000 is the default for Opus
#define TV_AUDIO_DEFAULT_SAMPLING_RATE 48000
#define TV_AUDIO_DEFAULT_BIT_DEPTH 16
#define TV_AUDIO_DEFAULT_CHANNEL_COUNT 1
/*
  Size of chunk set to speakers at the same time.
  If it is set too high, latencies will be a problem
  If it is set too low, too much computing time will be used

  256 is pretty low, but I think even Raspberry Pis can handle it
*/
#define TV_AUDIO_DEFAULT_CHUNK_SIZE 256

// shouldn't ever be the case
#define TV_AUDIO_FORMAT_UNDEFINED 0
// WAV is only used for segmenting into SDL
#define TV_AUDIO_FORMAT_WAV 1
#define TV_AUDIO_FORMAT_WAVE TV_AUDIO_FORMAT_WAV
// default, works pretty well, nice licensing
#define TV_AUDIO_FORMAT_OPUS 2
// not used, but planned (soon enough)
#define TV_AUDIO_FORMAT_FLAC 3
#define TV_AUDIO_FORMAT_MP3 4

#define TV_AUDIO_PROP_FORMAT_ONLY (1 << 0)

/*
  TODO: put tv_audio_prop_t first
 */
// Some flags that are Opus CTLs are already covered in the given
// variables (sampling frequency, bit depth, and bit rate). These
// are pulled from tv_audio_prop_t, clipped to the closest valid
// value, and used directly from there

/*
  Audio properties:
  This is non-exportbale, non-networkable  type that is used for reading,
  writing, and converting in audio streams.

  tv_audio_prop_t must contain all information for properly creating a
  decoder, since it's the only data type passed to the initializer function
*/

struct tv_audio_prop_t{
private:
	uint8_t format = 0;
	uint8_t flags = 0;
	// Only guaranteed to be filled on raw sample sets
	uint8_t bit_depth = 0;
	uint32_t bit_rate = 0;
	uint32_t sampling_freq = 0;
	uint8_t channel_count = 0;

	// since all data is encoded in some way, we can NEVER actually directly
	// calculate the frame duration from the aforementioned variables
	uint32_t snippet_duration_micro_s = 0;
	
	// 32 bits for an CTL and 32 bits for a value
	// Opus seems pretty set on 32-bits too
	std::vector<uint64_t> encoder_flags;

public:
	bool operator==(const tv_audio_prop_t& rhs){
		return format == rhs.format &&
		flags == rhs.flags &&
		bit_depth == rhs.bit_depth &&
		bit_rate == rhs.bit_rate &&
		sampling_freq == rhs.sampling_freq &&
		channel_count == rhs.channel_count &&
		snippet_duration_micro_s == rhs.snippet_duration_micro_s &&
		encoder_flags == rhs.encoder_flags;
	}
	bool operator!=(const tv_audio_prop_t &rhs){
		return !(*this == rhs);
	}
	void list_virtual_data(data_id_t *id);
	tv_audio_prop_t();
	~tv_audio_prop_t();
	void add_encoder_flag(uint32_t codec_var, uint32_t codec_val){
		encoder_flags.push_back(
			(uint64_t)(codec_var) & ((uint64_t)codec_val << (uint64_t)32));
	}
	GET_SET_S(encoder_flags, std::vector<uint64_t>);
	GET_SET_S(flags, uint32_t);
	GET_SET_S(format, uint8_t);
	GET_SET_S(sampling_freq, uint32_t);
	GET_SET_S(bit_depth, uint8_t);
	GET_SET_S(bit_rate, uint32_t);
	GET_SET_S(channel_count, uint8_t);
	GET_SET_S(snippet_duration_micro_s, uint32_t);
};

#define PRINT_AUDIO_PROP(audio_prop) P_V((audio_prop).get_sampling_freq(), P_VAR); \
	P_V((audio_prop).get_bit_depth(), P_VAR);			\
	P_V((audio_prop).get_channel_count(), P_VAR);			\
	
#endif
