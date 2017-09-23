#include "tv_video.h"
#include "../tv_prop.h"

tv_video_prop_t::tv_video_prop_t(){
}

tv_video_prop_t::~tv_video_prop_t(){
}

void tv_video_prop_t::list_virtual_data(data_id_t *id_){
	id = id_;
	id->add_data_raw(&x_res, sizeof(x_res), public_ruleset);
	id->add_data_raw(&y_res, sizeof(y_res), public_ruleset);
	id->add_data_raw(&bit_depth, sizeof(bit_depth), public_ruleset);
}
