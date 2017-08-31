#include "id_request.h"
#include "stat/id_request_stat.h"

const std::vector<id_request_tier_entry_medium_t> id_request_tier_mediums = {
	id_request_tier_entry_medium_t(
		ID_REQUEST_FORMAT_STAT,
		id_request_stat_init,
		id_request_stat_close,
		id_request_stat_refresh)
};

void id_request_init(){
}

void id_request_loop(){	
}

void id_request_close(){
}
