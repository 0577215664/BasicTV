#include "tv_frame_audio.h"
#include "tv_audio.h"
#include "tv_window.h"
#include "tv_channel.h"
#include "tv.h"
#include "../convert.h"
#include "../settings.h"

static uint32_t output_sampling_rate = 0;
static uint8_t output_bit_depth = 0;
static uint32_t output_chunk_size = 0;

/*
  TODO: implement a universal time state that all streams can sync up with
  (tv_time_t?)
 */

// tv_audio_channel_t is simple enough to stay in this file

static void tv_audio_wave(std::vector<uint8_t> *retval, const char *data){
	retval->push_back(((uint8_t*)data)[0]);
	retval->push_back(((uint8_t*)data)[1]);
	retval->push_back(((uint8_t*)data)[2]);
	retval->push_back(((uint8_t*)data)[3]);
}

static void tv_audio_wave(std::vector<uint8_t> *retval, uint32_t data){
#ifdef IS_BIG_ENDIAN
	throw std::runtime_error("this isn't big endian");
	data = __builtin_bswap32(data);
#endif
	retval->push_back(((uint8_t*)&data)[0]);
	retval->push_back(((uint8_t*)&data)[1]);
	retval->push_back(((uint8_t*)&data)[2]);
	retval->push_back(((uint8_t*)&data)[3]);
}

static void tv_audio_wave(std::vector<uint8_t> *retval, uint16_t data){
#ifdef IS_BIG_ENDIAN
	data = __builtin_bswap16(data);
#endif
	retval->push_back(((uint8_t*)&data)[0]);
	retval->push_back(((uint8_t*)&data)[1]);
}

static std::vector<uint8_t> tv_audio_get_wav_data_raw(std::vector<uint8_t> frame_data,
						      uint32_t sampling_freq,
						      uint8_t bit_depth){
	std::vector<uint8_t> retval;
	tv_audio_wave(&retval, "RIFF");
	tv_audio_wave(&retval, (uint32_t)(frame_data.size()+36));
	tv_audio_wave(&retval, "WAVE");
	tv_audio_wave(&retval, "fmt ");
	tv_audio_wave(&retval, (uint32_t)16); // length of data section
	tv_audio_wave(&retval, (uint16_t)1); // uncompressed PCM
	tv_audio_wave(&retval, (uint16_t)1); // channel count
	tv_audio_wave(&retval, (uint32_t)sampling_freq);
	tv_audio_wave(&retval, (uint32_t)(sampling_freq*1*bit_depth/8));
	tv_audio_wave(&retval, (uint16_t)(1*bit_depth/8)); // block align
	tv_audio_wave(&retval, (uint16_t)bit_depth);
	tv_audio_wave(&retval, "data");
	tv_audio_wave(&retval, (uint32_t)frame_data.size());
	// WAV forces this data to be in signed 16-bit form, which this isn't
	retval.insert(
		retval.end(),
		frame_data.begin(),
		frame_data.end());
	return retval;
}

static std::vector<uint8_t> tv_audio_get_wav_data_from_uncompressed(tv_frame_audio_t *frame){
	const uint32_t sampling_freq = frame->get_sampling_freq();
	const uint8_t bit_depth = frame->get_bit_depth();
	const std::vector<uint8_t> frame_data =
		frame->get_data();
	if(frame_data.size() % (bit_depth/8)){
		print("frame_data isn't the proper size", P_ERR);
	}
	return tv_audio_get_wav_data_raw(frame_data,
					 sampling_freq,
					 bit_depth);
}


static std::vector<uint8_t> tv_audio_get_wav_data_from_opus(tv_frame_audio_t *frame){
        uint32_t sampling_freq = frame->get_sampling_freq();
	uint8_t bit_depth = frame->get_bit_depth();
	std::vector<uint8_t> opus_data =
	        frame->get_data();
	OpusDecoder *decoder = opus_decoder_create(sampling_freq, 1, NULL);
	int32_t frame_size = opus_decoder_get_nb_samples(decoder,opus_data.data(),opus_data.size());
	std::vector<uint8_t> frame_data;
	frame_data.resize(frame_size*sizeof(opus_int16));
	opus_decode(decoder,opus_data.data(),opus_data.size(),(opus_int16*)frame_data.data(),frame_size,0);
	opus_decoder_destroy(decoder);
	return tv_audio_get_wav_data_raw(frame_data,
					 sampling_freq,
					 bit_depth);
}

static std::vector<uint8_t> tv_audio_get_wav_data(tv_frame_audio_t *frame){
	switch(GET_TV_FRAME_AUDIO_FORMAT(frame->get_flags())){
	case TV_FRAME_AUDIO_FORMAT_UNDEFINED:
		print("can't play undefined file", P_ERR);
		break;
	case TV_FRAME_AUDIO_FORMAT_RAW:
		return tv_audio_get_wav_data_from_uncompressed(frame);
	case TV_FRAME_AUDIO_FORMAT_OPUS:
		return tv_audio_get_wav_data_from_opus(frame);
	case TV_FRAME_AUDIO_FORMAT_FLAC:
		print("FLAC is not supported yet", P_ERR);
		break;
	default:
		print("unknown format", P_ERR);
		break;
	}
	return {};
}

