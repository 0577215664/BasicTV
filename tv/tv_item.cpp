#include "tv_item.h"
#include "tv_channel.h"
#include "tv_window.h"
#include "tv_frame_audio.h"
#include "tv_frame_video.h"
#include "tv_frame_caption.h"

#include "../id/set/id_set.h"

#include "../escape.h"

std::vector<std::vector<id_t_> > tv_item_t::get_frame_id_vector(){
	std::pair<std::vector<std::vector<uint8_t> >, std::vector<uint8_t> > tmp =
		unescape_all_vectors(
			frame_sets, 0x80);
	std::vector<std::vector<id_t_> > id_vector;
	for(uint64_t i = 0;i < tmp.first.size();i++){
		id_vector.push_back(
			expand_id_set(
				tmp.first[i]));
	}
	return id_vector;
}

void tv_item_t::add_frame_id(std::vector<id_t_> stream_id_vector_){
	std::vector<uint8_t> chunk =
		escape_vector(
			compact_id_set(
				stream_id_vector_,
				true),
			0x80);
	frame_sets.insert(
		frame_sets.end(),
		chunk.begin(),
		chunk.end());
}

// it is pretty hard to keep track of what is what, espeically
// with variable lengths, so just clear everything and only re-add what
// we need (I'm pretty sure there isn't a current use for del_frame_id
// in tv_channel_t (the way it was when I wrote this), so I don't think
// it will be missed).

void tv_item_t::clear_frame_sets(){
	frame_sets.clear();
}

tv_item_t::tv_item_t() : id(this, TYPE_TV_ITEM_T){
	ADD_DATA(tv_channel_id, public_ruleset);
	id.add_data_one_byte_vector(&frame_sets, ~0, public_ruleset);
	id.add_data_one_byte_vector(&name, ~0, public_ruleset);
	id.add_data_one_byte_vector(&desc, ~0, public_ruleset);
	ADD_DATA(start_time_micro_s, public_ruleset);
	ADD_DATA(end_time_micro_s, public_ruleset);
}

tv_item_t::~tv_item_t(){}
