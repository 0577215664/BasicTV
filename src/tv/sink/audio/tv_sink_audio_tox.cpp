#include "tv_sink_audio_tox.h"
#include "../../tv_frame_audio.h"
#include "../../tv_api.h"
#include "../../transcode/tv_transcode.h"
#include "../../tv.h"
#include "../../../settings.h"

#ifdef TOXLIBS_FOUND

#include <tox/tox.h>
#include <tox/toxav.h>

/*
  Any part of the program that needs to save files would use the same
  folder structure in src/ inside ~/.BasicTV (or wherever the data_folder
  is), with the exception of IDs, which are just dumped in the root
  directory for now (probably isn't a good idea)
 */


static void tv_sink_audio_tox_iterate(tv_sink_audio_tox_state_t *audio_tox_state_ptr){
        uint64_t time_milli_s = get_time_microseconds()/1000;
	// TODO: expose average iteration statistics somehow so we can under and
	// overshoot, and not always overshoot sleep
        if(time_milli_s - audio_tox_state_ptr->last_iter_time > tox_iteration_interval(audio_tox_state_ptr->tox)){
                tox_iterate(audio_tox_state_ptr->tox, nullptr);
                audio_tox_state_ptr->last_iter_time = time_milli_s;
		/*
		  Read in any Opus audio data directly and packetize it
		 */
        }
}

static void tv_sink_audio_tox_load_profile(
	tv_sink_audio_tox_state_t *audio_tox_state_ptr,
	Tox_Options *tox_opts,
        std::string path){

	try{
		audio_tox_state_ptr->tox_profile =
			file::read_file_vector(path);
	}catch(const std::exception &e){
		print("couldn't open Tox profile: " + e.what(), P_ERR);
	}
	
	TOX_ERR_NEW err_new;
	tox_opts->savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
	tox_opts->savedata_data = audio_tox_state_ptr->tox_profile.data();
	tox_opts->savedata_length = audio_tox_state_ptr->tox_profile.size();
	
	audio_tox_state_ptr->tox =
		tox_new(tox_opts,
			&err_new);
	ASSERT(audio_tox_state_ptr->tox != nullptr, P_ERR);

	if(settings::get_setting("tv_sink_audio_tox_reinvite") == "true"){
		print("reinvite enabled, waiting for invite", P_NOTE);
		
	}
}

static void tv_sink_audio_tox_update_profile(
	tv_sink_audio_tox_state_t *audio_tox_state_ptr){
	audio_tox_state_ptr->tox_profile.clear();
	const size_t tox_savesize =
		tox_get_savedata_size(
			audio_tox_state_ptr->tox);
	audio_tox_state_ptr->tox_profile =
		std::vector<uint8_t>(
			0, tox_savesize);
	tox_get_savedata(
		audio_tox_state_ptr->tox,
		audio_tox_state_ptr->tox_profile.data());
}

#pragma message("Only one tox sink can exist at one time, since the Tox profile is coded in settings.cfg")

static void tv_sink_audio_tox_save_profile(
	tv_sink_audio_tox_state_t *audio_tox_state_ptr){
	file::write_file_vector(
		settings::get_setting("data_folder") + "/" +
		settings::get_setting("tv_sink_audio_tox_profile_path"),
		audio_tox_state_ptr->tox_profile);
}

TV_SINK_MEDIUM_INIT(audio_tox){
        STD_STATE_INIT(tv_sink_state_t,
                       state_ptr,
                       tv_sink_audio_tox_state_t,
                       audio_tox_state_ptr);
        Tox_Options tox_opts;
        memset(&tox_opts, 0, sizeof(tox_opts));
        tox_options_default(&tox_opts);

	const std::string profile_path =
		settings::get_setting("data_folder") + "/"
		settings::get_setting("tv_sink_audio_tox_profile_path");
	try{
		tv_sink_audio_tox_load_profile(
			&(audio_tox_state_ptr->tox),
			profile_path);
			
	}catch(...){
		print("couldn't load Tox profile or doesn't exist, making a new", P_WARN);
	}
	audio_tox_state_ptr->tox =
		tox_new(toxopts,
			&err_new);
	ASSERT(audio_tox_state_ptr->tox != nullptr, P_ERR);

	// Create the group chat
	audio_tox_state_ptr->group_chat = new_groupchats();
	ASSERT(audio_tox_state_ptr->group_chat != nullptr, P_ERR);
	ASSERT(add_groupchat(group_chats, GROUPCHAT_TYPE_AV) != -1, P_ERR);
	
}

#pragma message("tox doesn't save public key bindings, should do that soon")

/*
  Again, since we don't have MT, and I care about callback performance in the first
  place, I'm going to mandate only one Tox instance per BasicTV instance so it
  can directly read and write to a std::vector of Opus packets in memory

  TODO: check that the PCM format is the same. Since Opus uses 48Khz at 16bit
  internally, then this should work out fine, right?
 */


// Because of VOX and PTT, we can't guarantee that all the vectors are in sync
struct tv_sink_audio_tox_audio_snippet_t{
public:
	uint64_t start_time_micro_s = 0;
	uint8_t channels = 0;
	uint32_t sampling_freq = 0;
	std::vector<uint8_t> data;
};

std::vector<tv_sink_audio_tox_audio_snippet_t> tox_audio_data;
std::mutex tox_audio_mutex;

// Opus can't guarantee all samples are compressed, so we just
// pass this as a ptr to transcoding functions and keep adding
// to the end of it
std::vector<uint8_t> tox_audio_working_samples;

