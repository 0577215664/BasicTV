#include "../main.h"
#include "../util.h"
#include "tv.h"
#include "tv_frame_standard.h"
#include "tv_frame_video.h"
#include "tv_frame_audio.h"
#include "tv_window.h"
#include "tv_channel.h"
#include "tv_item.h"

tv_window_t::tv_window_t() : id(this, TYPE_TV_WINDOW_T){
	id.add_data_id(&item_id, 1);
	ADD_DATA(timestamp_offset_micro_s);
	id.set_lowest_global_flag_level(
		ID_DATA_NETWORK_RULE_NEVER,
		ID_DATA_EXPORT_RULE_NEVER,
		ID_DATA_RULE_UNDEF);
}

tv_window_t::~tv_window_t(){
}

std::vector<std::tuple<id_t_,
		       id_t_,
		       std::vector<uint8_t> > > tv_window_t::get_active_streams(){
	return active_streams;
}

void tv_window_t::set_active_streams(
	std::vector<std::tuple<id_t_,
	id_t_,
	std::vector<uint8_t> > > active_streams_){
	active_streams = active_streams_;
}

