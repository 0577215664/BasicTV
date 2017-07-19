#include "test.h"
#include "../file.h"
#include "../tv/transcode/tv_transcode.h"
#include "../tv/transcode/tv_transcode_audio.h"

static std::vector<uint8_t> get_standard_sine_wave_form(){
	std::vector<uint8_t> retval;
	for(uint64_t i = 0;i < 48000*10;i++){
		uint16_t tmp =
			(uint16_t)((long double)(sin(1000 * (2 * 3.1415) * i / 44100))*65535);
		retval.push_back((uint8_t)(tmp&0xFF));
		retval.push_back((uint8_t)((tmp>>8)&0xFF));
	}
	return retval;
}

void test::audio::opus(){
	tv_audio_prop_t current_audio_prop;
	current_audio_prop.set_format(
		TV_AUDIO_FORMAT_OPUS);
	current_audio_prop.set_flags(
		TV_AUDIO_PROP_FORMAT_ONLY);

	tv_audio_prop_t output_audio_prop;
	output_audio_prop.set_format(
		TV_AUDIO_FORMAT_OPUS);
	output_audio_prop.set_flags(
		TV_AUDIO_PROP_FORMAT_ONLY);
	
	std::vector<uint8_t> samples =
		get_standard_sine_wave_form();
	std::vector<uint8_t> samples_buffer =
		samples;
	
	uint32_t sampling_freq = 48000;
	uint8_t bit_depth = 16;
	uint8_t channel_count = 1;
	
	std::vector<std::vector<uint8_t> > encoded_payload =
		transcode::audio::raw::to_codec(
			&samples_buffer,
			sampling_freq,
			bit_depth,
			channel_count,
			&current_audio_prop);

	encoded_payload =
		transcode::audio::frames::to_codec(
			transcode::audio::codec::to_frames(
				&encoded_payload,
				&current_audio_prop,
				&output_audio_prop,
				1000*1000),
			&output_audio_prop);
	std::vector<uint8_t> decoded_payload =
		transcode::audio::codec::to_raw(
			&encoded_payload,
			&current_audio_prop,
			&sampling_freq,
			&bit_depth,
			&channel_count);

	if(decoded_payload != samples){
		P_V(samples.size(), P_VAR);
		P_V(samples_buffer.size(), P_VAR);
		P_V(encoded_payload.size(), P_VAR);
		P_V(decoded_payload.size(), P_VAR);
		print("lossy conversion took place", P_WARN);
	}
	if(decoded_payload.size() != samples.size()){
		P_V(samples.size(), P_VAR);
		P_V(samples_buffer.size(), P_VAR);
		P_V(encoded_payload.size(), P_VAR);
		P_V(decoded_payload.size(), P_VAR);
		print("different number of samples", P_WARN);
	}
	tv_transcode_state_encode_codec_t encode_codec =
		encode_codec_lookup(
			TV_AUDIO_FORMAT_WAV);
	tv_audio_prop_t wave_file_audio_prop;
	wave_file_audio_prop.set_format(
		TV_AUDIO_FORMAT_WAV);
	wave_file_audio_prop.set_snippet_duration_micro_s(
		5*1000*1000); // force this for writing to files
	tv_transcode_encode_state_t *encode_state =
		encode_codec.encode_init_state(
			&wave_file_audio_prop);
	/*
	  There is no general file-writing API for exporting encoded data,
	  and in most cases, it's more complicated than just routing the
	  encoded data to a file (Opus uses OGG metadata). However, with
	  WAV being a non-state codec, and the frames being individual files,
	  we can set the frame size to be exactly the sample size and we
	  should be OK to write it to a file.

	  However, i'm not doing that yet
	 */
	file::write_file_vector(
		"raw_opus.wav",
		encode_codec.encode_samples_to_snippets(
			encode_state,
			&samples,
			sampling_freq,
			bit_depth,
			channel_count).at(0));
	file::write_file_vector(
		"out_opus.wav",
		encode_codec.encode_samples_to_snippets(
			encode_state,
			&decoded_payload,
			sampling_freq,
			bit_depth,
			channel_count).at(0));
}