static uint32_t tv_audio_sdl_format_from_depth(uint8_t bit_depth){
	switch(bit_depth){
	case 24:
		print("24-bit sound output isn't supported yet, falling back to 16-bit", P_WARN);
	case 16:
		print("using unsigned 16-bit system byte order", P_NOTE);
		return AUDIO_U16LSB;
	case 8:
		print("using unsigned 8-bit system byte order (are you sure you want to do this?)", P_WARN);
		return AUDIO_U8; // no SYS in 8-bit, is it assumed?
	default:
		print("unknown bit depth for SDL conversion, using 16-bit", P_WARN);
		return AUDIO_U16LSB;
	}
}

/*
  Loads up a chosen .wav file (sine wave) into one second chunks, writes that
  information into a tv_frame_audio_t entry (along with proper timing info), and
  links them all together with a linked list function (through std::vector<id_t_>)
*/

/*
  Return value is only used by the testing functions (currently) to set all
  of the data as non exportable
 */

std::vector<id_t_> tv_audio_load_wav(id_t_ channel_id, uint64_t start_time_micro_s, std::string file){
	Mix_Chunk *chunk =
		Mix_LoadWAV(file.c_str());
	if(chunk == nullptr){
		print("cannot load file (" + file + "):" + (std::string)Mix_GetError(), P_ERR);
	}
	/*
	  LoadWAV converts the data into the settings set in Mix_OpenAudio,
	  derived from the settings.cfg file for all around audio, so we should
	  be good on that front
	 */
	// five seconds ought to be enough
	const uint64_t frame_duration_micro_s =
		1000*1000;
	P_V(output_sampling_rate, P_SPAM);
	P_V(output_bit_depth, P_SPAM);
	const uint64_t sample_length_per_frame =
		((output_sampling_rate*output_bit_depth)/8.0)*(frame_duration_micro_s/(1000.0*1000.0));
	uint64_t current_start = 0;
	std::vector<id_t_> audio_frame_vector;
	while(current_start < chunk->alen){
		tv_frame_audio_t *audio =
			new tv_frame_audio_t;
		uint64_t length = 0;
		if(current_start + sample_length_per_frame < chunk->alen){
			print("adding full value of sample_length_per_frame", P_SPAM);
			length = sample_length_per_frame;
		}else{
			print("can't add full frame, ran out of room", P_SPAM);
			length = chunk->alen-current_start;
		}
		audio->set_data(
			std::vector<uint8_t>(
				&(chunk->abuf[current_start]),
				&(chunk->abuf[current_start+length])
				)
			);
		uint8_t audio_flags = 0;
		SET_TV_FRAME_AUDIO_FORMAT(
			audio_flags,
			TV_FRAME_AUDIO_FORMAT_RAW);
		audio->set_type(
			output_sampling_rate,
			output_bit_depth,
			audio_flags);
		audio->set_standard(start_time_micro_s+(frame_duration_micro_s*audio_frame_vector.size()),
				    frame_duration_micro_s,
				    audio_frame_vector.size());
		P_V(current_start, P_SPAM);
		P_V(length, P_SPAM);
		current_start += length;
		audio_frame_vector.push_back(
			audio->id.get_id());
	}
	id_api::linked_list::link_vector(audio_frame_vector);
	tv_channel_t *channel =
		PTR_DATA(channel_id,
			 tv_channel_t);
	if(channel == nullptr){
		print("channel is a nullptr", P_ERR);
	}
	channel->add_stream_id(audio_frame_vector[0]);
	Mix_FreeChunk(chunk);
	chunk = nullptr;
	return audio_frame_vector;
}

void tv_audio_init(){
	if(settings::get_setting("audio") == "true"){
		SDL_Init(SDL_INIT_AUDIO);
		try{
			output_sampling_rate =
				std::stoi(
					settings::get_setting(
						"snd_output_sampling_rate"));
			output_bit_depth =
				std::stoi(
					settings::get_setting(
						"snd_output_bit_depth"));
			output_chunk_size =
				std::stoi(
					settings::get_setting(
						"snd_output_chunk_size"));
		}catch(...){
			// no big problem, these values are sane (maybe not chunk size)
			print("cannot read sound settings from file, setting default", P_WARN);
			output_sampling_rate = TV_AUDIO_DEFAULT_SAMPLING_RATE;
			output_bit_depth = TV_AUDIO_DEFAULT_BIT_DEPTH;
			output_chunk_size = TV_AUDIO_DEFAULT_CHUNK_SIZE;
		}
		if(Mix_OpenAudio(output_sampling_rate,
				 tv_audio_sdl_format_from_depth(
					 output_bit_depth),
				 1,
				 output_chunk_size) < 0){
			P_V(output_sampling_rate, P_WARN);
			P_V(output_bit_depth, P_WARN);
			P_V(output_chunk_size, P_WARN);
			print("cannot open audio:" + (std::string)(Mix_GetError()), P_ERR);
		}
	}else{
		print("audio has been disabled in the settings", P_NOTE);
	}
}

