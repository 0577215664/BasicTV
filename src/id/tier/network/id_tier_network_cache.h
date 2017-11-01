#ifndef ID_TIER_NETWORK_CACHE_H
#define ID_TIER_NETWORK_CACHE_H

// only used to update caches between peers
// puts ID caches of all non-networking tiers back to back, and
// removes any redundant IDs

// currently does not handle mod_inc, probably should tho

#include "../../id.h"

#include "../../../escape.h"

struct id_tier_network_cache_t{
private:
	/* std::vector<id_t_> ids; */
	std::vector<uint8_t> ids;
	std::vector<mod_inc_t_> mod_inc;
	std::vector<uint16_t> prob;

public:
	id_tier_network_cache_t();
	~id_tier_network_cache_t();

	GET_SET_S(ids, std::vector<uint8_t>);

	GET_SET_S(mod_inc, std::vector<mod_inc_t_>);
	FULL_VECTOR_CONTROL_S(mod_inc, mod_inc_t_);
	
	GET_SET_S(prob, std::vector<uint16_t>);
	FULL_VECTOR_CONTROL_S(prob, uint16_t);
};

/*
  Each peer keeps track of the other peer's version of the local cache, 
  and generates a sort of diff file to update their version. Diff file
  contains a weak hash at the end to verify the contents of the complete
  sequence, and requests a reset on a failure.

  Resets also use the diff system, 
 */

#define ID_TIER_NETWORK_CACHE_DIFF_TYPE_FULL 0
#define ID_TIER_NETWORK_CACHE_DIFF_TYPE_DIFF 1

#define ID_TIER_NETWORK_CACHE_DIFF_ESCAPE 0xEE

extern void id_tier_network_cache_apply_diff(
	id_tier_network_cache_t *cache_ptr,
	std::vector<uint8_t> diff);

extern std::vector<uint8_t> id_tier_network_cache_compute_diff(
	id_tier_network_cache_t *cache_ptr,
	id_tier_network_cache_t *old_cache_ptr,
	uint8_t diff_type);

extern void id_tier_network_compute_local_cache(
	id_tier_network_cache_t *cache_ptr);

#endif
