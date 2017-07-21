#include "id_set.h"
#include "id_set_copy.h"

std::vector<uint8_t> compact_id_set_copy(
	std::vector<id_t_> id_set){
	// convert the IDs to NBO
	std::vector<uint8_t> retval;
	for(uint64_t i = 0;i < id_set.size();i++){
		convert::nbo::to(
			id_set[i].data(),
			id_set[i].size());
	}
	retval.insert(
		retval.begin(),
		(uint8_t*)&(id_set[0]),
		(uint8_t*)&(id_set[id_set.size()]));
	return retval;
}

std::vector<id_t_> expand_id_set_copy(
	std::vector<uint8_t> id_set){
	ASSERT((id_set.size() % sizeof(id_t_)) == 0, P_ERR);
	std::vector<id_t_> retval;
	retval.insert(
		retval.begin(),
		(id_t_*)&(id_set[0]),
		(id_t_*)&(id_set[id_set.size()]));
	for(uint64_t i = 0;i < retval.size();i++){
		convert::nbo::from(
			retval[i].data(),
			retval[i].size());
	}
	return retval;
}