/*
  First is the ID for the tv_frame_audio_t type
  Second is the timestamp (in microseconds) that the playback ends (this is
  computed from the end_time_micro_s() function in the tv_frame_audio_t type
  with the timestamp_offset of tv_window_t
  Third is the Mix_Chunk that SDL_mixer uses

  To prevent blips on older machines, I should either move towards SDL_Audio and
  feeding PCM information directly, or just add a start time with the end time,
  so there isn't a need for OTF conversions between sampling frequencies. I'm
  not worried about the little blips for now, and I haven't seen any problems
  as long as I don't use Valgrind
 */

std::vector<std::tuple<id_t_, uint64_t, Mix_Chunk*> > audio_data;

static void tv_audio_clean_audio_data(){
	const uint64_t cur_time_micro_s =
		get_time_microseconds();
	for(uint64_t i = 0;i < audio_data.size();i++){
		const uint64_t end_time_micro_s =
			std::get<1>(audio_data[i]);
		// Make sure that SDL is done with it (should use locks)
		if(end_time_micro_s < cur_time_micro_s){
			print("destroying old audio data", P_SPAM);
			Mix_Chunk **ptr =
				&std::get<2>(audio_data[i]);
			// should always not be null
			if(*ptr != nullptr){
				Mix_FreeChunk(*ptr);
				*ptr = nullptr;
				ptr = nullptr;
			}
			audio_data.erase(audio_data.begin()+i);
		}
	}
}

/*
  All frame_audios entries need to be started NOW, so don't offset anything
 */

static std::vector<id_t_> tv_audio_remove_redundant_ids(std::vector<id_t_> frame_audios){
	for(uint64_t i = 0;i < frame_audios.size();i++){
		for(uint64_t c = 0;c < audio_data.size();c++){
			if(frame_audios[i] == std::get<0>(audio_data[c])){
				frame_audios.erase(
					frame_audios.begin()+i);
				i--;
				c = 0;
			}
		}
	}
	return frame_audios;
}

static void tv_audio_add_frame_audios(std::vector<id_t_> frame_audios){
	const uint64_t cur_timestamp_microseconds =
		get_time_microseconds();
	for(uint64_t i = 0;i < frame_audios.size();i++){
		tv_frame_audio_t *audio =
			PTR_DATA(frame_audios[i],
				 tv_frame_audio_t);
		if(audio == nullptr){
			continue;
		}
		const uint64_t ttl_micro_s =
			audio->get_ttl_micro_s();
		std::vector<uint8_t> wav_data =
			tv_audio_get_wav_data(audio);
		P_V(wav_data.size(), P_SPAM);
		SDL_RWops *rw =
			SDL_RWFromMem(
				wav_data.data(),
				wav_data.size());
		if(rw == nullptr){
			print("can't allocate SDL_RWops:"+(std::string)SDL_GetError(), P_ERR);
		}
		Mix_Chunk *chunk =
			Mix_LoadWAV_RW(rw, 1);
		if(chunk == nullptr){
			print("can't load Mix_Chunk:"+(std::string)Mix_GetError(), P_ERR);
		}
		std::tuple<id_t_,
			   uint64_t,
			   Mix_Chunk*> new_data =
			std::make_tuple(
				frame_audios[i],
				cur_timestamp_microseconds+ttl_micro_s,
				chunk);
		print("appending tv_frame_audio_t tuple to audio_data", P_SPAM);
		audio_data.push_back(new_data);
		print("playing audio", P_SPAM);
		Mix_PlayChannel(-1, chunk, 0);
	}
}

static std::vector<id_t_> tv_audio_get_current_frame_audios(){
	std::vector<id_t_> windows =
		id_api::cache::get(
			"tv_window_t");
	const uint64_t cur_timestamp_micro_s =
		get_time_microseconds();
	std::vector<id_t_> frame_audios;
	for(uint64_t i = 0;i < windows.size();i++){
		tv_window_t *window =
			PTR_DATA(windows[i],
				 tv_window_t);
		if(window == nullptr){
			print("window is a nullptr", P_WARN);
			continue;
		}
		const std::vector<id_t_> active_streams =
			window->get_active_streams();
		const uint64_t play_time =
			cur_timestamp_micro_s+window->get_timestamp_offset();
		for(uint64_t i = 0;i < active_streams.size();i++){
			tv_frame_audio_t *audio_frame_tmp =
				PTR_DATA(active_streams[i],
					 tv_frame_audio_t);
			frame_audios.push_back(
				tv_frame_scroll_to_time(
					audio_frame_tmp,
					play_time));
		}
	}
	return frame_audios;
}

void tv_audio_loop(){
	Mix_Volume(-1, MIX_MAX_VOLUME); // -1 sets all channels
	tv_audio_clean_audio_data();
	tv_audio_add_frame_audios(
		tv_audio_remove_redundant_ids(
			tv_audio_get_current_frame_audios()));
}

void tv_audio_close(){
}
