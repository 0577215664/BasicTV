#include "id_tier.h"
#include "id_tier_memory.h"

std::vector<id_tier_medium_t> id_tier_medium = {
	id_tier_medium_t(
		id_tier_mem_add_data,
		id_tier_mem_del_id,
		id_tier_mem_get_id,
		id_tier_mem_get_id_mod_inc,
		id_tier_mem_get_id_buffer,
		id_tier_mem_update_id_buffer)
};

id_tier_medium_t id_tier::get_medium(uint8_t medium_type){
	return id_tier_medium.at(medium_type);
}

// id_t_ id_tier::only_state_of_tier(
// 	uint8_t tier_major,
// 	uint8_t tier_minor){
// 	id_t_ id;
// 	std::vector<id_t_> id_state_list =
// 		id_api::cache::get(
// 			TYPE_ID_TIER_STATE_T);
// 	std::vector<id_t_> match_vector;
// 	for(uint64_t i = 0;i < id_state_list.size();i++){
// 		id_tier_state_t *id_tier_state_ptr =
// 			PTR_DATA(id_state_list[i],
// 				 id_tier_state_t);
// 		CONTINUE_IF_NULL(id_tier_state_ptr, P_ERR);
// 		if(id_tier_state_ptr->get_tier
// 	}
// 	return id;
// }
