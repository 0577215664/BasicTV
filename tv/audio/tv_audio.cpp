#include "tv_audio.h"

tv_audio_prop_t::tv_audio_prop_t(){
}

tv_audio_prop_t::~tv_audio_prop_t(){
}

void tv_audio_prop_t::list_virtual_data(data_id_t *id_){
	id = id_;
	list_prop_data(id);
	id->add_data_raw(&bit_depth, sizeof(bit_depth), public_ruleset);
	id->add_data_raw(&sampling_freq, sizeof(sampling_freq), public_ruleset);
	id->add_data_raw(&channel_count, sizeof(channel_count), public_ruleset);
}

