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
	ASSERT(diff.size() > 0, P_ERR);
	const uint8_t diff_type =
		diff[0];
	diff.erase(diff.begin());
	switch(diff_type){
	case ID_TIER_NETWORK_CACHE_DIFF_TYPE_FULL:
		id_tier_network_cache_apply_diff_full(
			cache_ptr,
			diff);
		break;
	case ID_TIER_NETWORK_CACHE_DIFF_TYPE_DIFF:
		id_tier_network_cache_apply_diff_diff(
			cache_ptr,
			diff);
		break;
	default:
		print("invalid diff_type", P_ERR);
	}
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
	if(old_cache_ptr != nullptr){
		old_cache_ptr->set_ids(
			cache_ptr->get_ids());
		old_cache_ptr->set_mod_inc(
			cache_ptr->get_mod_inc());
	}
	std::raise(SIGINT);
	return retval;
}

static void id_tier_network_id_compress(
	std::vector<id_t_> *new_ids,
	std::vector<uint8_t> *new_id_set,
	std::vector<mod_inc_t_> *new_mod_inc,
	std::vector<std::pair<id_t_, mod_inc_t_> > tmp){
	new_ids->clear();
	for(uint64_t i = 0;i < tmp.size();i++){
		new_ids->push_back(
			tmp[i].first);
	}
	*new_id_set = compact_id_set(
		*new_ids,
		false);
	*new_ids = expand_id_set(
		*new_id_set,
		nullptr);

	for(uint64_t i = 0;i < tmp.size();i++){
		for(uint64_t c = 0;c < new_ids->size();c++){
			if((*new_ids)[c] == tmp[i].first){
				new_mod_inc->push_back(
					tmp[i].second);
				break;
			}
		}
	}
}

void id_tier_network_compute_local_cache(
	id_tier_network_cache_t *cache_ptr){
	std::vector<id_t_> ids;
	std::vector<uint8_t> id_set;
	std::vector<mod_inc_t_> mod_inc;
	id_tier_network_id_compress(
		&ids,
		&id_set,
		&mod_inc,
		id_tier::lookup::id_mod_inc::from_tier(
			all_tiers));
	cache_ptr->set_ids(id_set);
	cache_ptr->set_mod_inc(mod_inc);
	cache_ptr->set_prob(std::vector<uint16_t>(mod_inc.size(), ~static_cast<uint16_t>(0)));
}
