#include "id_tier.h"
#include "id_tier_memory.h"

std::vector<id_tier_medium_t> id_tier_medium = {
	id_tier_medium_t(
		id_tier_mem_init_state,
		id_tier_mem_del_state,
		id_tier_mem_add_data,
		id_tier_mem_del_id,
		id_tier_mem_get_id,
		id_tier_mem_get_id_mod_inc,
		id_tier_mem_get_id_buffer,
		id_tier_mem_update_id_buffer)
};

id_tier_medium_t id_tier::get_medium(uint8_t medium_type){
	return id_tier_medium.at(medium_type-1); // 0 is undefined
}

id_t_ id_tier::state_tier::only_state_of_tier(
	uint8_t tier_major,
	uint8_t tier_minor){
	id_t_ id = ID_BLANK_ID;
	/*
	  TODO: To stop a deadlock, we need to ensure that all id_tier_state_t stay
	  in memory. Enforce that
	 */
	id_tier_state_t *tier_state_ptr =
		mem_tier_state_lookup(
			tier_major,
			tier_minor);
	if(tier_state_ptr != nullptr){
		return tier_state_ptr->id.get_id();
	}
	print("state isn't in memory at the moment, this needs to be ensured", P_CRIT);
	std::vector<id_t_> id_state_list =
		ID_TIER_CACHE_GET(
			TYPE_ID_TIER_STATE_T);
	std::vector<id_t_> match_vector;
	for(uint64_t i = 0;i < id_state_list.size();i++){
		id_tier_state_t *id_tier_state_ptr =
			PTR_DATA(id_state_list[i],
				 id_tier_state_t);
		CONTINUE_IF_NULL(id_tier_state_ptr, P_ERR);
		if(id_tier_state_ptr->get_tier_major() == tier_major &&
		   id_tier_state_ptr->get_tier_minor() == tier_minor){
			return id_state_list[i];
		}
	}
	return id;
}

std::vector<id_t_> id_tier::state_tier::optimal_state_vector_of_tier(
	uint8_t tier_major,
	uint8_t tier_minor){
	std::vector<id_t_> retval;
	std::vector<id_t_> id_state_list =
		ID_TIER_CACHE_GET(
			TYPE_ID_TIER_STATE_T);
	for(uint64_t i = 0;i < id_state_list.size();i++){
		id_tier_state_t *id_tier_state_ptr =
			PTR_DATA(id_state_list[i],
				 id_tier_state_t);
		CONTINUE_IF_NULL(id_tier_state_ptr, P_ERR);
		if(id_tier_state_ptr->get_tier_major() == tier_major &&
		   id_tier_state_ptr->get_tier_minor() == tier_minor){
			retval.push_back(
				id_state_list[i]);
		}
	}
	return retval;
}

std::vector<std::tuple<id_t_, uint8_t, uint8_t> > id_tier::operation::valid_state_with_id(
	id_t_ id){
	std::vector<std::tuple<id_t_, uint8_t, uint8_t> > retval;
	std::vector<id_t_> id_tier_states =
		ID_TIER_CACHE_GET(
			TYPE_ID_TIER_STATE_T);
	for(uint64_t i = 0;i < id_tier_states.size();i++){
		id_tier_state_t *id_tier_state_ptr =
			PTR_DATA(id_tier_states[i],
				 id_tier_state_t);
		CONTINUE_IF_NULL(id_tier_state_ptr, P_WARN);
		std::vector<std::pair<id_t_, mod_inc_t_> > state_cache =
			id_tier_state_ptr->get_id_buffer();
		print("don't bother right now", P_CRIT);
		std::vector<id_t_> all_ids =
			id_tier::lookup::ids::from_state(
				id_tier_state_ptr);
		if(std::find(
			   all_ids.begin(),
			   all_ids.end(),
			   id) != all_ids.end()){
			retval.push_back(
				std::make_tuple(
					id_tier_states[i],
					id_tier_state_ptr->get_tier_major(),
					id_tier_state_ptr->get_tier_minor()));
		}
	}
	return retval;
}

