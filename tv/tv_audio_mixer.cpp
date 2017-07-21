#include "tv_audio_mixer.h"
#include "tv_audio.h"

#include "../id/id.h"
#include "../id/id_api.h"
#include "../tv/transcode/tv_transcode.h"
#include "tv_frame_audio.h"
#include "tv_item.h"
#include "tv_window.h"
#include "tv_channel.h"

// start play time (compare to real), end play time (compare to real), frame (for duplication prevention), WAVE file
static std::mutex buffer_lock;
static std::thread buffer_thread;
static std::vector<std::tuple<uint64_t, uint64_t, id_t_, Mix_Chunk*, bool> > buffer;

static void tv_audio_mixer_play_thread(){
	while(true){
		const uint64_t timestamp =
			get_time_microseconds();
		buffer_lock.lock();
		for(uint64_t i = 0;i < buffer.size();i++){
			if(unlikely(BETWEEN(
					    std::get<0>(buffer[i]),
					    timestamp,
					    std::get<1>(buffer[i])) &&
				    std::get<4>(buffer[i]) == false)){
				Mix_PlayChannel(
					-1,
					std::get<3>(buffer[i]),
					0);
				std::get<4>(buffer[i]) = true;
			}else if(std::get<1>(buffer[i]) > timestamp+(1*1000*1000)){
				Mix_FreeChunk(
					std::get<3>(buffer[i]));
				buffer.erase(
					buffer.begin()+i);
				i--;
			}
		}
		buffer_lock.unlock();
	}
}

void tv_audio_mixer_init(){
	if(Mix_OpenAudio(
		   TV_AUDIO_DEFAULT_SAMPLING_RATE,
		   AUDIO_U16SYS,
		   1,
		   4096) < 0){
		print("can't open audio", P_WARN);
	}
	buffer_thread =
		std::move(
			std::thread(tv_audio_mixer_play_thread));
}

static void tv_audio_mixer_assert_default(
	tv_frame_audio_t *frame_audio_ptr){
	ASSERT(frame_audio_ptr->get_audio_prop().get_sampling_freq() == TV_AUDIO_DEFAULT_SAMPLING_RATE, P_ERR);
	ASSERT(frame_audio_ptr->get_audio_prop().get_bit_depth() == TV_AUDIO_DEFAULT_BIT_DEPTH, P_ERR);
	ASSERT(frame_audio_ptr->get_audio_prop().get_channel_count() == TV_AUDIO_DEFAULT_CHANNEL_COUNT, P_ERR);
}

static void tv_audio_mixer_add_and_start(std::vector<std::pair<id_t_, uint64_t> > frame_vector){
	// needs to be in the loop, since I forget how the transcode
	// functions work exactly with TV_AUDIO_PROP_FORMAT_ONLY
	for(uint64_t i = 0;i < frame_vector.size();i++){
		tv_frame_audio_t *frame_audio_ptr =
			PTR_DATA(frame_vector[i].first,
				 tv_frame_audio_t);
		PRINT_IF_NULL(frame_audio_ptr, P_WARN);
		tv_audio_prop_t wav_audio_prop;
		wav_audio_prop.set_format(
			TV_AUDIO_FORMAT_WAV);
		wav_audio_prop.set_flags(
			TV_AUDIO_PROP_FORMAT_ONLY);

		std::vector<id_t_> load_buffer =
			frame_audio_ptr->id.get_linked_list().second;
		if(load_buffer.size() > 10){
			load_buffer.erase(
				load_buffer.begin()+10,
				load_buffer.end());
		}
		load_buffer.insert(
			load_buffer.begin(),
			frame_vector[i].first);
		for(uint64_t c = 0;c < load_buffer.size();c++){
			const id_t_ tmp_id =
				load_buffer[c];
			if(std::find_if(
				   buffer.begin(),
				   buffer.end(),
				   [&tmp_id](const std::tuple<uint64_t, uint64_t, id_t_, Mix_Chunk*, bool> &elem){
					   return std::get<2>(elem) == tmp_id;
				   }) == buffer.end()){
				tv_frame_audio_t *cache_frame_audio_ptr =
					PTR_DATA(tmp_id,
						 tv_frame_audio_t);
				PRINT_IF_NULL(cache_frame_audio_ptr, P_WARN); // bad state handling means it's gonna sound like crap for Opus
				std::vector<std::vector<uint8_t> > wave_buffer;
				// try{
				// 	if(cache_frame_audio_ptr->get_audio_prop().get_format() == TV_AUDIO_FORMAT_WAVE &&
				// 	   cache_frame_audio_ptr->get_packet_set().size() == 1){
				// 		print("quit being lazy, but just copying over raw WAVE data to SDL_mixer", P_DEBUG);
						wave_buffer =
							cache_frame_audio_ptr->get_packet_set();
				// 	}
				// 	wave_buffer =
				// 		transcode::audio::frames::to_codec(
				// 			{tmp_id},
				// 			&wav_audio_prop);
				// }catch(...){}
				if(wave_buffer.size() == 0){
					print("couldn't decode wave_buffer", P_WARN);
					continue;
				}
				ASSERT(wave_buffer.size() > 0, P_WARN);
				ASSERT(wave_buffer.at(0).size() > 0, P_WARN);
				SDL_RWops *rw =
					SDL_RWFromMem(
						wave_buffer[0].data(),
						wave_buffer[0].size());
				ASSERT(rw != nullptr, P_ERR);
				Mix_Chunk *chunk =
					Mix_LoadWAV_RW(rw, 1);
				ASSERT(chunk != nullptr, P_ERR);
				buffer_lock.lock();
				buffer.push_back(
					std::make_tuple(
						cache_frame_audio_ptr->get_start_time_micro_s()+frame_vector[i].second,
						cache_frame_audio_ptr->get_end_time_micro_s()+frame_vector[i].second,
						tmp_id,
						chunk,
						false));
				buffer_lock.unlock();
			}
		}
	}
}