void test::audio::opus_file(){
	// std::vector<uint8_t> raw_samples;
	// int32_t ogg_opus_error;
	// OggOpusFile *opus_file =
	// 	op_open_file(
	// 		file.c_str(),
	// 		&ogg_opus_error);
	// if(opus_file == nullptr){
	// 	print("couldn't open the OGG Opus file, error code " + std::to_string(ogg_opus_error), P_ERR);
	// }
	// opus_int16 pcm[5760*2];
	// int samples_read = 0;
	// while((samples_read =
	//        op_read(opus_file,
	// 	       &(pcm[0]),
	// 	       5760*2,
	// 	       nullptr)) > 0){
	// 	raw_samples.insert(
	// 		raw_samples.end(),
	// 		(uint8_t*)(&(pcm[0])),
	// 		(uint8_t*)(&(pcm[0])+samples_read));
	// }
	// op_free(opus_file);
	// opus_file = nullptr;
}

void test::audio::wav(){
	tv_audio_prop_t current_audio_prop;
	current_audio_prop.set_format(
		TV_AUDIO_FORMAT_WAVE);
	current_audio_prop.set_flags(
		TV_AUDIO_PROP_FORMAT_ONLY);

	tv_audio_prop_t output_audio_prop;
	output_audio_prop.set_format(
		TV_AUDIO_FORMAT_WAVE);
	output_audio_prop.set_flags(
		TV_AUDIO_PROP_FORMAT_ONLY);
	
	std::vector<uint8_t> samples =
		get_standard_sine_wave_form();
	std::vector<uint8_t> samples_buffer =
		samples;
	
	uint32_t sampling_freq = 48000;
	uint8_t bit_depth = 16;
	uint8_t channel_count = 1;
	
	std::vector<std::vector<uint8_t> > encoded_payload =
		transcode::audio::raw::to_codec(
			&samples_buffer,
			sampling_freq,
			bit_depth,
			channel_count,
			&current_audio_prop);

	encoded_payload =
		transcode::audio::frames::to_codec(
			transcode::audio::codec::to_frames(
				&encoded_payload,
				&current_audio_prop,
				&output_audio_prop,
				1000*1000),
			&output_audio_prop);
	std::vector<uint8_t> decoded_payload =
		transcode::audio::codec::to_raw(
			&encoded_payload,
			&current_audio_prop,
			&sampling_freq,
			&bit_depth,
			&channel_count);

	if(decoded_payload != samples){
		P_V(samples.size(), P_VAR);
		P_V(samples_buffer.size(), P_VAR);
		P_V(encoded_payload.size(), P_VAR);
		P_V(decoded_payload.size(), P_VAR);
		print("lossy conversion took place", P_WARN);
	}
	if(decoded_payload.size() != samples.size()){
		P_V(samples.size(), P_VAR);
		P_V(samples_buffer.size(), P_VAR);
		P_V(encoded_payload.size(), P_VAR);
		P_V(decoded_payload.size(), P_VAR);
		print("different number of samples", P_WARN);
	}
	tv_transcode_state_encode_codec_t encode_codec =
		encode_codec_lookup(
			TV_AUDIO_FORMAT_WAV);
	tv_audio_prop_t wave_file_audio_prop;
	wave_file_audio_prop.set_format(
		TV_AUDIO_FORMAT_WAV);
	wave_file_audio_prop.set_snippet_duration_micro_s(
		5*1000*1000); // force this for writing to files
	tv_transcode_encode_state_t *encode_state =
		encode_codec.encode_init_state(
			&wave_file_audio_prop);
	/*
	  There is no general file-writing API for exporting encoded data,
	  and in most cases, it's more complicated than just routing the
	  encoded data to a file (Opus uses OGG metadata). However, with
	  WAV being a non-state codec, and the frames being individual files,
	  we can set the frame size to be exactly the sample size and we
	  should be OK to write it to a file.

	  However, i'm not doing that yet
	 */
	file::write_file_vector(
		"raw_wave.wav",
		encode_codec.encode_samples_to_snippets(
			encode_state,
			&samples,
			sampling_freq,
			bit_depth,
			channel_count).at(0));
	file::write_file_vector(
		"out_wave.wav",
		encode_codec.encode_samples_to_snippets(
			encode_state,
			&decoded_payload,
			sampling_freq,
			bit_depth,
			channel_count).at(0));
}
