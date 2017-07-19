#include "../id.h"
#include "id_set.h"
#include "id_set_copy.h"
#include "id_set_uuid_list.h"

/*
  TODO: allow for individual ID lookups without expanding

  TODO: actually use NBO for this
 */

const std::vector<uint8_t> seperator = {0, 0, 0, 0, 0, 0, 0, 0};

std::vector<id_t_> expand_id_set(std::vector<uint8_t> id_set){
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
	return retval;
}

std::vector<uint8_t> compact_id_set(std::vector<id_t_> id_set){
	for(uint64_t i = 0;i < id_set.size();i++){
		id_api::assert_valid_id(
			id_set[i]);
	}
	std::vector<uint8_t> retval =
		compact_id_set_copy(
			id_set);
	retval.insert(
		retval.begin(),
		ID_SET_SCHEME_COPY);
	return retval;
}

/*
  Again super slow, but super simple

  Murphy's Law
 */

std::vector<uint8_t> add_id_to_set(std::vector<uint8_t> id_set, id_t_ id){
	std::vector<id_t_> out =
		expand_id_set(id_set);
	out.push_back(
		id);
	return compact_id_set(
		out);
}

std::vector<uint8_t> del_id_from_set(std::vector<uint8_t> id_set, id_t_ id){	
	std::vector<id_t_> out =
		expand_id_set(id_set);
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
		out);
}

uint64_t size_of_id_set(std::vector<uint8_t> id_set){
	return expand_id_set(id_set).size();
}

#undef ASSERT_LENGTH
