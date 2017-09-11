#include "tv_frame_standard.h"
#include "tv_frame_video.h"

tv_frame_video_t::tv_frame_video_t() : id(this, TYPE_TV_FRAME_VIDEO_T){
	id.add_data_one_byte_vector_vector(&packet_set, ~0, ~0, public_ruleset);
	video_prop.list_prop_data(&id);
	list_virtual_data(&id);
}

tv_frame_video_t::~tv_frame_video_t(){
}
