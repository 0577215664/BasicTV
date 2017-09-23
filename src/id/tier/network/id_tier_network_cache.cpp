#include "id_tier_network_cache.h"
#include "id_tier_network.h"

id_tier_network_cache_t::id_tier_network_cache_t() : id(this, TYPE_ID_TIER_NETWORK_CACHE_T){
	id.add_data_one_byte_vector(&id_cache, ~0);
}

id_tier_network_cache_t::~id_tier_network_cache_t(){
}

std::vector<uint8_t> id_tier_network_cache_create_serialize(){
	id_tier_network_cache_t *network_cache_ptr =
		new id_tier_network_cache_t;
	network_cache_ptr->set_id_cache(
		compact_id_set(
			id_tier::lookup::ids::from_tier(
				{{ID_TIER_MAJOR_MEM, 0},
					{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_UNENCRYPTED_UNCOMPRESSED},
					{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_UNENCRYPTED_COMPRESSED},
					{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_ENCRYPTED_COMPRESSED},
					{ID_TIER_MAJOR_DISK, 0}}),
			false));
     	const std::vector<uint8_t> retval =
		network_cache_ptr->id.export_data(
			ID_EXTRA_ENCRYPT & ID_EXTRA_COMPRESS);
	ID_TIER_DESTROY(network_cache_ptr->id.get_id());
   	network_cache_ptr = nullptr;
	return retval;
}
