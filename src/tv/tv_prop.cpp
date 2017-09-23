#include "tv_prop.h"
#include "tv.h"

tv_prop_t::tv_prop_t(){
}

tv_prop_t::~tv_prop_t(){
}

void tv_prop_t::list_prop_data(data_id_t *id_){
	id = id_;
	id->add_data_raw(&bit_rate, sizeof(bit_rate), public_ruleset);
	id->add_data_raw(&format, sizeof(format), public_ruleset);
	id->add_data_raw(&flags, sizeof(flags), public_ruleset);
	id->add_data_raw(&snippet_duration_micro_s, sizeof(snippet_duration_micro_s), public_ruleset);
	id->add_data_eight_byte_vector(&encoder_flags, sizeof(encoder_flags), public_ruleset);
}
