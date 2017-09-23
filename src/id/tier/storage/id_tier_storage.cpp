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
	ADD_DATA_PTR(last_refresh_micro_s, public_ruleset);
	ADD_DATA_PTR(refresh_interval_micro_s, public_ruleset);
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
