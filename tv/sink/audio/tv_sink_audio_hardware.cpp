#include "tv_sink_audio_hardware.h"
#include "../../tv_frame_audio.h"
#include "../../tv_api.h"
#include "../../transcode/tv_transcode.h"
#include "../../tv.h"
#include "../../../settings.h"

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
	
	ASSERT(input != nullptr, P_ERR); // get rid of unused-variable warns
	ASSERT(timeInfo != nullptr, P_ERR);
	ASSERT((statusFlags & paInputUnderflow) == 0 &&
	       (statusFlags & paInputOverflow) == 0, P_ERR); // no inputs yet
	ASSERT(userdata == nullptr, P_ERR);
	const uint8_t channel_count = 1;
	const uint64_t bytes_to_write =
		frameCount*channel_count*sizeof(int16_t);
	memset(output, 0, frameCount*channel_count*sizeof(int16_t));
	playback_lock.lock();
	try{
		const uint64_t cur_time_micro_s =
			get_time_microseconds();
		int64_t start_playback_iter =
			-1;
		for(uint64_t i = 0;i < playback_vector.size();i++){
			if(std::get<0>(playback_vector[i]) < cur_time_micro_s &&
			   std::get<1>(playback_vector[i]) > cur_time_micro_s){
				start_playback_iter = i;
				break;
			}
		}
		if(start_playback_iter >= 0){
			// TODO: write this to output directly when finished
			std::vector<uint8_t> data_to_push;
			data_to_push.reserve(bytes_to_write);
			while(data_to_push.size() < bytes_to_write){
				const uint64_t data_push_size =
					data_to_push.size();
				if(std::get<2>(playback_vector[start_playback_iter]).size() > bytes_to_write-data_to_push.size()){
					data_to_push.insert(
						data_to_push.end(),
						std::get<2>(playback_vector[start_playback_iter]).begin(),
						std::get<2>(playback_vector[start_playback_iter]).begin()+bytes_to_write-data_push_size);
					std::get<2>(playback_vector[start_playback_iter]).erase(
						std::get<2>(playback_vector[start_playback_iter]).begin(),
						std::get<2>(playback_vector[start_playback_iter]).begin()+bytes_to_write-data_push_size);
				}else{
					const uint64_t copy_amount =
						(std::get<2>(playback_vector[start_playback_iter]).size() > bytes_to_write-data_push_size) ?
						bytes_to_write-data_push_size : std::get<2>(playback_vector[start_playback_iter]).size();
					data_to_push.insert(
						data_to_push.end(),
						std::get<2>(playback_vector[start_playback_iter]).begin(),
						std::get<2>(playback_vector[start_playback_iter]).begin()+copy_amount);
					if(playback_vector.size() <= static_cast<uint64_t>(start_playback_iter+1)){
						break;
					}else if(copy_amount == std::get<2>(playback_vector[start_playback_iter]).size()){
						playback_vector.erase(
							playback_vector.begin()+start_playback_iter);
						// automatically put us at the next one
					}
						
				}
			}
			memcpy(output,
			       data_to_push.data(),
			       data_to_push.size());
			ASSERT(data_to_push.size() > 0, P_ERR);
			ASSERT(data_to_push.size() == bytes_to_write, P_WARN);
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
	ASSERT(flow_direction == TV_SINK_MEDIUM_FLOW_DIRECTION_OUT, P_ERR); // no inputs yet
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
		const PaDeviceInfo *dev_info =
			Pa_GetDeviceInfo(
				output_parameters.device);
		ASSERT(dev_info != nullptr, P_ERR);
		output_parameters.suggestedLatency =
			dev_info->defaultLowInputLatency;
		output_parameters.hostApiSpecificStreamInfo =
			nullptr;

		const uint64_t chunk_size =
			settings::get_setting_unsigned_def(
				"tv_sink_audio_hardware_chunk_size",
				8192);
		// 8192 is a bit much, but needed for the callback (std::vector)
		const int32_t stream_retval =
			Pa_OpenStream(
				&stream,
				nullptr,
				&output_parameters,
				48000,
				chunk_size,
				paClipOff,
				tv_sink_audio_hardware_callback,
				nullptr);
		ASSERT(stream_retval == paNoError, P_ERR);
		ASSERT(Pa_StartStream(stream) == paNoError, P_ERR);
		pa_init = true;
	}
	state_ptr->set_state_ptr(
		reinterpret_cast<void*>(
			audio_hardware_state_ptr));
	state_ptr->set_medium(
		TV_SINK_MEDIUM_AUDIO_HARDWARE);
	state_ptr->set_frame_type(
		TV_FRAME_TYPE_AUDIO);
	state_ptr->set_flow_direction(
		TV_SINK_MEDIUM_FLOW_DIRECTION_OUT);
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
	ASSERT(state_ptr != nullptr, P_ERR);
	ASSERT(mapping == 0, P_ERR);
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
	const std::vector<id_t_> push_history =
		state_ptr->get_push_history();
	// we have to have the frames for them to be cataloged as played
	for(uint64_t i = 0;i < frames.size();i++){
		if(PTR_ID(frames[i], ) == nullptr){
			// can't decode a null frame
			frames.erase(
				frames.begin()+i,
				frames.end());
			break;
		}
	}
	for(uint64_t i = 0;i < push_history.size();i++){
		for(uint64_t c = 0; c < frames.size();c++){
			if(push_history[i] == frames[c]){
				frames.erase(
					frames.begin()+c);
				c--;
			}
		}
	}
	if(frames.size() > 0){
		std::vector<uint8_t> raw =
			transcode::audio::frames::to_raw(
				frames,
				&sampling_freq,
				&bit_depth,
				&channel_count);
		P_V(sampling_freq, P_VAR);
		P_V(bit_depth, P_VAR);
		P_V(channel_count, P_VAR);
		// raw = transcode::audio::raw::unsigned_to_signed(
		// 	raw,
		// 	bit_depth);
		ASSERT(sampling_freq == 48000, P_ERR);
		ASSERT(bit_depth == 16, P_ERR);
		ASSERT(channel_count == 1, P_ERR);
		// TODO: make an insert for vectors
		for(uint64_t i = 0;i < frames.size();i++){
			state_ptr->add_push_history(
				frames[i]);
		}
		const std::tuple<uint64_t, uint64_t, std::vector<uint8_t> > tmp =
			std::make_tuple(
				start_end_from_frame(frames[0]).first-window_offset_micro_s,
				start_end_from_frame(frames[frames.size()-1]).second-window_offset_micro_s,
				raw);
		playback_lock.lock();
		playback_vector.push_back(
			tmp);
		tv_sink_audio_hardware_resort_playback_vector();
		P_V(playback_vector.size(), P_VAR);
		playback_lock.unlock();
	}
	/*
	  push IDs to state
	  push decompressed to local
	 */
}
