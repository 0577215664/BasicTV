#include "id_request_stats.h"
#include "../id_request.h"

static math_stat_pval_t id_request_stats_linked_list_logic(
	id_tier_state_t *tier_state_ptr,
	id_t_ target_id){
	math_stat_pval_t retval = 0;
	return retval;
}

math_stat_pval_t id_request_stats_gen_pval(
	id_t_ desired_id,
	id_t_ tier_id){
	math_stat_pval_t retval = 0;
	id_tier_state_t *tier_state_ptr =
		PTR_DATA(tier_id,
			 id_tier_state_t);
	ASSERT(tier_state_ptr != nullptr,  P_ERR);

	/*
	 */
	return retval;
}
