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

}
