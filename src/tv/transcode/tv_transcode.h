#ifndef TV_TRANSCODE_H
#define TV_TRANSCODE_H
#include "../../id/id.h"
#include "../../id/id_api.h"
#include "../../util.h"
#include "../../convert.h"

#include "../../tv/audio/tv_audio.h"
#include "../../tv/tv_frame_audio.h"
#include "../../tv/video/tv_video.h"
#include "../../tv/tv_frame_video.h"

#include "../../state.h"

#include "tv_transcode_state.h"

/*
  RULES FOR TRANSCODERS:
  
  Sampling frequency, bit depth, and channel count are defined at the
  state initialization, all raw samples coming from this function
  HAVE to match with the tv_audio_prop_t enough for a coupled
  tv_frame_audio_t to work

  Snippets are encoded frames (not saying frames to avoid confusion with
  tv_frame_*_t), and are referred to internally as double byte vectors
  (std::vector<std::vector<uint8_t> >), but these can collapse down
  as needed. Snippets are NOT internally referred to as 1D vectors,
  because it is far easier to segment packets into tv_frame_*_t
  through a 2D byte vector of independent information than a 1D
  byte vector with a (currently) hard to compute length per entry.

  Raw samples are referred to internally as std::vector<uint8_t>, passed
  as pointers to the encoder, wherein the encoded samples are deleted from
  the std::vector<uint8_t> (frame size and sample size discrepencies would
  force us to either drop samples or fill in with nonsense)
 */

/*
  STANDARDS:
  All raw audio samples coming from any decoding function are in unsigned
  system byte order
 */

struct tv_transcode_state_encode_codec_t{
private:
	uint8_t format = 0;
public:
	GET_SET_S(format, uint8_t);
	tv_transcode_state_encode_codec_t(
		uint8_t format_,
		tv_transcode_encode_state_t* (*encode_init_state_)(tv_audio_prop_t*),
		std::vector<std::vector<uint8_t> >  (*encode_samples_to_snippets_)(tv_transcode_encode_state_t*, std::vector<uint8_t>*, uint32_t, uint8_t, uint8_t),
		void (*encode_close_state_)(tv_transcode_encode_state_t *)){
		format = format_;
		encode_init_state =
			encode_init_state_;
		encode_samples_to_snippets =
			encode_samples_to_snippets_;
		encode_close_state =
			encode_close_state_;
	}

	tv_transcode_encode_state_t* (*encode_init_state)(tv_audio_prop_t*) = nullptr;
	std::vector<std::vector<uint8_t> > (*encode_samples_to_snippets)(tv_transcode_encode_state_t*, std::vector<uint8_t>*, uint32_t, uint8_t, uint8_t) = nullptr;
	void (*encode_close_state)(tv_transcode_encode_state_t *) = nullptr;
};

struct tv_transcode_state_decode_codec_t{
private:
	uint8_t format = 0;
public:
	GET_SET_S(format, uint8_t);
	tv_transcode_state_decode_codec_t(
		uint8_t format_,
		tv_transcode_decode_state_t* (*decode_init_state_)(tv_audio_prop_t*),
		std::vector<uint8_t> (*decode_snippets_to_samples_)(tv_transcode_decode_state_t*, std::vector<std::vector<uint8_t> >*, uint32_t*, uint8_t*, uint8_t*),
		void (*decode_close_state_)(tv_transcode_decode_state_t *)){
		format = format_;
		decode_init_state =
			decode_init_state_;
		decode_snippets_to_samples =
			decode_snippets_to_samples_;
		decode_close_state =
			decode_close_state_;
	}
	tv_transcode_decode_state_t* (*decode_init_state)(tv_audio_prop_t*) = nullptr;
	std::vector<uint8_t> (*decode_snippets_to_samples)(tv_transcode_decode_state_t*, std::vector<std::vector<uint8_t> >*, uint32_t*, uint8_t*, uint8_t*) = nullptr;
	void (*decode_close_state)(tv_transcode_decode_state_t *) = nullptr;
};

/*
  As long as some linkage can exist between the frame functions, the codec
  functions, and the raw samples, we should be good. 
 */

