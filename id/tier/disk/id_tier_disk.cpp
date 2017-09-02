#include "id_tier_disk.h"
#include "../id_tier.h"
#include "../id_tier_define.h"

#include "../../id.h"
#include "../../id_api.h"

#include "../../../file.h"
#include "../../../system.h"

/*
  Since the type exists through the ID, we just save everything to 
  one giant folder
 */

std::vector<std::pair<id_t_, mod_inc_t_> > gen_id_buffer(
	std::vector<uint8_t> path){
	std::vector<std::pair<id_t_, mod_inc_t_> > retval;
	const std::vector<std::string> all_files =
		system_handler::find(
			convert::string::from_bytes(
				path),
			"");
	print("found " + std::to_string(all_files.size()) + " files under path " +
	      convert::string::from_bytes(path), P_DEBUG);
	for(uint64_t i = 0;i < all_files.size();i++){
		// TODO: if this exists in a directory with an underscore,
		// it'll falsely register everything and read it in.
		std::string id_component;
		std::string mod_inc_component;
		uint64_t underscore_pos = 0;
		try{
			underscore_pos =
				all_files[i].find_last_of('_');
			if(underscore_pos == std::string::npos){
				continue;
			}
			uint64_t last_slash_pos =
				all_files[i].find_last_of('/');
			ASSERT(last_slash_pos != std::string::npos, P_ERR);
			id_component =
				all_files[i].substr(
					last_slash_pos+1,
					underscore_pos-last_slash_pos-1);
			mod_inc_component =
				all_files[i].substr(
					underscore_pos+1,
					all_files[i].size());
		}catch(...){}
		if(id_component == "" || mod_inc_component == ""){
			continue;
		}
		P_V_S(id_component, P_DEBUG);
		P_V_S(mod_inc_component, P_DEBUG);
		retval.push_back(
			std::make_pair(
				convert::array::id::from_hex(id_component),
				std::stoull(mod_inc_component)));
	}
	return retval;
}

static mod_inc_t_ mod_inc_from_id_buffer(
	std::vector<std::pair<id_t_, mod_inc_t_> > id_buffer,
	id_t_ id){
	for(uint64_t i = 0;i < id_buffer.size();i++){
		if(unlikely(std::get<0>(id_buffer[i]) == id)){
			return std::get<1>(id_buffer[i]);
		}
	}
	print("no mod_inc available", P_UNABLE);
	return 0;
}

ID_TIER_INIT_STATE(disk){
	id_tier_state_t *tier_state_ptr =
		new id_tier_state_t;
	id_tier_disk_state_t *disk_state_ptr =
		new id_tier_disk_state_t;
	tier_state_ptr->set_medium(
		ID_TIER_MEDIUM_DISK);
	tier_state_ptr->set_payload(
		disk_state_ptr);
	return tier_state_ptr->id.get_id();
}

ID_TIER_DEL_STATE(disk){
	GET_ALL_STATE_PTR(disk);
	delete tier_state_ptr;
	tier_state_ptr = nullptr;
	delete disk_state_ptr;
	disk_state_ptr = nullptr;		
}

static std::string gen_filename(id_t_ id, mod_inc_t_ mod_inc){
	return convert::array::id::to_hex(id) + "_"  + std::to_string(mod_inc);
}

// TODO: move from nodisk over to actually parsing and removing
// cache information as well. Cache information shouldn't be exported
// anyways, but we can get more granularity when it comes to where
// data actually goes

// strip_to_only_rules attempts to do this, but what's being asked is
// pretty impossible with the current exporting model

ID_TIER_ADD_DATA(disk){
	GET_ALL_STATE_PTR(disk);
	const id_t_ new_id =
		id_api::raw::fetch_id(
			data);
	const mod_inc_t_ mod_inc_new =
		id_api::raw::fetch_mod_inc(
			data);
	const extra_t_ extra_new =
		id_api::raw::fetch_extra(
			data);
	ASSERT(tier_state_ptr->storage.is_allowed_extra(
		       extra_new,
		       new_id), P_ERR);
	const std::string filename =
		gen_filename(
			new_id,
			mod_inc_new);
	ASSERT(tier_state_ptr->storage.is_allowed_extra(
		       ID_EXTRA_ENCRYPT & ID_EXTRA_COMPRESS,
		       new_id), P_ERR);
	const std::string pathname =
		convert::string::from_bytes(
			disk_state_ptr->path);
	file::write_file_vector(pathname + "/" + filename, data);

	tier_state_ptr->storage.add_id_buffer(
	 	std::make_pair(
	 		new_id,
	 		mod_inc_new));
}

ID_TIER_DEL_ID(disk){
	GET_ALL_STATE_PTR(disk);
	std::string filename =
		gen_filename(
			id,
			mod_inc_from_id_buffer(
				id_tier::lookup::id_mod_inc::from_state(tier_state_ptr),
				id));
	const std::string pathname =
		convert::string::from_bytes(
			disk_state_ptr->path);
	system_handler::rm(pathname + "/" + filename);
	tier_state_ptr->storage.del_id_buffer(
		id);
}

ID_TIER_GET_ID(disk){
	GET_ALL_STATE_PTR(disk);
	std::vector<uint8_t> retval;
	try{
		retval =
			file::read_file_vector(
				convert::string::from_bytes(
					disk_state_ptr->path) + "/" +
				gen_filename(
					id,
				        mod_inc_from_id_buffer(
						id_tier::lookup::id_mod_inc::from_state(tier_state_ptr),
						id)));
	}catch(...){}
	return retval;
}

ID_TIER_GET_HINT_ID(disk){
}

ID_TIER_UPDATE_CACHE(disk){
	GET_ALL_STATE_PTR(disk);
	system_handler::mkdir(
		convert::string::from_bytes(
			disk_state_ptr->path) + "/");
	tier_state_ptr->storage.set_id_buffer(
		gen_id_buffer(
			disk_state_ptr->path));
	tier_state_ptr->storage.set_last_refresh_micro_s(
		get_time_microseconds());
}
