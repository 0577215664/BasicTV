#include "tv_frame_standard.h"
#include "tv_frame_audio.h"

tv_frame_audio_t::tv_frame_audio_t() : id(this, TYPE_TV_FRAME_AUDIO_T){
	list_virtual_data(&id);
	audio_prop.list_virtual_data(&id);
	id.add_data_one_byte_vector_vector(&packet_set, ~((uint32_t)0), ~((uint32_t)0), public_ruleset);
}

tv_frame_audio_t::~tv_frame_audio_t(){
}
