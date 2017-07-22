#include "tv_sink_audio_hardware.h"
#include "../../tv_frame_audio.h"
#include "../../tv_api.h"
#include "../../transcode/tv_transcode.h"

#include "../../tv.h"

#include <portaudio.h>

static PaStream *stream = nullptr;
static PaStreamParameters output_parameters;
static bool pa_init = false;

// start_time_micro_s, end_time_micro_s, raw sample vector per output_parameters
// since we can't push everything back, we delete and check the raw sample size
// sample format inside playback_vector is LRLR signed 16 bit
// to increase callback response time, the playback_vector is ordered by
// broadcast time
static std::mutex playback_lock;
static std::vector<std::tuple<uint64_t, uint64_t, std::vector<uint8_t> > > playback_vector;

static int tv_sink_audio_hardware_callback(
	const void *input,
	void *output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo *timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userdata){

	int16_t *output_samples =
		reinterpret_cast<int16_t*>(
			output);
	playback_lock.lock();
	try{
		const uint64_t cur_time_micro_s =
			get_time_microseconds();
		int64_t start_playback_iter =
			-1;
		for(uint64_t i = 0;i < playback_vector.size();i++){
			std::vector<uint8_t> *samples_tmp =
				&(std::get<2>(playback_vector[i]));
			if(BETWEEN(std::get<0>(playback_vector[i]),
				   cur_time_micro_s,
				   std::get<1>(playback_vector[i]))){
				start_playback_iter = i;
				break;
			}
		}

		// TODO: probably change this for multiple channels...
		uint64_t cur_output = 0;
		while(cur_output < frameCount &&
		      start_playback_iter < playback_vector.size()){
			std::vector<uint8_t> *tmp_vector =
				&(std::get<2>(playback_vector[start_playback_iter]));
			const uint64_t copy_chunk_size =
				(tmp_vector->size() >= frameCount) ?
				   frameCount : tmp_vector->size();
			memcpy(&(output_samples[cur_output]),
			       tmp_vector->data(),
			       copy_chunk_size);
			tmp_vector->erase(
				tmp_vector->begin(),
				tmp_vector->begin()+copy_chunk_size);
		}
	}catch(...){
		print("caught an exception", P_WARN);
	}
	playback_lock.unlock();
	return paContinue;
}

static void tv_sink_audio_hardware_resort_playback_vector(){
	std::sort(
		playback_vector.begin(),
		playback_vector.end(),
		[](const std::tuple<uint64_t, uint64_t, std::vector<uint8_t> > first,
		   const std::tuple<uint64_t, uint64_t, std::vector<uint8_t> > second){
			return std::get<0>(first) < std::get<0>(second);
		});
}

TV_SINK_MEDIUM_INIT(audio_hardware){
	tv_sink_state_t *state_ptr =
		new tv_sink_state_t;
	tv_sink_audio_hardware_state_t *audio_hardware_state_ptr =
		new tv_sink_audio_hardware_state_t;
	if(pa_init == false){
		const int32_t init_retval =
			Pa_Initialize();
		ASSERT(init_retval == paNoError, P_ERR);
		CLEAR(output_parameters);
		output_parameters.device =
			Pa_GetDefaultOutputDevice();
		output_parameters.channelCount =
			1;
		output_parameters.sampleFormat =
			paInt16; // Convert from native unsigned
		output_parameters.suggestedLatency =
			Pa_GetDeviceInfo(output_parameters.device)->defaultLowInputLatency;
		output_parameters.hostApiSpecificStreamInfo =
			nullptr;

		const int32_t stream_retval =
			Pa_OpenStream(
				&stream,
				nullptr,
				&output_parameters,
				48000,
				0,
				paClipOff,
				NULL,
				NULL);
		ASSERT(stream_retval == paNoError, P_ERR);
		pa_init = true;
	}
	state_ptr->set_state_ptr(
		reinterpret_cast<void*>(
			audio_hardware_state_ptr));
	state_ptr->set_medium(
		TV_SINK_MEDIUM_AUDIO_HARDWARE);
	state_ptr->set_frame_type(
		TV_FRAME_TYPE_AUDIO);
	return state_ptr;
}

TV_SINK_MEDIUM_CLOSE(audio_hardware){
	if(state_ptr != nullptr){
		if(state_ptr->get_state_ptr() != nullptr){
			delete reinterpret_cast<tv_sink_audio_hardware_state_t*>(
				state_ptr->get_state_ptr());
			state_ptr->set_state_ptr(
				nullptr);
		}
		delete state_ptr;
		state_ptr = nullptr;
	}
}

TV_SINK_MEDIUM_PULL(audio_hardware){
	print("worry about pulling audio (marginallly) later on", P_ERR);
	return std::vector<id_t_>({});
}

static std::pair<uint64_t, uint64_t> start_end_from_frame(id_t_ frame_id){
	tv_frame_audio_t *frame_audio_ptr =
		PTR_DATA(frame_id,
			 tv_frame_audio_t);
	PRINT_IF_NULL(frame_audio_ptr, P_ERR);
	return std::make_pair(frame_audio_ptr->get_start_time_micro_s(),
			      frame_audio_ptr->get_end_time_micro_s());
}

TV_SINK_MEDIUM_PUSH(audio_hardware){
	uint32_t sampling_freq = 0;
	uint8_t bit_depth = 0;
	uint8_t channel_count = 0;
	if(frames.size() == 0){
		return;
	}
	std::vector<uint8_t> raw =
		transcode::audio::raw::unsigned_to_signed(
			transcode::audio::frames::to_raw(
				frames,
				&sampling_freq,
				&bit_depth,
				&channel_count),
			bit_depth);
	ASSERT(sampling_freq == 48000, P_ERR);
	ASSERT(bit_depth == 16, P_ERR);
	ASSERT(channel_count == 1, P_ERR);
	// TODO: make an insert for vectors
	for(uint64_t i = 0;i < frames.size();i++){
		state_ptr->add_push_history(
			frames[i]);
	}
	playback_lock.lock();
	playback_vector.push_back(
		std::make_tuple(
			start_end_from_frame(frames[0]).first,
			start_end_from_frame(frames[frames.size()-1]).second,
			raw));
	tv_sink_audio_hardware_resort_playback_vector();
	playback_lock.unlock();
	/*
	  push IDs to state
	  push decompressed to local
	 */
}
