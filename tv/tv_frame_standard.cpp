#include "tv_frame_standard.h"

tv_frame_standard_t::tv_frame_standard_t(){
}

tv_frame_standard_t::~tv_frame_standard_t(){
}

void tv_frame_standard_t::list_virtual_data(data_id_t *id_){
	id = id_;
	id->add_data_raw(&start_time_micro_s, sizeof(start_time_micro_s), public_ruleset);
	id->add_data_raw(&ttl_micro_s, sizeof(ttl_micro_s), public_ruleset);
	id->add_data_raw(&frame_entry, sizeof(frame_entry), public_ruleset);
}

void tv_frame_standard_t::set_standard(uint64_t start_time_micro_s_,
				       uint32_t ttl_micro_s_,
				       uint64_t frame_entry_){
	set_start_time_micro_s(start_time_micro_s);
	set_ttl_micro_s(ttl_micro_s_);
	set_frame_entry(frame_entry_);
	// TODO:  check for invalid stuff
}

void tv_frame_standard_t::get_standard(uint64_t *start_time_micro_s_,
				       uint32_t *ttl_micro_s_,
				       uint64_t *frame_entry_){
	if(likely(start_time_micro_s_ != nullptr)){
		*start_time_micro_s_ = get_start_time_micro_s();
	}
	if(likely(ttl_micro_s_ != nullptr)){
		*ttl_micro_s_ = get_ttl_micro_s();
	}
	if(likely(frame_entry_ != nullptr)){
		*frame_entry_ = get_frame_entry();
	}
}

bool tv_frame_standard_t::valid(uint64_t timestamp_micro_s){
	const bool retval = BETWEEN(get_start_time_micro_s(),
				    timestamp_micro_s,
				    get_start_time_micro_s()+static_cast<uint64_t>(get_ttl_micro_s()));
	return retval;
}
