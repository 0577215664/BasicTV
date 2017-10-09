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
		tox_new(tox)opts,
		&err_new);
	ASSERT(audio_tox_state_ptr->tox != nullptr, P_ERR);
}

#pragma message("tox doesn't save public key bindings, should do that soon")

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
