#ifndef ID_TIER_NETWORK_CACHE_H
#define ID_TIER_NETWORK_CACHE_H

// only used to update caches between peers
// puts ID caches of all non-networking tiers back to back, and
// removes any redundant IDs

// currently does not handle mod_inc, probably should tho

#include "../../id.h"

struct id_tier_network_cache_t{
private:
	std::vector<uint8_t> id_cache;
public:
	data_id_t id;
	id_tier_network_cache_t();
	~id_tier_network_cache_t();
	GET_SET(id_cache, std::vector<uint8_t>);
};

extern std::vector<uint8_t> id_tier_network_cache_create_serialize();

#endif
