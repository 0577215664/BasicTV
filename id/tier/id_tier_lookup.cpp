#include "id_tier.h"
#include "memory/id_tier_memory.h"
#include "id_tier_cache.h"

std::vector<std::pair<id_t_, mod_inc_t_> > id_tier::lookup::id_mod_inc::from_state(
	id_tier_state_t *tier_state_ptr){
	ASSERT(tier_state_ptr != nullptr, P_ERR);
	const std::vector<id_t_> id_list =
		tier_state_ptr->storage.get_ids();
	const std::vector<mod_inc_t_> mod_inc_list =
		tier_state_ptr->storage.get_mod_incs();
	ASSERT(id_list.size() == mod_inc_list.size(), P_ERR);
	std::vector<std::pair<id_t_, mod_inc_t_> > retval;
	retval.reserve(id_list.size());
	for(uint64_t i = 0;i < id_list.size();i++){
		retval.push_back(
			std::make_pair(
				id_list[i],
				mod_inc_list[i]));
	}
	return retval;
}

std::vector<std::pair<id_t_, mod_inc_t_> > id_tier::lookup::id_mod_inc::from_state(
	std::vector<id_t_> state_vector){
	std::vector<std::pair<id_t_, mod_inc_t_> > retval;
	for(uint64_t i = 0;i < state_vector.size();i++){
		try{
			std::vector<std::pair<id_t_, mod_inc_t_> > tmp =
				from_state(
					PTR_DATA(state_vector[i],
						 id_tier_state_t));
			ASSERT(tmp.size() > 0, P_UNABLE);
			retval.insert(
				retval.end(),
				tmp.begin(),
				tmp.end());
		}catch(...){}
	}
	return retval;
}

std::vector<std::pair<id_t_, mod_inc_t_> > id_tier::lookup::id_mod_inc::from_tier(
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector){
	std::vector<std::pair<id_t_, mod_inc_t_> > retval;
	for(uint64_t i = 0;i < tier_vector.size();i++){
		try{
			std::vector<std::pair<id_t_, mod_inc_t_> > tmp =
				from_state(
					PTR_DATA(id_tier::state_tier::only_state_of_tier(
							 tier_vector[i].first,
							 tier_vector[i].second),
						 id_tier_state_t));
			ASSERT(tmp.size() > 0, P_UNABLE);
			retval.insert(
				retval.end(),
				tmp.begin(),
				tmp.end());
		}catch(...){}
	}
	return retval;
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
	std::vector<id_t_> state_vector){
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < state_vector.size();i++){
		try{
			std::vector<id_t_> tmp =
				from_state(
					PTR_DATA(state_vector[i],
						 id_tier_state_t));
			ASSERT(tmp.size() > 0, P_UNABLE);
			retval.insert(
				retval.end(),
				tmp.begin(),
				tmp.end());
		}catch(...){}
	}
	return retval;
}

std::vector<id_t_> id_tier::lookup::ids::from_tier(
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector){
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < tier_vector.size();i++){
		try{
			std::vector<id_t_> tmp =
				from_state(
					PTR_DATA(id_tier::state_tier::only_state_of_tier(
							 tier_vector[i].first,
							 tier_vector[i].second),
						 id_tier_state_t));
			ASSERT(tmp.size() > 0, P_UNABLE);
			retval.insert(
				retval.end(),
				tmp.begin(),
				tmp.end());
		}catch(...){}
	}
	return retval;
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
	std::vector<id_t_> state_vector,
	uint8_t type){
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < state_vector.size();i++){
		try{
			std::vector<id_t_> tmp =
				from_state(
					PTR_DATA(state_vector[i],
						 id_tier_state_t),
					type);
			ASSERT(tmp.size() > 0, P_UNABLE);
			retval.insert(
				retval.end(),
				tmp.begin(),
				tmp.end());
		}catch(...){}
	}
	std::sort(retval.begin(), retval.end());
	auto last = std::unique(retval.begin(), retval.end());
	retval.erase(
		last, retval.end());
	return retval;
}

std::vector<id_t_> id_tier::lookup::type::from_tier(
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector,
	uint8_t type){
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < tier_vector.size();i++){
		try{
			std::vector<id_t_> tmp =
				from_state(
					PTR_DATA(id_tier::state_tier::only_state_of_tier(
							 tier_vector[i].first,
							 tier_vector[i].second),
						 id_tier_state_t),
					type);
			retval.insert(
				retval.end(),
				tmp.begin(),
				tmp.end());
		}catch(...){}
	}
	std::sort(retval.begin(), retval.end());
	auto last = std::unique(retval.begin(), retval.end());
	retval.erase(
		last, retval.end());
	return retval;
}
		
std::vector<id_t_> id_tier::lookup::type::from_tier(
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector,
	std::string type){
	return from_tier(
		tier_vector,
		convert::type::to(type));
}
