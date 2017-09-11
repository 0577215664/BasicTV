#include "id_request_stat.h"

ID_REQUEST_FORMAT_INIT(stat){
	STD_STATE_INIT(id_request_tier_entry_t,
		       tier_entry_state_ptr,
		       id_request_tier_entry_stat_t,
		       stat_ptr);
	return tier_entry_state_ptr;
}

ID_REQUEST_FORMAT_CLOSE(stat){
	STD_STATE_CLOSE(tier_entry_state_ptr,
			id_request_tier_entry_stat_t);
}

ID_REQUEST_FORMAT_REFRESH(stat){
	const uint64_t cur_time_micro_s =
		get_time_microseconds();
	STD_STATE_GET_PTR(tier_entry_state_ptr,
			  id_request_tier_entry_stat_t,
			  stat_ptr);
	if(cur_time_micro_s-stat_ptr->last_update_micro_s > stat_ptr->update_sleep_micro_s){
		stat_ptr->last_update_micro_s = cur_time_micro_s;
	}
}
