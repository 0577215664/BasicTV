#include "tv_audio_mixer.h"
#include "tv_audio.h"

#include "../id/id.h"
#include "../id/id_api.h"
#include "../tv/transcode/tv_transcode.h"
#include "tv_frame_audio.h"
#include "tv_item.h"
#include "tv_window.h"
#include "tv_channel.h"

void tv_audio_mixer_init(){
	if(Mix_OpenAudio(
		   TV_AUDIO_DEFAULT_SAMPLING_RATE,
		   AUDIO_U16SYS,
		   1,
		   4096) < 0){
		print("can't open audio", P_WARN);
	}
}

// start play time (compare to real), end play time (compare to real), frame (for duplication prevention), WAVE file
static std::vector<std::tuple<uint64_t, uint64_t, id_t_, Mix_Chunk*> > buffer;

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
		const id_t_ tmp_id =
			frame_vector[i].first;
		tv_frame_audio_t *frame_audio_ptr =
			PTR_DATA(tmp_id,
				 tv_frame_audio_t);
		PRINT_IF_NULL(frame_audio_ptr, P_WARN);
		tv_audio_prop_t wav_audio_prop;
		wav_audio_prop.set_format(
			TV_AUDIO_FORMAT_WAV);
		wav_audio_prop.set_flags(
			TV_AUDIO_PROP_FORMAT_ONLY);
		std::vector<std::vector<uint8_t> > wave_buffer =
			transcode::audio::frames::to_codec(
				{frame_audio_ptr->id.get_id()},
				&wav_audio_prop);
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
		if(std::find_if(
			   buffer.begin(),
			   buffer.end(),
			   [&tmp_id](const std::tuple<uint64_t, uint64_t, id_t_, Mix_Chunk* > &elem){
				   return std::get<2>(elem) == tmp_id;
			   }) == buffer.end()){
			print("adding a buffer", P_VAR);
			buffer.push_back(
				std::make_tuple(
					frame_audio_ptr->get_start_time_micro_s()+frame_vector[i].second,
					frame_audio_ptr->get_end_time_micro_s()+frame_vector[i].second,
					tmp_id,
					chunk));
			Mix_PlayChannel(-1, chunk, 0);
		}
	}
}

static void tv_audio_mixer_cleanup(){
	for(uint64_t i = 0;i < buffer.size();i++){
		if(std::get<1>(buffer[i]) < get_time_microseconds()){
			Mix_FreeChunk(
				std::get<3>(buffer[i]));
			std::get<3>(buffer[i]) = nullptr;
			buffer.erase(
				buffer.begin()+i);
			i--;
		}
	}
}

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
		P_V(window_ptr->get_timestamp_offset(), P_VAR);
		frame_audio_ptr =
			PTR_DATA(new_id,
				 tv_frame_audio_t);
		CONTINUE_IF_NULL(frame_audio_ptr, P_WARN);
		retval.push_back(
			std::make_pair(new_id,
				       window_ptr->get_timestamp_offset()+frame_audio_ptr->get_end_time_micro_s()));
	}
	return retval;
}

void tv_audio_mixer_loop(){
	Mix_Volume(-1, MIX_MAX_VOLUME);
	tv_audio_mixer_add_and_start(
		tv_audio_mixer_find_frames());
	tv_audio_mixer_cleanup();
}

void tv_audio_mixer_close(){
	
}