// static void tv_audio_mixer_cleanup(){
// 	for(uint64_t i = 0;i < buffer.size();i++){
// 		if(std::get<1>(buffer[i]) < get_time_microseconds()){
// 			Mix_FreeChunk(
// 				std::get<3>(buffer[i]));
// 			std::get<3>(buffer[i]) = nullptr;
// 			buffer.erase(
// 				buffer.begin()+i);
// 			i--;
// 		}
// 	}
// }

static std::vector<uint8_t> tv_audio_mixer_wave_from_frame(
	tv_frame_audio_t *frame_audio_ptr){
	std::vector<uint8_t> retval;
	PRINT_IF_NULL(frame_audio_ptr, P_ERR);
	tv_audio_prop_t wav_audio_prop;
	wav_audio_prop.set_format(
		TV_AUDIO_FORMAT_WAV);
	wav_audio_prop.set_flags(
		TV_AUDIO_PROP_FORMAT_ONLY);
	return retval;
}

static std::vector<std::pair<id_t_, uint64_t> > tv_audio_mixer_find_frames(){
	std::vector<std::pair<id_t_, uint64_t> > retval;
	std::vector<id_t_> window_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_WINDOW_T);
	for(uint64_t i = 0;i < window_vector.size();i++){
		tv_window_t *window_ptr =
			PTR_DATA(window_vector[i],
				 tv_window_t);
		CONTINUE_IF_NULL(window_ptr, P_WARN);
		tv_item_t *item_ptr =
			PTR_DATA(window_ptr->get_item_id(),
				 tv_item_t);
		CONTINUE_IF_NULL(item_ptr, P_WARN);
		// temporary sanity checks
		ASSERT(item_ptr->get_frame_id_vector().size() == 1, P_ERR); // one stream
		ASSERT(item_ptr->get_frame_id_vector().at(0).size() > 0, P_ERR); // stream contents
		tv_frame_audio_t *frame_audio_ptr =
			PTR_DATA(item_ptr->get_frame_id_vector().at(0).at(0),
				 tv_frame_audio_t);
		CONTINUE_IF_NULL(frame_audio_ptr, P_WARN);
		const id_t_ new_id =
			tv_frame_scroll_to_time(
				frame_audio_ptr,
				(int64_t)get_time_microseconds()+(int64_t)window_ptr->get_timestamp_offset());
		frame_audio_ptr =
			PTR_DATA(new_id,
				 tv_frame_audio_t);
		CONTINUE_IF_NULL(frame_audio_ptr, P_WARN);
		retval.push_back(
			std::make_pair(new_id,
				       window_ptr->get_timestamp_offset()));
	}
	return retval;
}

void tv_audio_mixer_loop(){
	Mix_Volume(-1, MIX_MAX_VOLUME);
	tv_audio_mixer_add_and_start(
		tv_audio_mixer_find_frames());
	// tv_audio_mixer_cleanup();
}

void tv_audio_mixer_close(){
	
}
