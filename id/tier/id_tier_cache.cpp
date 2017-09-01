#include "id_tier.h"
#include "id_tier_cache.h"
#include "id_tier_define.h"

// We only need one medium for all cache, since they work interchangeably,
// the real cool-ness comes from the restrictions on the extra flags (which
// is how we enforce the encryption/compression states across all imported IDsg

ID_TIER_INIT_STATE(cache){
	id_tier_state_t *tier_state_ptr =
		new id_tier_state_t;
	id_tier_cache_state_t *cache_state_ptr =
		new id_tier_cache_state_t;
	tier_state_ptr->set_medium(
		ID_TIER_MEDIUM_CACHE); // ?
	tier_state_ptr->set_payload(
		cache_state_ptr);
	return tier_state_ptr->id.get_id();
}

ID_TIER_DEL_STATE(cache){
	GET_ALL_STATE_PTR(cache);
	delete tier_state_ptr;
	tier_state_ptr = nullptr;
	delete cache_state_ptr;
	cache_state_ptr = nullptr;
}

ID_TIER_ADD_DATA(cache){
	GET_ALL_STATE_PTR(cache);
	ASSERT(tier_state_ptr->storage.is_allowed_extra(
		       id_api::raw::fetch_extra(
			       data),
		       id_api::raw::fetch_id(
			       data)), P_ERR);
	const id_t_ id_new =
		id_api::raw::fetch_id(
			data);
	const mod_inc_t_ mod_inc_new =
		id_api::raw::fetch_mod_inc(
			data);
	ASSERT(get_id_hash(id_new) != blank_hash, P_ERR);
	bool wrote = false;
	for(uint64_t i = 0;i < cache_state_ptr->cache_data.size();i++){
		const id_t_ id_tmp =
			id_api::raw::fetch_id(
				cache_state_ptr->cache_data[i]);
		const mod_inc_t_ mod_inc_tmp =
			id_api::raw::fetch_mod_inc(
				cache_state_ptr->cache_data[i]);
		if(unlikely(id_tmp == id_new)){
			if(mod_inc_tmp > mod_inc_new){
				print("we are overriding an newer version with an older version, this is bad", P_ERR);
			}else{
				cache_state_ptr->cache_data[i] =
					data;
				wrote = true;
				break;
			}
		}
	}
	if(wrote == false){
		cache_state_ptr->cache_data.push_back(
			data);
		wrote = true;
	}
	tier_state_ptr->storage.add_id_buffer(
		std::make_pair(id_new,
			       mod_inc_new));
}

ID_TIER_DEL_ID(cache){
	GET_ALL_STATE_PTR(cache);
	tier_state_ptr->storage.del_id_buffer(
		id);
	for(uint64_t i = 0;i < cache_state_ptr->cache_data.size();i++){
		const id_t_ id_tmp =
			id_api::raw::fetch_id(
				cache_state_ptr->cache_data[i]);
		if(unlikely(id_tmp == id)){
			cache_state_ptr->cache_data.erase(
				cache_state_ptr->cache_data.begin()+i);
			break;
		}
	}
}

ID_TIER_GET_ID(cache){
	GET_ALL_STATE_PTR(cache);
	for(uint64_t i = 0;i < cache_state_ptr->cache_data.size();i++){
		const id_t_ id_tmp =
			id_api::raw::fetch_id(
				cache_state_ptr->cache_data[i]);
		if(unlikely(id_tmp == id)){
			// print("found ID " + id_breakdown(id_tmp) + " in cache", P_DEBUG);
			return cache_state_ptr->cache_data[i];
		}
	}
	print("couldn't find ID " + id_breakdown(id) + " in cache", P_DEBUG);
	return std::vector<uint8_t>({});
}

ID_TIER_UPDATE_CACHE(cache){
	ASSERT(state_id != ID_BLANK_ID, P_ERR);
	return;
}
