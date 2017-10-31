#include "id_tier_network_cache.h"
#include "id_tier_network.h"

#include "id_tier_network_cache_diff_diff.h"
#include "id_tier_network_cache_diff_full.h"


id_tier_network_cache_t::id_tier_network_cache_t(){
}

id_tier_network_cache_t::~id_tier_network_cache_t(){
}

// hash is SHA 256 & 0xFFFF, collisions aren't catastrophic
// weak hashes here aren't a security flaw, since the entire channel is
// asymmetrically encrypted, only hash DIFF, not FULL

/*
  First byte is the type of writing:
  DIFF_TYPE_FULL adds all the IDs in a vector

  DIFF_TYPE_DIFF is nice, it:
  1. Creates an unordered ID set
  2. Expands that ID set out to get the transported list
  3. Inserts a list of 8-byte entries to denote the mod_inc
*/

void id_tier_network_cache_apply_diff(
	id_tier_network_cache_t *cache_ptr,
	std::vector<uint8_t> diff){
}

std::vector<uint8_t> id_tier_network_cache_compute_diff(
	id_tier_network_cache_t *cache_ptr,
	id_tier_network_cache_t *old_cache_ptr,
	uint8_t diff_type){
	std::vector<uint8_t> retval({diff_type});
	const std::vector<uint8_t> * const ids =
		cache_ptr->get_const_ptr_ids();
	const std::vector<mod_inc_t_> * const mod_inc =
		cache_ptr->get_const_ptr_mod_inc();
	switch(diff_type){
	case ID_TIER_NETWORK_CACHE_DIFF_TYPE_FULL:
		return id_tier_network_cache_compute_diff_full(
			old_cache_ptr,
			ids,
			mod_inc);
	case ID_TIER_NETWORK_CACHE_DIFF_TYPE_DIFF:
		return id_tier_network_cache_compute_diff_diff(
			old_cache_ptr,
			ids,
			mod_inc);
	default:
		print("invalid diff_type", P_ERR);
	}
	// TODO: operator overloading
	old_cache_ptr->set_ids(
		cache_ptr->get_ids());
	old_cache_ptr->set_mod_inc(
		cache_ptr->get_mod_inc());
	return retval;
}
