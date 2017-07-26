#include "id_tier_memory_helper.h"
#include "id_tier_memory.h"

extern std::array<std::vector<data_id_t*>, TYPE_COUNT+1> id_vector;
extern std::vector<std::pair<id_t_, mod_inc_t_> > id_buffer;

data_id_t *mem_helper::lookup::id(id_t_ id_){
	if(id_ == ID_BLANK_ID){
		return nullptr;
	}
	const type_t_ id_type =
		get_id_type(id_);
	for(uint64_t i = 0;i < id_vector[id_type].size();i++){
		if(get_id_hash(id_vector[id_type][i]->get_id(true)) == blank_hash){
			P_V_S(id_breakdown(id_vector[id_type][i]->get_id(true)), P_NOTE);
			ASSERT(get_id_hash(id_vector[id_type][i]->get_id(true)) != blank_hash, P_WARN);
		}
		try{
			const id_t_ list_id =
				id_vector[id_type][i]->get_id(true);
			const bool matching_nonhash =
				get_id_uuid(list_id) == get_id_uuid(id_) &&
				get_id_type(list_id) == get_id_type(id_);
			if(unlikely(matching_nonhash)){
				const bool matching_hash =
					get_id_hash(list_id) == get_id_hash(id_);
				const bool list_hash_blank =
					get_id_hash(list_id) == blank_hash;
				const bool param_hash_blank =
					get_id_hash(id_) == blank_hash;
				if(matching_hash ||
				   (list_hash_blank || param_hash_blank)){
					// data_id_t *retval =
					// 	id_vector[i];
					// std::swap(
					// 	id_vector[0],
					// 	id_vector[i]);
					// std::swap(
					// 	id_buffer[0],
					// 	id_buffer[i]);
					return id_vector[id_type][i];
				}
			}
		}catch(...){}
	}
	return nullptr;
}

void *mem_helper::lookup::ptr(id_t_ id_){
	data_id_t *id_ptr =
		id(id_);
	if(id_ptr != nullptr){
		return id_ptr->get_ptr();
	}
	return nullptr;
}

std::vector<id_tier_state_t*> mem_helper::lookup::tier_state(
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector){
	std::vector<id_tier_state_t*> retval;
	for(uint64_t i = 0;i < tier_vector.size();i++){
		for(uint64_t c = 0;c < id_vector[TYPE_ID_TIER_STATE_T].size();c++){
			if(likely(id_vector[TYPE_ID_TIER_STATE_T][c]->get_type_byte() == TYPE_ID_TIER_STATE_T)){
				id_tier_state_t *tier_state_ptr =
					reinterpret_cast<id_tier_state_t*>(
						id_vector[TYPE_ID_TIER_STATE_T][c]->get_ptr());
				CONTINUE_IF_NULL(tier_state_ptr, P_WARN);
				if(std::find(
					   tier_vector.begin(),
					   tier_vector.end(),
					   std::make_pair(
						   tier_state_ptr->get_tier_major(),
						   tier_state_ptr->get_tier_minor())) !=
				   tier_vector.end()){
					retval.push_back(
						tier_state_ptr);
				}
			}
		}
	}
	return retval;
}

std::vector<data_id_t*> mem_helper::get_data_id_vector(){
	std::vector<data_id_t*> retval;
	for(uint64_t i = 0;i < TYPE_COUNT+1;i++){
		retval.insert(
			retval.end(),
			id_vector[i].begin(),
			id_vector[i].end());
	}
	return retval;
}
