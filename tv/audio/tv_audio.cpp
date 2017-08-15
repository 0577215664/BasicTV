#include "tv_audio.h"

tv_audio_prop_t::tv_audio_prop_t(){
}

tv_audio_prop_t::~tv_audio_prop_t(){
}

void tv_audio_prop_t::list_virtual_data(data_id_t *id){
	id->add_data_raw(&flags, sizeof(flags), public_ruleset);
	id->add_data_raw(&format, sizeof(format), public_ruleset);
	id->add_data_raw(&bit_depth, sizeof(bit_depth), public_ruleset);
	id->add_data_raw(&sampling_freq, sizeof(sampling_freq), public_ruleset);
	id->add_data_raw(&channel_count, sizeof(channel_count), public_ruleset);

	id->add_data_raw(&snippet_duration_micro_s, sizeof(snippet_duration_micro_s), public_ruleset);
	id->add_data_eight_byte_vector(&encoder_flags, ~0, public_ruleset);
}

