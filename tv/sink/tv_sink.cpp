#include "../tv.h"
#include "tv_sink.h"

#include "audio/tv_sink_audio_hardware.h"
#include "numerical/tcp/tv_sink_numerical_tcp_accept.h"

std::vector<tv_sink_medium_t> sink_medium_vector =
{
	tv_sink_medium_t(TV_SINK_MEDIUM_FLOW_DIRECTION_OUT,
			 TV_SINK_MEDIUM_AUDIO_HARDWARE,
			 tv_sink_audio_hardware_init,
			 tv_sink_audio_hardware_close,
			 tv_sink_audio_hardware_pull,
			 tv_sink_audio_hardware_push),
	tv_sink_medium_t(TV_SINK_MEDIUM_FLOW_DIRECTION_BOTH,
			 TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT,
			 tv_sink_tcp_accept_init,
			 tv_sink_tcp_accept_close,
			 tv_sink_tcp_accept_pull,
			 tv_sink_tcp_accept_push)
};

tv_sink_medium_t tv_sink_get_medium(uint8_t medium){
	for(uint64_t i = 0;i < sink_medium_vector.size();i++){
		if(sink_medium_vector[i].medium == medium){
			return sink_medium_vector[i];
		}
	}
	print("invalid medium", P_ERR);
}

tv_sink_state_t::tv_sink_state_t() : id(this, TYPE_TV_SINK_STATE_T){
}

tv_sink_state_t::~tv_sink_state_t(){
}

tv_sink_state_t *tv::sink::state::init(
	uint8_t medium,
	uint8_t flow_direction){
	tv_sink_medium_t sink_medium =
		tv_sink_get_medium(medium);
	return sink_medium.init(
		flow_direction);
}

void tv::sink::state::close(tv_sink_state_t *state_ptr){
	ASSERT(state_ptr != nullptr, P_UNABLE);
	tv_sink_medium_t sink_medium =
		tv_sink_get_medium(state_ptr->get_medium());
	sink_medium.close(state_ptr);
}

void tv::sink::state::close(id_t_ sink_state_id){
	tv_sink_state_t *sink_state_ptr =
		PTR_DATA(sink_state_id,
			 tv_sink_state_t);
	close(sink_state_ptr);
}

// pull and push here ensure there aren't any duplicates, but they don't
// ensure the data comes/leaves in order

std::vector<id_t_> tv::sink::state::pull(
	tv_sink_state_t *sink_state_ptr,
	uint8_t mapping){
	PRINT_IF_NULL(sink_state_ptr, P_ERR);
	tv_sink_medium_t sink_medium =
		tv_sink_get_medium(
			sink_state_ptr->get_medium());
	std::vector<id_t_> retval =
		sink_medium.pull(
			sink_state_ptr,
			mapping);
	return retval;
}

std::vector<id_t_> tv::sink::state::pull(
	id_t_ sink_state_id,
	uint8_t mapping){
	return pull(
		PTR_DATA(sink_state_id,
			 tv_sink_state_t),
		mapping);
}

void tv::sink::state::push(
	tv_sink_state_t *sink_state_ptr,
	int64_t window_offset_micro_s,
	std::vector<id_t_> data){
	PRINT_IF_NULL(sink_state_ptr, P_ERR);
	tv_sink_medium_t sink_medium =
		tv_sink_get_medium(
			sink_state_ptr->get_medium());
	sink_medium.push(
		sink_state_ptr,
		window_offset_micro_s,
		data);
}

void tv::sink::state::push(
	id_t_ sink_state_id,
	int64_t window_offset_micro_s,
	std::vector<id_t_> data){
	push(PTR_DATA(sink_state_id,
		      tv_sink_state_t),
	     window_offset_micro_s,
	     data);
}

