#include "id_tier.h"
#include "id_tier_memory.h"

std::vector<std::pair<id_t_, mod_inc_t_> > id_tier::lookup::id_mod_inc::from_state(
	id_tier_state_t *tier_state_ptr){
	ASSERT(tier_state_ptr != nullptr, P_ERR);
	return tier_state_ptr->get_id_buffer();
}

std::vector<std::pair<id_t_, mod_inc_t_> > id_tier::lookup::id_mod_inc::from_state(
	id_t_ state_id){
	return from_state(
		PTR_DATA(state_id,
			 id_tier_state_t));
}

std::vector<std::pair<id_t_, mod_inc_t_> > id_tier::lookup::id_mod_inc::from_tier(
	uint8_t major, uint8_t minor){
	return from_state(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(
				 major, minor),
			 id_tier_state_t));
}

std::vector<id_t_> id_tier::lookup::ids::from_state(
	id_tier_state_t *tier_state_ptr){
	std::vector<std::pair<id_t_, mod_inc_t_> > id_mod_inc =
		id_tier::lookup::id_mod_inc::from_state(
			tier_state_ptr);
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < id_mod_inc.size();i++){
		retval.push_back(
			id_mod_inc[i].first);
	}
	return retval;
}

std::vector<id_t_> id_tier::lookup::ids::from_state(
	id_t_ state_id){
	return from_state(
		PTR_DATA(state_id,
			 id_tier_state_t));
}

std::vector<id_t_> id_tier::lookup::ids::from_tier(
	uint8_t major, uint8_t minor){
	return from_state(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(
				 major,
				 minor),
			 id_tier_state_t));
}

std::vector<id_t_> id_tier::lookup::type::from_state(
	id_tier_state_t* tier_state_ptr,
	uint8_t type){
	std::vector<id_t_> id_vector =
		id_tier::lookup::ids::from_state(
			tier_state_ptr);
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < id_vector.size();i++){
		if(get_id_type(id_vector[i]) == type){
			retval.push_back(
				id_vector[i]);
		}
	}
	return retval;
}

std::vector<id_t_> id_tier::lookup::type::from_state(
	id_t_ state_id,
	uint8_t type){
	return from_state(
		PTR_DATA(state_id,
			 id_tier_state_t),
		type);
}

std::vector<id_t_> id_tier::lookup::type::from_tier(
	uint8_t major,
	uint8_t minor,
	uint8_t type){
	return from_state(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(
				 major,
				 minor),
			 id_tier_state_t),
		type);
}
		
std::vector<id_t_> id_tier::lookup::type::from_tier(
	uint8_t major,
	uint8_t minor,
	std::string type){
	return from_state(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(
				 major,
				 minor),
			 id_tier_state_t),
		convert::type::to(type));
}
