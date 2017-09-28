#include "tv_sink_audio_tox.h"
#include "../../tv_frame_audio.h"
#include "../../tv_api.h"
#include "../../transcode/tv_transcode.h"
#include "../../tv.h"
#include "../../../settings.h"

#include <tox/tox.h>
#include <tox/toxav.h>

static std::vector<uint8_t> tv_sink_audio_tox_load_profile(
        std::string path){
}

TV_SINK_MEDIUM_INIT(audio_tox){
        STD_STATE_INIT(tv_sink_state_t,
                       state_ptr,
                       tv_sink_audio_tox_state_t,
                       audio_tox_state_ptr);
        Tox_Options tox_opts;
        memset(&tox_opts, 0, sizeof(tox_opts));
        tox_options_default(&tox_opts);

        TOX_ERR_NEW err_new;
        tox_opts.savedata_type = TOX_SAVEDATA_TYPE_TOX_SAVE;
        tox_opts.savedata_data = audio_tox_state_ptr->tox_profile.data();
        tox_opts.savedata_length = audio_tox_state_ptr->tox_profile.size();

        audio_tox_state_ptr->tox =
		tox_new(&tox_opts,
			&err_new);
	ASSERT(audio_tox_state_ptr->tox != nullptr, P_ERR);
}

#pragma message("tox doesn't save public key bindings, should do that soon")

TV_SINK_MEDIUM_CLOSE(audio_tox){
        STD_STATE_GET_PTR(state_ptr,
                          tv_sink_audio_tox_state_t,
                          audio_tox_state_ptr);
        // save_data(audio_tox_state_ptr->tox,
        //        audio_tox_state_ptr->profile_file.c_str());
        tox_kill(audio_tox_state_ptr->tox);
        audio_tox_state_ptr->tox = nullptr;
        audio_tox_state_ptr = nullptr;
}

static void tv_sink_audio_tox_iterate(tv_sink_audio_tox_state_t *audio_tox_state_ptr){
        uint64_t time_milli_s = get_time_microseconds()/1000;
        if(time_milli_s - audio_tox_state_ptr->last_iter_time > tox_iteration_interval(audio_tox_state_ptr->tox)){
                tox_iterate(audio_tox_state_ptr->tox, nullptr);
                audio_tox_state_ptr->last_iter_time = time_milli_s;
        }
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
