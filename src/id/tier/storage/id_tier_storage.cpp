#include "../../id.h"
#include "id_tier_storage.h"
#include "../../set/id_set.h"

id_tier_state_storage_t::id_tier_state_storage_t(){
}

id_tier_state_storage_t::~id_tier_state_storage_t(){
}

void id_tier_state_storage_t::list_virtual_data(data_id_t *id_){
	id = id_;
	ADD_DATA_PTR(flags, public_ruleset);
	ADD_DATA_PTR(total_bytes, public_ruleset);
	ADD_DATA_PTR(used_bytes, public_ruleset);
	id->add_data_one_byte_vector(&ids, ~0, public_ruleset);
	id->add_data_eight_byte_vector(&mod_incs, ~0, public_ruleset);
	id->add_data_one_byte_vector(&extras, ~0, public_ruleset);
	
}

void id_tier_state_storage_t::add_id_buffer(std::pair<id_t_, mod_inc_t_> tmp){
	ids = add_id_to_set(ids, tmp.first);
	mod_incs.push_back(tmp.second);
}

void id_tier_state_storage_t::del_id_buffer(id_t_ id_){
	ASSERT(mod_incs.size() == ids.size(), P_ERR);
	std::vector<id_t_> ids_ = get_ids();
	std::vector<id_t_>::iterator tmp =
		std::find(
			ids_.begin(),
			ids_.end(),
			id_);
	if(tmp != ids_.end()){
		mod_incs.erase(
			mod_incs.begin()+std::distance(
				ids_.begin(),
				tmp));
	}
	ids_.erase(tmp);
}

void id_tier_state_storage_t::set_id_buffer(std::vector<std::pair<id_t_, mod_inc_t_> > tmp){
	std::vector<id_t_> ids_new;
	std::vector<mod_inc_t_> mod_incs_new;
	ids_new.reserve(tmp.size());
	mod_incs_new.reserve(tmp.size());
	for(uint64_t i = 0;i < tmp.size();i++){
		ids_new.push_back(tmp[i].first);
		mod_incs_new.push_back(tmp[i].second);
	}
	set_ids(ids_new);
	set_mod_incs(mod_incs_new);
}

bool id_tier_state_storage_t::is_allowed_extra(uint8_t extra, id_t_ id_){
	return std::find(extras.begin(),
			 extras.end(),
			 extra) != extras.end() ||
		std::find(
			encrypt_blacklist.begin(),
			encrypt_blacklist.end(),
			get_id_type(id_)) != encrypt_blacklist.end();

}

std::vector<id_t_> id_tier_state_storage_t::get_ids(){
	return expand_id_set(ids, nullptr);
}

void id_tier_state_storage_t::set_ids(std::vector<id_t_> ids_){
	ids = compact_id_set(ids_, true);
}

id_tier_state_storage_entry_meta_t id_tier_state_storage_t::get_entry_meta(
	id_t_ id_){
	const std::vector<id_t_> id_vector =
		get_ids();
	ASSERT(id_vector.size() == mod_incs.size() &&
	       id_vector.size() == extras.size(), P_ERR);
	for(uint64_t i = 0;i < id_vector.size();i++){
		if(unlikely(id_vector[i] == id_)){
			id_tier_state_storage_entry_meta_t meta_entry;
			meta_entry.set_id(id_vector[i]);
			meta_entry.set_mod_inc(mod_incs[i]);
			meta_entry.set_extra(extras[i]);
			return meta_entry;
		}
	}
	print("can't find the ID in the tier state vector", P_ERR);
	return id_tier_state_storage_entry_meta_t();
}

std::vector<uint8_t> id_tier_state_storage_cache_t::search_cache_for_id(
	id_t_ id_){
	for(uint64_t i = 0;i < cache.size();i++){
		if(unlikely(cache[i].second.get_id() == id_)){
			return cache[i].first;
		}
	}
	print("ID not found in tier cache", P_ERR);
	return {};
}
