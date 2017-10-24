#include "id_tier_move.h"
#include "id_tier_move_network.h"
#include "../id_tier.h"

#pragma message("seperate tier_move_logic into standard and network shifting rules")

static std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > tier_standard_move_logic(
	id_tier_state_t *first_state_ptr,
	id_tier_state_t *second_state_ptr){
	std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > retval;

	const std::vector<std::pair<id_t_, mod_inc_t_> > first_id_buffer =
		id_tier::lookup::id_mod_inc::from_state(
			first_state_ptr);
	const std::vector<std::pair<id_t_, mod_inc_t_> > second_id_buffer =
		id_tier::lookup::id_mod_inc::from_state(
			second_state_ptr);

	for(uint64_t a = 0;a < first_id_buffer.size();a++){
		bool found = false;
		const id_t_ a_id = std::get<0>(first_id_buffer[a]);
		if(get_id_type(a_id) == TYPE_ID_TIER_STATE_T){
			continue;
		}
		if(std::find(
			   mem_only_types.begin(),
			   mem_only_types.end(),
			   get_id_type(a_id)) != mem_only_types.end()){
			// we assume its in memory currently
			continue;
		}
		for(uint64_t b = 0;b < second_id_buffer.size();b++){
			if(std::find(
				   mem_only_types.begin(),
				   mem_only_types.end(),
				   get_id_type(std::get<0>(second_id_buffer[b]))) != mem_only_types.end()){
				// we assume its in memory currently	
				continue;
			}
			
			if(std::get<0>(second_id_buffer[b]) == a_id){
				const mod_inc_t_ a_mod_inc =
					std::get<1>(first_id_buffer[a]);
				const mod_inc_t_ b_mod_inc =
					std::get<1>(second_id_buffer[b]);
				found = true;
				if(a_mod_inc > b_mod_inc){
					retval.push_back(
						std::make_tuple(
							a_id,
							first_state_ptr->id.get_id(),
							second_state_ptr->id.get_id(),
							COPY_UP));
				}else if(a_mod_inc < b_mod_inc){
					retval.push_back(
						std::make_tuple(
							a_id,
							first_state_ptr->id.get_id(),
							second_state_ptr->id.get_id(),
							COPY_DOWN));
				}
				break;
			}
		}
		if(found == false){ // push it down if it isn't already there
			retval.push_back(
				std::make_tuple(
					a_id,
					first_state_ptr->id.get_id(),
					second_state_ptr->id.get_id(),
					COPY_UP));
		}
	}
	return retval;
}

std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > tier_move_logic(
	id_t_ first_id,
	id_t_ second_id){
	id_tier_state_t *first_state_ptr =
		PTR_DATA(first_id,
			 id_tier_state_t);
	ASSERT(first_state_ptr != nullptr, P_ERR);
	id_tier_state_t *second_state_ptr =
		PTR_DATA(second_id,
			 id_tier_state_t);
	ASSERT(second_state_ptr != nullptr, P_ERR);

	// We don't bother with cache right now, since that's more hinting
	// and pre-loading (which isn't an optimization until MT gets going)
	if(first_state_ptr->get_tier_major() == second_state_ptr->get_tier_major()){
		return {};
	}

	if(first_state_ptr->get_tier_major() > second_state_ptr->get_tier_major()){
		std::swap(first_state_ptr, second_state_ptr);
		std::swap(first_id, second_id);
	}
	std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > retval;
	if(second_state_ptr->get_tier_major() == ID_TIER_MAJOR_NETWORK){
		retval =
			tier_network_move_push_logic(
				first_state_ptr,
				second_state_ptr);
	}else{
		retval =
			tier_standard_move_logic(
				first_state_ptr,
				second_state_ptr);
	}
	if(retval.size() != 0){
		print("computed " + std::to_string(retval.size()) + " shift operations", P_NOTE);
		if(retval.size() == 3){
			std::raise(SIGINT);
		}
	}
	return retval;
}
