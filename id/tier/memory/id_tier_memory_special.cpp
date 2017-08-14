#include "id_tier_memory_special.h"
#include "id_tier_memory_helper.h"

extern std::array<std::vector<data_id_t*>, TYPE_COUNT+1> id_vector;
extern std::vector<std::pair<id_t_, mod_inc_t_> > id_buffer;


void id_tier_mem_regen_state_cache(){
	id_buffer.clear();
	for(uint64_t c = 0;c < TYPE_COUNT+1;c++){
		for(uint64_t i = 0;i < id_vector[c].size();i++){
			id_buffer.push_back(
				std::make_pair(id_vector[c][i]->get_id(),
					       id_vector[c][i]->get_mod_inc()));
		}
	}
}

void id_tier_mem_update_state_cache(
	id_tier_state_t *tier_state_ptr){
	// Probably could use some pointer magic
	ASSERT(tier_state_ptr != nullptr, P_ERR);
	tier_state_ptr->set_id_buffer(
		id_buffer);
}

/*
  TODO: don't regenerate the entire buffer
 */

void mem_add_id(data_id_t *ptr){
	ASSERT(get_id_type(ptr->get_id()) <= TYPE_COUNT, P_ERR);
	const type_t_ id_type =
		get_id_type(ptr->get_id());
	id_vector[id_type].push_back(ptr);
	id_buffer.push_back(
		std::make_pair(
			ptr->get_id(),
			ptr->get_mod_inc()));
	id_tier_mem_regen_state_cache();
	id_tier_mem_update_state_cache(
		mem_helper::lookup::tier_state(
			std::vector<std::pair<uint8_t, uint8_t> >({
					std::make_pair(0, 0)})).at(0));
}

void mem_del_id(data_id_t *ptr){
	ASSERT(get_id_type(ptr->get_id()) <= TYPE_COUNT, P_ERR);
	const type_t_ id_type =
		get_id_type(ptr->get_id());
	auto id_pos =
		std::find(
			id_vector[id_type].begin(),
			id_vector[id_type].end(),
			ptr);
	if(id_pos != id_vector[id_type].end()){
		id_vector[id_type].erase(
			id_pos);
	}else{
		print("couldn't find ptr in id_vector", P_WARN);
	}
	id_tier_mem_regen_state_cache();
	id_tier_mem_update_state_cache(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(0, 0),
			 id_tier_state_t));
}