namespace transcode{
	namespace audio{
		/*
		  We can safely assume that all frames are independent of each
		  other, and they can all be decoded (since they should all
		  be loaded with transcode::audio/video::codec::to_frames or
		  equivalent)
		 */
		/*
		  Frame functions are the functions handling looking up the
		  input_decode_state and output_encode_state, so they don't
		  need to call anything per-se.

		  A null input_decode_state or output_encode_state means we
		  are starting off on a clean slate, and all of the encoded
		  data should be self-standing (i.e. create and destroy a
		  state inside the function).
		 */
		namespace frames{
			std::vector<id_t_> to_frames(
				std::vector<id_t_> frame_set,
				tv_audio_prop_t *output_audio_prop,
				uint64_t frame_duration_micro_s);
			std::vector<std::vector<uint8_t> > to_codec(
				std::vector<id_t_> frame_set,
				tv_audio_prop_t *output_audio_prop);
			std::vector<uint8_t> to_raw(
				std::vector<id_t_> frame_set,
				uint32_t *sampling_freq,
				uint8_t *bit_depth,
				uint8_t *channel_count);
		};
		namespace codec{
			std::vector<id_t_> to_frames(
				std::vector<std::vector<uint8_t> > *codec_set,
				tv_audio_prop_t *input_audio_prop,
				tv_audio_prop_t *output_audio_prop,
				uint64_t frame_duration_micro_s,
				tv_transcode_decode_state_t *input_decode_state = nullptr,
				tv_transcode_encode_state_t *output_encode_state = nullptr);
			std::vector<std::vector<uint8_t> > to_codec(
				std::vector<std::vector<uint8_t> > *codec_set,
				tv_audio_prop_t *input_audio_prop,
				tv_audio_prop_t *output_audio_prop,
				tv_transcode_decode_state_t *input_decode_state = nullptr,
				tv_transcode_encode_state_t *output_encode_state = nullptr);
			std::vector<uint8_t> to_raw(
				std::vector<std::vector<uint8_t> > *codec_set,
				tv_audio_prop_t *input_audio_prop,
				uint32_t *sampling_freq,
				uint8_t *bit_depth,
				uint8_t *channel_count,
				tv_transcode_decode_state_t *input_decode_state = nullptr);
		};
		namespace raw{
			std::vector<std::vector<uint8_t> > to_codec(
				std::vector<uint8_t> *raw_set,
				uint32_t sampling_freq,
				uint8_t bit_depth,
				uint8_t channel_count,
				tv_audio_prop_t *output_audio_prop,
				tv_transcode_encode_state_t *output_encode_state = nullptr);
			/*
			  When repacketizing is popular among all codecs, there
			  should be minimal overheads in just calling that from
			  an external function
			 */
			std::vector<uint8_t> signed_to_unsigned(
				std::vector<uint8_t> signed_payload,
				uint8_t bit_depth);
			std::vector<uint8_t> unsigned_to_signed(
				std::vector<uint8_t> unsigned_payload,
				uint8_t bit_depth);
		};
		// File interface is a lot simpler, since:
		// 1. no shared state between calls
		// 2. no specificed input audio prop (outside of file data)
		// 3. different interface and libraries are used (not simple,
		// as much as more complex if they were integrated)

		// so...
		
		// 1. prevents needing any reference to tv_transcode_encode_state_ptr
		// (via the API)
		// 2. audio prop is replaced by a file_audio_format (required
		// for simplicity, maybe I can auto-detect files one day)
		// 3. once I move to CMake, it'll be easier to segment what is
		// and what is not compiled in
		
		// TODO: allow for returning vorbis comments with the codec'd
		// information so we can bind it to a tv_item_t
		namespace file{
			std::vector<std::vector<uint8_t> > to_codec(
				std::vector<uint8_t> file_data,
				uint8_t file_audio_format, 
				tv_audio_prop_t *output_audio_prop);
			std::vector<uint8_t> to_raw(
				std::vector<uint8_t> file_data,
				uint8_t file_audio_format, 
				uint32_t *sampling_freq,
				uint8_t *bit_depth,
				uint8_t *channel_count);
		};
	};
};

extern bool audio_prop_repackageable(
	tv_audio_prop_t state_audio_prop,
	tv_audio_prop_t snippet_audio_prop);

#include "tv_transcode_wav.h"
#include "tv_transcode_opus.h"
#include "tv_transcode_audio.h"
#endif