void id_tier::operation::shift_data_to_state(
	id_t_ start_state_id,
	id_t_ end_state_id,
	std::vector<id_t_> id_vector){
	id_tier_state_t *state[2] = {
		PTR_DATA(start_state_id,
			 id_tier_state_t),
		PTR_DATA(end_state_id,
			 id_tier_state_t)
	};
	ASSERT(state[0] != nullptr, P_ERR);
	ASSERT(state[1] != nullptr, P_ERR);

	std::vector<std::pair<id_t_, mod_inc_t_> > first_buffer =
		state[0]->get_id_buffer();
	std::vector<std::pair<id_t_, mod_inc_t_> > second_buffer =
		state[1]->get_id_buffer();
	
	for(uint64_t i = 0;i < id_vector.size();i++){
		id_tier_medium_t first_medium =
			id_tier::get_medium(
				state[0]->get_medium());
		id_tier_medium_t second_medium =
			id_tier::get_medium(
				state[1]->get_medium());
		if(std::find_if(
			   first_buffer.begin(),
			   first_buffer.end(),
			   [&id_vector, &i](std::pair<id_t_, mod_inc_t_> const& elem){
				   return id_vector[i] == elem.first;
			   }) != first_buffer.end()){
			try{
				second_medium.add_data(
					end_state_id,
					first_medium.get_id(
						start_state_id,
						id_vector[i]));
						
			}catch(...){
				print("couldn't shift id " + id_breakdown(id_vector[i]) + " over to new device", P_ERR);
			}
		}
	}
}

void id_tier::operation::del_id_from_state(
	std::vector<id_t_> state_id,
	std::vector<id_t_> id){
	for(uint64_t i = 0;i < state_id.size();i++){
		try{
			id_tier_state_t *tier_state_ptr =
				PTR_DATA(state_id[i],
					 id_tier_state_t);
			PRINT_IF_NULL(tier_state_ptr, P_ERR);
			id_tier_medium_t medium =
				id_tier::get_medium(
					tier_state_ptr->get_medium());
			for(uint64_t c = 0;c < id.size();i++){
				try{
					medium.del_id(
						tier_state_ptr->id.get_id(),
						id[c]);
				}catch(...){}
			}
		}catch(...){}
	}
}

std::vector<std::vector<uint8_t> > id_tier::operation::get_data_from_state(
	std::vector<id_t_> state_id,
	std::vector<id_t_> id_vector){
	std::vector<std::vector<uint8_t> > retval;
	for(uint64_t i = 0;i < state_id.size();i++){
		try{
			id_tier_state_t *tier_state_ptr =
				PTR_DATA(state_id[i],
					 id_tier_state_t);
			CONTINUE_IF_NULL(tier_state_ptr, P_WARN);
			std::vector<id_t_> state_cache =
				id_tier::lookup::ids::from_state(
					tier_state_ptr);
			for(uint64_t c = 0;c < id_vector.size();c++){
				try{
					if(std::find(
						   state_cache.begin(),
						   state_cache.end(),
						   id_vector[c]) != state_cache.end()){
						id_tier_medium_t medium =
							id_tier::get_medium(
								tier_state_ptr->get_medium());
						retval.push_back(
							medium.get_id(
								tier_state_ptr->id.get_id(),
								id_vector[c]));
					}
				}catch(...){}
			}
		}catch(...){}
	}
	return retval;
}

void id_tier::operation::add_data_to_state(
	std::vector<id_t_> state_id,
	std::vector<std::vector<uint8_t> > data_vector){
	
	for(uint64_t i = 0;i < state_id.size();i++){
		try{
			id_tier_state_t *tier_state_ptr =
				PTR_DATA(state_id[i],
					 id_tier_state_t);
			CONTINUE_IF_NULL(tier_state_ptr, P_WARN);
			for(uint64_t c = 0;c < data_vector.size();c++){
				try{
					id_tier_medium_t medium =
						id_tier::get_medium(
							tier_state_ptr->get_medium());
					medium.add_data(
						tier_state_ptr->id.get_id(),
						data_vector[c]);
				}catch(...){}
			}
		}catch(...){}
	}
}

id_tier_state_t::id_tier_state_t() : id(this, TYPE_ID_TIER_STATE_T){
}

id_tier_state_t::~id_tier_state_t(){
}


void id_tier_init(){
}

void id_tier_loop(){
}

void id_tier_close(){
}
