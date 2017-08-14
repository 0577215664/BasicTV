#include "../id.h"
#include "id_set.h"
#include "id_set_copy.h"
#include "id_set_uuid_list.h"

/*
  TODO: allow for individual ID lookups without expanding

  TODO: actually use NBO for this
 */

const std::vector<uint8_t> seperator = {0, 0, 0, 0, 0, 0, 0, 1};

std::vector<id_t_> expand_id_set(
	std::vector<uint8_t> id_set,
	bool *order){
	if(id_set.size() <= 2){
		return std::vector<id_t_>({});
	}
	std::vector<id_t_> retval;
	switch(id_set[0]){
	case ID_SET_SCHEME_UUID_LIST:
		retval =
			expand_id_set_uuid_list(
				std::vector<uint8_t>(
					id_set.begin()+1,
					id_set.end()));
		break;
	case ID_SET_SCHEME_COPY:
		retval = 
			expand_id_set_copy(
				std::vector<uint8_t>(
					id_set.begin()+1,
					id_set.end()));
		break;
	default:
		P_V(id_set[0], P_WARN);
		print("invalid scheme for ID set", P_ERR);
	}
	for(uint64_t i = 0;i < retval.size();i++){
		id_api::assert_valid_id(
			retval[i]);
	}
	if(order != nullptr){
		// playing it safe ATM
		*order = true;
	}
	return retval;
}

std::vector<uint8_t> compact_id_set(
	std::vector<id_t_> id_set,
	bool order){
	if(id_set.size() == 0){
		return std::vector<uint8_t>({});
	}
	for(uint64_t i = 0;i < id_set.size();i++){
		id_api::assert_valid_id(
			id_set[i]);
	}
	uint8_t scheme =
		ID_SET_SCHEME_COPY;
	if(order){
		const hash_t_ hash =
			get_id_hash(id_set[0]);
		bool single_hash = true;
		for(uint64_t i = 1;i < id_set.size();i++){
			if(hash != get_id_hash(id_set[i])){
				single_hash = false;
				break;
			}
		}
		if(single_hash){
			scheme =
				ID_SET_SCHEME_UUID_LIST;
		}else{
			scheme =
				ID_SET_SCHEME_COPY;
		}
	}else{
		scheme =
			ID_SET_SCHEME_UUID_LIST;
	}
	std::vector<uint8_t> retval;
	switch(scheme){
	case ID_SET_SCHEME_UUID_LIST:
		retval =
			compact_id_set_uuid_list(
				id_set);
		break;
	case ID_SET_SCHEME_COPY:
		retval =
			compact_id_set_copy(
				id_set);
		break;
	default:
		P_V(scheme, P_WARN);
		print("invalid scheme for ID set", P_ERR);
	}
	retval.insert(
		retval.begin(),
		scheme);
	return retval;
}

/*
  Again super slow, but super simple

  Murphy's Law
 */

std::vector<uint8_t> add_id_to_set(std::vector<uint8_t> id_set, id_t_ id){
	bool order = true;
	std::vector<id_t_> out =
		expand_id_set(
			id_set,
			&order);
	out.push_back(
		id);
	return compact_id_set(
		out, order);
}

std::vector<uint8_t> del_id_from_set(std::vector<uint8_t> id_set, id_t_ id){	
	bool order = true;
	std::vector<id_t_> out =
		expand_id_set(
			id_set,
			&order);
	out.push_back(
		id);
	for(uint64_t i = 0;i < out.size();i++){
		if(unlikely(out[i] == id)){
			out.erase(
				out.begin()+i);
			i--;
		}
	}
	return compact_id_set(
		out,
		order);
}

uint64_t size_of_id_set(std::vector<uint8_t> id_set){
	return expand_id_set(id_set).size();
}

#undef ASSERT_LENGTH
