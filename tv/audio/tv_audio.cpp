#include "tv_audio.h"

tv_audio_prop_t::tv_audio_prop_t(){
}

tv_audio_prop_t::~tv_audio_prop_t(){
}

void tv_audio_prop_t::list_virtual_data(data_id_t *id){
	id->add_data_raw(&flags, sizeof(flags));
	id->add_data_raw(&format, sizeof(format));
	id->add_data_raw(&bit_depth, sizeof(bit_depth));
	id->add_data_raw(&sampling_freq, sizeof(sampling_freq));
	id->add_data_raw(&channel_count, sizeof(channel_count));

	id->add_data_raw(&snippet_duration_micro_s, sizeof(snippet_duration_micro_s));
	id->add_data_eight_byte_vector(&encoder_flags, ~0);
}