static tv_sink_audio_tox_audio_callback(
	ToxAV *av,
	uint32_t friend_number,
	const int16_t *pcm,
	size_t sample_count,
	uint8_t channels,
	uint32_t sampling_rate,
	void *user_data){

	ASSERT(sampling_rate == 48000, P_ERR);
	ASSERT(channels == 1, P_ERR);
	
	tox_audio_mutex.lock();
	tox_audio_data[friend_number].insert(
		tox_audio_data[friend_number].end(),
		reinterpret_cast<const uint8_t*>(pcm),
		reinterpret_cast<const uint8_t*>(pcm)+(sample_count*sizeof(int16_t)))
	tox_audio_mutex.unlock();
	
}

// "Simple" pull just creates a 1D vector based on the collapsed PCM data of
// everything, where the "Complex" actually creates multiple streams and
// maintains a 1:1 mapping between the offset in the list and the pub key (so
// people who join once have a continuous stream on rejoins with the same ID)

// For now i'm opting for simple

static std::vector<id_t_> tv_sink_audio_tox_simple_pull(
	tv_sink_state_t *sink_state_ptr,
	tv_sink_audio_tox_state_t *audio_tox_state_ptr){

	uint64_t start_time_micro_s = 0;
	uint32_t sampling_freq = 0;
	uint8_t channels = 0;
	
	tox_audio_mutex.lock();
	for(uint64_t i = 0;i < tox_audio_data.size();i++){
		if(start_time_micro_s > tox_audio_data[i].start_time_micro_s){
			start_time_micro_s = tox_audio_data[i].start_time_micro_s;
		}
	}

	/*
	  Terrible for caching, but it shouldn't slow things down enough to
	  interfere with Tox, and that's all that matters right now

	  Also this can generate arbitrarially small frames at the end, which
	  isn't optimized, but should playback relatively fine.
	 */
	bool generating = true;
	uint64_t samples_from_start = 0;

	const uint64_t sampling_freq = 48000;
	const uint8_t bit_depth = 16;
	const uint64_t channel_count = 1;

	while(generating){
		int16_t sample = 0;
		uint64_t number_past = 0;
		for(uint64_t i = 0;i < tox_audio_data.size();i += 2){ // bit depth
			
			const uint64_t effective_time =
				samples_from_start*sampling_freq*channel_count*static_cast<uint64_t>(1000000UL);
			if(tox_audio_data[i].start_time_micro_s < effective_time){
				continue;
			}else{
				const uint64_t sample_start =
					(effective_time-start_time_micro_s)/(sampling_freq*channel*2);
				if(tox_audio_data[i].data.size() >= sample_start+1){
					number_past++;
					continue;
				}
				sample += *reinterpret_cast<int16_t*>(tox_audio_data[i].data()+sample_start);
			}
		}
		tox_audio_working_samples.push_back(
			static_cast<uint8_t>((samples & 0xFF00) >> 8));
		tox_audio_working_samples.push_back(
			static_cast<uint8_t>(samples & 0x00FF));
		
		if(number_past == tox_audio_data.size()){
			generating = false;
		}
		samples_from_start++;
	}
	tox_audio_data.clear();
	tox_audio_mutex.unlock();

	/*
	  Again, break it off into one second chunks (should make this a
	  standard interface into these blocks of code).
	 */

	std::vector<id_t_> retval;
	tv_audio_prop_t opus_audio_prop;
	opus_audio_prop.set_format(TV_AUDIO_FORAMT_OPUS);
	opus_audio_prop.set_flags(TV_AUDIO_PROP_FORMAT_ONLY);

	const std::vector<std::vector<uint8_t> > packets =
		transcode::audio::raw::to_codec(
			&tox_audio_working_samples,
			sampling_freq,
			bit_depth,
			channel_count,
			&opus_audio_prop);
	tv_frame_audio_t *frame_audio_ptr =
		new tv_frame_audio_t;

	frame_audio_ptr->set_packet_set(
		packets);
	frame_audio_ptr->set_audio_prop(
		opus_audio_prop);
	frame_audio_ptr->set_ttl_micro_s(
		opus_audio_prop->get_snippet_duration_micro_s()*packets.size());
	sink_state_ptr->set_current_frame_entry(
		sink_state_ptr->get_current_frame_entry()+1);
	frame_audio_ptr->set_frame_entry(
		sink_state_ptr->get_current_frame_entry());
	return std::vector<id_t_>({frame_audio_ptr->id.get_id()});
}

TV_SINK_MEDIUM_CLOSE(audio_tox){
        STD_STATE_GET_PTR(state_ptr,
                          tv_sink_audio_tox_state_t,
                          audio_tox_state_ptr);
	tv_sink_audio_tox_save_profile();
        tox_kill(audio_tox_state_ptr->tox);
        audio_tox_state_ptr->tox = nullptr;
        audio_tox_state_ptr = nullptr;
}

TV_SINK_MEDIUM_PULL(audio_tox){
        STD_STATE_GET_PTR(state_ptr,
                          tv_sink_audio_tox_state_t,
                          audio_tox_state_ptr);
        tv_sink_audio_tox_iterate(audio_tox_state_ptr);
	
}

TV_SINK_MEDIUM_PUSH(audio_tox){
        STD_STATE_GET_PTR(state_ptr,
                          tv_sink_audio_tox_state_t,
                          audio_tox_state_ptr);
        tv_sink_audio_tox_iterate(audio_tox_state_ptr);
}
#endif
