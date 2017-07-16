#include "id_tier_memory_special.h"
#include "id_tier_memory_helper.h"

extern std::vector<data_id_t*> id_vector;
extern std::vector<std::pair<id_t_, mod_inc_t_> > id_buffer;


void id_tier_mem_regen_state_cache(){
	id_buffer.clear();
	for(uint64_t i = 0;i < id_vector.size();i++){
		id_buffer.push_back(
			std::make_pair(id_vector[i]->get_id(),
				       id_vector[i]->get_mod_inc()));
	}
}

void id_tier_mem_update_state_cache(
	id_tier_state_t *tier_state_ptr){
	// Probably could use some pointer magic
	ASSERT(tier_state_ptr != nullptr, P_ERR);
	tier_state_ptr->set_id_buffer(
		id_buffer);
}

void mem_add_id(data_id_t *ptr){
	id_vector.push_back(ptr);
	id_buffer.push_back(
		std::make_pair(
			ptr->get_id(),
			ptr->get_mod_inc()));
	id_tier_mem_update_state_cache(
		mem_helper::lookup::tier_state(
			std::vector<std::pair<uint8_t, uint8_t> >({
					std::make_pair(0, 0)})).at(0));
}

void mem_del_id(data_id_t *ptr){
	auto id_pos =
		std::find(
			id_vector.begin(),
			id_vector.end(),
			ptr);
	if(id_pos != id_vector.end()){
		id_vector.erase(
			id_pos);
	}
	for(uint64_t i = 0;i < id_buffer.size();i++){
		if(id_buffer[i].first == ptr->get_id()){
			id_buffer.erase(
				id_buffer.begin()+i);
			break;
		}
	}
	id_tier_mem_update_state_cache(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(0, 0),
			 id_tier_state_t));
}
