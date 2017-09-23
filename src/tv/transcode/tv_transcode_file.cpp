#include "tv_transcode_file.h"
#include "tv_transcode.h"

std::vector<std::vector<uint8_t> > transcode::audio::file::to_codec(
	std::vector<uint8_t> file_data,
	uint8_t file_audio_format, 
	tv_audio_prop_t *output_audio_prop){
	if(file_audio_format == 0){
		// move away from this model later on and interpret the file
		print("file_audio_format must be explicitly stated at this point in development", P_ERR);
	}
	ASSERT(file_audio_format == TV_AUDIO_FORMAT_OPUS, P_ERR);
	uint32_t sampling_freq = 0;
	uint8_t bit_depth = 0;
	uint8_t channel_count = 0;
	std::vector<uint8_t> raw_samples =
		transcode::audio::file::to_raw(
			file_data,
			file_audio_format,
			&sampling_freq,
			&bit_depth,
			&channel_count);
	return transcode::audio::raw::to_codec(
			&raw_samples,
			sampling_freq,
			bit_depth,
			channel_count,
			output_audio_prop); // no state (default to nullptr)

}

std::vector<uint8_t> transcode::audio::file::to_raw(
	std::vector<uint8_t> file_data,
	uint8_t file_audio_format,
	uint32_t *sampling_freq,
	uint8_t *bit_depth,
	uint8_t *channel_count){
	std::vector<uint8_t> retval;
	if(file_audio_format == TV_AUDIO_FORMAT_OPUS){
		// TODO: opusfile can't do this IIRC
		// Allow direct reading of opus packets from the file into
		// the retval vector, instead of having to decode to
		// raw samples to re-encode it again
		int ogg_opus_error = 0;
		OggOpusFile *opus_file =
			op_open_memory(
				file_data.data(),
				file_data.size(),
				&ogg_opus_error);
		if(opus_file == nullptr){
			print("couldn't open the OGG Opus file, error code " + std::to_string(ogg_opus_error), P_ERR);
		}
		opus_int16 pcm[5760*2];
		int samples_read = 0;
		std::vector<uint8_t> raw_samples;
		int li = 0;
		while((samples_read =
		       op_read(opus_file,
			       &(pcm[0]),
			       5760*2,
			       &li)) > 0){
			raw_samples.insert(
				raw_samples.end(),
				(uint8_t*)(&(pcm[0])),
				(uint8_t*)(&(pcm[0])+samples_read));
		}
		ASSERT(raw_samples.size() == 0, P_WARN);
		// sane opus defaults
		*sampling_freq = 48000;
		*bit_depth = 16;
		*channel_count = op_channel_count(opus_file, li);

		op_free(opus_file);
		opus_file = nullptr;
	}	
	return retval;
}
	
