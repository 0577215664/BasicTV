#include "id_tier_gc.h"
#include "../id_tier.h"

/*
  Make a list of all IDs that don't belong to us, and sort by their type

  Search all other ID tier states and count how often the ID exists in
  lower tiers (does it exist on the disk?)

  Make two lists of hashes, ours and other hashes. 

  OTHER HASH VECTOR:
  1. Make a vector of all tv_item_ts created by a hash in the tier in question
  (sort by last access timestamp), and shave off a quality level. If we only
  have one quality level, delete tv_item_t if we don't have any more data to 
  shave off.

  2. Start randomly deleting wallet sets

  3. Start deleting net_proto_peer_ts

  OUR HASH VECTOR:
  #1 from OTHER HASH VECTOR

  2. (After I put compression level and scheme in id_extra,) Go through all
  exported IDs, sort them by compression level (lowest to highest), and
  compress them to the highest compression level possible (The current default
  is Zstandard at full compression, so this doesn't make sense now)

 */

static std::pair<std::vector<id_t_>, std::vector<id_t_> > id_tier_gc_segment_vector(
	std::vector<id_t_> ids){
	std::pair<std::vector<id_t_>, std::vector<id_t_> > retval;
	const hash_t_ my_hash =
		get_id_hash(
			production_priv_key_id);
	for(uint64_t i = 0;i < ids.size();i++){
		if(get_id_hash(ids[i]) != my_hash){
			retval.first.push_back(
				ids[i]);
		}else{
			retval.second.push_back(
				ids[i]);
		}
	}
	return retval;
}

#pragma warning("id_tier::gc::to_remove just starts deleting stuff, probably shouldn't do that")

std::vector<id_t_> id_tier::gc::to_remove(
	id_tier_state_t *id_tier_state_ptr,
	uint64_t new_size){
	ASSERT(id_tier_state_ptr != nullptr, P_ERR);
	std::pair<std::vector<id_t_>, std::vector<id_t_> > breakdown =
		id_tier_gc_segment_vector(
			id_tier::lookup::ids::from_state(
				id_tier_state_ptr));
	// TODO: actually implement the 
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < breakdown.first.size();i++){
		// id_tier::operation::del_id_from_state(
		// 	id_tier_state_ptr->id.get_id(),
		// 	std::vector<id_t_>
	}
	return retval;
}
	

std::vector<id_t_> id_tier::gc::to_remove(
	id_t_ id_tier_state_id,
	uint64_t new_size){
	return to_remove(
		PTR_DATA(id_tier_state_id,
		       id_tier_state_t),
		new_size);
}
