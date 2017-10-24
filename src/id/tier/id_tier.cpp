#include "id_tier.h"
#include "move/id_tier_move.h"
#include "memory/id_tier_memory.h"
#include "memory/id_tier_memory_helper.h"
#include "id_tier_cache.h"
#include "disk/id_tier_disk.h"
#include "network/id_tier_network.h"

#include "../../settings.h"

// default directory relies on HOME path
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "../../file.h"

std::vector<type_t_> memory_locked = {
	TYPE_ID_TIER_STATE_T
};

std::vector<id_tier_medium_t> id_tier_medium = {
	id_tier_medium_t(
		id_tier_mem_init_state,
		id_tier_mem_del_state,
		id_tier_mem_loop,
		id_tier_mem_update_cache),
	id_tier_medium_t(
		id_tier_cache_init_state,
		id_tier_cache_del_state,
		id_tier_cache_loop,
		id_tier_cache_update_cache),
	id_tier_medium_t(
		id_tier_disk_init_state,
		id_tier_disk_del_state,
		id_tier_disk_loop,
		id_tier_disk_update_cache),
	id_tier_medium_t(
		id_tier_network_init_state,
		id_tier_network_del_state,
		id_tier_network_loop,
		id_tier_network_update_cache)
};

id_tier_medium_t id_tier::get_medium(uint8_t medium_type){
	ASSERT(medium_type == ID_TIER_MEDIUM_MEM ||
	       medium_type == ID_TIER_MEDIUM_CACHE ||
	       medium_type == ID_TIER_MEDIUM_DISK ||
	       medium_type == ID_TIER_MEDIUM_NETWORK, P_ERR);
	       
	return id_tier_medium.at(medium_type-1); // 0 is undefined
}

id_t_ id_tier::state_tier::only_state_of_tier(
	uint8_t tier_major,
	uint8_t tier_minor){
	id_t_ id = ID_BLANK_ID;
	/*
	  TODO: To stop a deadlock, we need to ensure that all id_tier_state_t stay
	  in memory. Enforce that
	 */
	id_tier_state_t *tier_state_ptr =
		mem_helper::lookup::tier_state(
			std::vector<std::pair<uint8_t, uint8_t> >({
				std::make_pair(
					tier_major,
					tier_minor)})).at(0);
	if(tier_state_ptr != nullptr){
		return tier_state_ptr->id.get_id();
	}
	print("state isn't in memory at the moment, this needs to be ensured", P_ERR);
	std::vector<id_t_> id_state_list =
		ID_TIER_CACHE_GET(
			TYPE_ID_TIER_STATE_T);
	std::vector<id_t_> match_vector;
	for(uint64_t i = 0;i < id_state_list.size();i++){
		id_tier_state_t *id_tier_state_ptr =
			PTR_DATA(id_state_list[i],
				 id_tier_state_t);
		CONTINUE_IF_NULL(id_tier_state_ptr, P_ERR);
		if(id_tier_state_ptr->get_tier_major() == tier_major &&
		   id_tier_state_ptr->get_tier_minor() == tier_minor){
			return id_state_list[i];
		}
	}
	return id;
}

std::vector<id_t_> id_tier::state_tier::optimal_state_vector_of_tier(
	uint8_t tier_major,
	uint8_t tier_minor){
	std::vector<id_t_> retval;
	std::vector<id_t_> id_state_list =
		ID_TIER_CACHE_GET(
			TYPE_ID_TIER_STATE_T);
	for(uint64_t i = 0;i < id_state_list.size();i++){
		id_tier_state_t *id_tier_state_ptr =
			PTR_DATA(id_state_list[i],
				 id_tier_state_t);
		CONTINUE_IF_NULL(id_tier_state_ptr, P_ERR);
		if(id_tier_state_ptr->get_tier_major() == tier_major &&
		   id_tier_state_ptr->get_tier_minor() == tier_minor){
			retval.push_back(
				id_state_list[i]);
		}
	}
	return retval;
}

std::vector<id_t_> id_tier::state_tier::optimal_state_vector_of_tier_vector(
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector){
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < tier_vector.size();i++){
		std::vector<id_t_> tmp =
			optimal_state_vector_of_tier(
				tier_vector[i].first,
				tier_vector[i].second);
		retval.insert(
			retval.end(),
			tmp.begin(),
			tmp.end());
	}
	return retval;
}

std::vector<std::tuple<id_t_, uint8_t, uint8_t> > id_tier::operation::valid_state_with_id(
	id_t_ id){
	std::vector<std::tuple<id_t_, uint8_t, uint8_t> > retval;
	std::vector<id_t_> id_tier_states =
		ID_TIER_CACHE_GET(
			TYPE_ID_TIER_STATE_T);
	for(uint64_t i = 0;i < id_tier_states.size();i++){
		id_tier_state_t *id_tier_state_ptr =
			PTR_DATA(id_tier_states[i],
				 id_tier_state_t);
		CONTINUE_IF_NULL(id_tier_state_ptr, P_WARN);
		std::vector<std::pair<id_t_, mod_inc_t_> > state_cache =
			id_tier::lookup::id_mod_inc::from_state(
				id_tier_state_ptr);
		print("don't bother right now", P_CRIT);
		std::vector<id_t_> all_ids =
			id_tier::lookup::ids::from_state(
				id_tier_state_ptr);
		if(std::find(
			   all_ids.begin(),
			   all_ids.end(),
			   id) != all_ids.end()){
			retval.push_back(
				std::make_tuple(
					id_tier_states[i],
					id_tier_state_ptr->get_tier_major(),
					id_tier_state_ptr->get_tier_minor()));
		}
	}
	return retval;
}

void id_tier::operation::shift_data_to_state(
	id_t_ start_state_id,
	id_t_ end_state_id,
	std::vector<id_t_> *id_vector){
	shift_data_to_state(
		PTR_DATA(start_state_id, id_tier_state_t),
		PTR_DATA(end_state_id, id_tier_state_t),
		id_vector);
}

void id_tier::operation::shift_data_to_state(
	id_tier_state_t *start_state_ptr,
	id_tier_state_t *end_state_ptr,
	std::vector<id_t_> *id_vector){

	ASSERT(start_state_ptr != nullptr, P_ERR);
	ASSERT(end_state_ptr != nullptr, P_ERR);
	std::vector<id_t_> first_buffer =
		id_tier::lookup::ids::from_state(
			start_state_ptr);
	std::vector<id_t_> second_buffer =
		id_tier::lookup::ids::from_state(
			end_state_ptr);
	for(uint64_t i = 0;i < id_vector->size();i++){
		if(std::find(
			   memory_locked.begin(),
			   memory_locked.end(),
			   get_id_type((*id_vector)[i])) != memory_locked.end()){
			// can't leave memory because it is needed for core
			// functionality (id_tier_state_t only at the moment)
			print("can't remove id_tier_state_t, needed for core function", P_DEBUG);
			continue;
		}
		if(start_state_ptr->get_tier_major() == 0 &&
		   get_id_hash((*id_vector)[i]) !=
		   get_id_hash(production_priv_key_id)){
			continue;
		}
		if(std::find(
			   first_buffer.begin(),
			   first_buffer.end(),
			   (*id_vector)[i]) != first_buffer.end()){
			std::vector<uint8_t> shift_payload;
			std::vector<uint8_t> allowed_extra =
				end_state_ptr->storage.get_extras();
				ASSERT(allowed_extra.size() > 0, P_ERR);
			try{
				const std::vector<std::vector<uint8_t> > state_data =
					get_data_from_state(
						std::vector<id_t_>({start_state_ptr->id.get_id()}),
						std::vector<id_t_>({(*id_vector)[i]}));
				if(state_data.size() == 0){
					print("no data returned from get_data_from_state", P_WARN);
					continue;
				}
				shift_payload = state_data[0];
			}catch(...){
				print("get_data_from_state failed", P_ERR);
			}
			CONTINUE_IF_TRUE(shift_payload.size() == 0);
			if(get_id_hash((*id_vector)[i]) ==
			   get_id_hash(production_priv_key_id)){
				try{
					const data_id_transport_rules_t tmp_rules(
						std::vector<std::pair<uint8_t, uint8_t> >(
							{std::make_pair(end_state_ptr->get_tier_major(),
									end_state_ptr->get_tier_minor())}),
						std::vector<uint8_t>({}));
					shift_payload =
						id_api::raw::strip_to_transportable(
							shift_payload,
							tmp_rules);
				}catch(...){
					print("strip_to_transportable failed", P_ERR);
				}
			}
			CONTINUE_IF_TRUE(shift_payload.size() == 0);
			try{
				shift_payload =
					id_api::raw::force_to_extra(
						shift_payload,
						allowed_extra[0]); // TODO: compute this stuff better (?)
			}catch(...){
				print("force_to_extra failed", P_ERR);
			}
			try{
				add_data_to_state(
					std::vector<id_t_>({end_state_ptr->id.get_id()}),
					std::vector<std::vector<uint8_t> >({shift_payload}));
				id_vector->erase(
					id_vector->begin()+i);
			}catch(...){
				print("couldn't shift id " + id_breakdown((*id_vector)[i]) + " over to new device (set)", P_WARN);
			}
			
		}
	}
}

void id_tier::operation::del_id_from_state(
	std::vector<id_t_> state_id,
	std::vector<id_t_> id){
	id_tier_operation_entry_t oper;
	oper.set_ids(id);
	oper.set_operation(ID_TIER_OPERATION_DEL);
	for(uint64_t i = 0;i < state_id.size();i++){
		try{
			id_tier_state_t *tier_state_ptr =
				PTR_DATA(state_id[i],
					 id_tier_state_t);
			PRINT_IF_NULL(tier_state_ptr, P_ERR);
			tier_state_ptr->operations.push_back(
				oper);
		}catch(...){}
	}
}

std::vector<std::vector<uint8_t> > id_tier::operation::get_data_from_state(
	std::vector<id_t_> state_id,
	std::vector<id_t_> id_vector){
	std::vector<std::vector<uint8_t> > retval;
	for(uint64_t c = 0;c < id_vector.size();c++){
		for(uint64_t i = 0;i < state_id.size();i++){
			try{
				id_tier_state_t *tier_state_ptr =
					PTR_DATA(state_id[i],
						 id_tier_state_t);
				CONTINUE_IF_NULL(tier_state_ptr, P_WARN);
				std::vector<id_tier_transport_entry_t>::iterator req_iter =
					std::find_if(
						tier_state_ptr->inbound_transport.begin(),
						tier_state_ptr->inbound_transport.end(),
						[&](const id_tier_transport_entry_t &rhs){
							return rhs.get_payload_id() == id_vector[i];
						});	
				if(req_iter == tier_state_ptr->inbound_transport.end()){
					id_tier_transport_entry_t transport_entry;
					transport_entry.set_payload_id(
						id_vector[i]);
					tier_state_ptr->inbound_transport.push_back(
						transport_entry);
					id_tier_medium_t medium =
						id_tier::get_medium(
							tier_state_ptr->get_medium());
					medium.loop(
						tier_state_ptr->id.get_id());
					req_iter =
						std::find_if(
							tier_state_ptr->inbound_transport.begin(),
							tier_state_ptr->inbound_transport.end(),
							[&](const id_tier_transport_entry_t &rhs){
								return rhs.get_payload_id() == id_vector[i];
							});
				}

				if(req_iter->get_payload().size() != 0){
					retval.push_back(
						req_iter->get_payload());
					tier_state_ptr->inbound_transport.erase(
						req_iter);
				}
			}catch(...){}
		}
	}
	return retval;
}

void id_tier::operation::add_data_to_state(
	std::vector<id_t_> state_id,
	std::vector<std::vector<uint8_t> > data_vector){
	for(uint64_t i = 0;i < data_vector.size();i++){
		std::array<std::vector<uint8_t>, 4> extra_vector;
		for(uint64_t c = 0;c < state_id.size();c++){
			try{
				id_tier_state_t *tier_state_ptr =
					PTR_DATA(state_id[c],
						 id_tier_state_t);
				PRINT_IF_NULL(tier_state_ptr, P_UNABLE);
				// id_tier_medium_t medium =
				// 	id_tier::get_medium(
				// 		tier_state_ptr->get_medium());
				const extra_t_ extra_byte =
					tier_state_ptr->storage.get_extras().at(0);
				if(extra_vector[extra_byte].size() == 0){
					for(uint8_t ext = 0;ext < 4;ext++){
						// lower extras are simpler
						try{
							if(extra_vector[ext].size() == 0){
								continue;
							}
							extra_vector[extra_byte] =
								id_api::raw::force_to_extra(
									extra_vector[ext],
									extra_byte);
							break;
						}catch(...){}
					}
					if(extra_vector[extra_byte].size() == 0){
						extra_vector[extra_byte] =
							id_api::raw::force_to_extra(
								data_vector[i],
								extra_byte);
					}
				}
				id_tier_transport_entry_t transport_entry;
				transport_entry.set_payload(
					extra_vector[extra_byte]);
				transport_entry.set_payload_id(
					id_api::raw::fetch_id(
						extra_vector[extra_byte]));
				tier_state_ptr->outbound_transport.push_back(
					transport_entry);
				id_tier_medium_t medium =
					id_tier::get_medium(
						tier_state_ptr->get_medium());
				medium.loop(
					tier_state_ptr->id.get_id());

			}catch(...){
				print("couldn't insert " + id_breakdown(id_api::raw::fetch_id(data_vector[i])) + " into " +
				      id_breakdown(state_id[c]), P_WARN);
			}
		}
	}
}

id_tier_state_t::id_tier_state_t() : id(this, TYPE_ID_TIER_STATE_T){
	storage.list_virtual_data(&id);
	benchmark.list_virtual_data(&id);
}

id_tier_state_t::~id_tier_state_t(){
}


// creation of tier states happens in the main init()

static void id_tier_init_disk(){
	id_tier_medium_t disk_medium_ptr =
		id_tier::get_medium(
			ID_TIER_MEDIUM_DISK);
	id_tier_state_t *tier_state_ptr =
		PTR_DATA(disk_medium_ptr.init_state(), id_tier_state_t);
	tier_state_ptr->set_medium(
		ID_TIER_MEDIUM_DISK);
	tier_state_ptr->set_tier_major(
		ID_TIER_MAJOR_DISK);
	tier_state_ptr->set_tier_minor(
		0);
	tier_state_ptr->storage.set_extras(
		{ID_EXTRA_ENCRYPT & ID_EXTRA_COMPRESS});

	id_tier_disk_state_t *disk_state_ptr =
		reinterpret_cast<id_tier_disk_state_t*>(
			tier_state_ptr->get_payload());
	ASSERT(disk_state_ptr != nullptr, P_ERR);

	
	std::string path;
	try{
		path =
			settings::get_setting(
				"data_folder");
		if(path[path.size()-1] != SLASH){
			path += std::string(1, SLASH);
		}
	}catch(...){}
	if(path == ""){
#ifdef __linux
		struct passwd *pw = getpwuid(getuid());
		path = pw->pw_dir + (std::string)"/.";
#else
		path = "";
#endif
		path += "BasicTV";
	}
	print("setting data folder path to '" + path + "'", P_NOTE);
	// P_V_S(path, P_VAR);
	disk_state_ptr->path =
		convert::string::to_bytes(
			path);
	disk_medium_ptr.update_cache(
		tier_state_ptr->id.get_id());
}

static void id_tier_init_cache(){
	id_tier_medium_t cache_medium_ptr =
		id_tier::get_medium(
			ID_TIER_MEDIUM_CACHE);
	// can have a vector of std::pair<uint8_t> (allowed extra), but
	// the cache is set up so there's only one extra configuration per
	// minor tier
	std::vector<std::tuple<id_tier_state_t*, uint8_t, uint8_t, std::pair<uint8_t, uint8_t> > > cache_data =
		std::vector<std::tuple<id_tier_state_t*, uint8_t, uint8_t, std::pair<uint8_t, uint8_t> > >({
				std::make_tuple(
					PTR_DATA(cache_medium_ptr.init_state(), id_tier_state_t),
					0,
					ID_TIER_MEDIUM_CACHE,
					std::make_pair(
						ID_TIER_MAJOR_CACHE,
						ID_TIER_MINOR_CACHE_UNENCRYPTED_UNCOMPRESSED)),
					std::make_tuple(
						PTR_DATA(cache_medium_ptr.init_state(), id_tier_state_t),
						ID_EXTRA_COMPRESS,
						ID_TIER_MEDIUM_CACHE,
						std::make_pair(
							ID_TIER_MAJOR_CACHE,
							ID_TIER_MINOR_CACHE_UNENCRYPTED_COMPRESSED)),
					std::make_tuple(
						PTR_DATA(cache_medium_ptr.init_state(), id_tier_state_t),
						ID_EXTRA_ENCRYPT & ID_EXTRA_COMPRESS,
						ID_TIER_MEDIUM_CACHE,
						std::make_pair(
							ID_TIER_MAJOR_CACHE,
							ID_TIER_MINOR_CACHE_ENCRYPTED_COMPRESSED)),
					});
	for(uint64_t i = 0;i < cache_data.size();i++){
		id_tier_state_t *tier_state_ptr =
			std::get<0>(cache_data[i]);
		tier_state_ptr->storage.set_extras(
			{std::get<1>(cache_data[i])});
		tier_state_ptr->set_medium(
			std::get<2>(cache_data[i]));
		tier_state_ptr->set_tier_major(
			std::get<3>(cache_data[i]).first);
		tier_state_ptr->set_tier_minor(
			std::get<3>(cache_data[i]).second);
	}
}

void id_tier_init(){
	// memory is handled in-line in init() for private key loading
	// id_tier_init_cache();
	id_tier_init_disk();
}

void id_tier_loop(){
	std::vector<id_t_> tier_state_vector =
		ID_TIER_CACHE_GET(
			TYPE_ID_TIER_STATE_T);
	for(uint64_t i = 0;i < tier_state_vector.size();i++){
		for(uint64_t c = 0;c < tier_state_vector.size();c++){
			if(i == c){
				continue;
			}
			const std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > move_logic =
				tier_move_logic(
					tier_state_vector[i],
					tier_state_vector[c]);
			for(uint64_t a = 0;a < move_logic.size();a++){
				id_t_ from_id = ID_BLANK_ID;
				id_t_ to_id = ID_BLANK_ID;
				switch(std::get<3>(move_logic[a])){
				case COPY_UP:
					from_id = std::get<1>(move_logic[a]);
					to_id = std::get<2>(move_logic[a]);
					break;
				case COPY_DOWN: // reversed
					from_id = std::get<2>(move_logic[a]);
					to_id = std::get<1>(move_logic[a]);
					break;
				default:
					print("invalid copy setting", P_ERR);
				}
				std::vector<id_t_> shift_operation(
					{std::get<0>(move_logic[a])});
				try{
					id_tier::operation::shift_data_to_state(
						from_id,
						to_id,
						&shift_operation);
				}catch(...){}
				if(shift_operation.size() == 1){
					// completely normal behavior, since we
					// let shift_data_to_state handle a lot
					// of the rules
					// print("couldn't shift data over", P_SPAM);
				}else{
					// update this if we have bulk transfers
					// refer to tiers as tier major and tier minor
					id_tier_state_t *from_state_ptr =
						PTR_DATA(from_id,
							 id_tier_state_t);
					ASSERT(from_state_ptr != nullptr, P_ERR);
					id_tier_state_t *to_state_ptr =
						PTR_DATA(to_id,
							 id_tier_state_t);
					ASSERT(to_state_ptr != nullptr, P_ERR);
					const std::string from_tier =
						std::to_string(from_state_ptr->get_tier_major()) + "." +
						std::to_string(from_state_ptr->get_tier_minor());
					const std::string to_tier =
						std::to_string(to_state_ptr->get_tier_major()) + "." +
						std::to_string(to_state_ptr->get_tier_minor());
					print("shifted data " + id_breakdown(std::get<0>(move_logic[a])) + " from tier " + from_tier + " to tier " + to_tier, P_SPAM);
				}
			}
		}
		id_tier_state_t *tier_state_ptr =
			PTR_DATA(tier_state_vector[i],
				 id_tier_state_t);
		CONTINUE_IF_NULL(tier_state_ptr, P_WARN);
		id_tier_medium_t medium =
			id_tier::get_medium(
				tier_state_ptr->get_medium());
		try{
			medium.loop(
				tier_state_ptr->id.get_id());
		}catch(...){}
		if(tier_state_ptr->storage.cache.update_freq.due()){
			tier_state_ptr->control.flags |= ID_TIER_CONTROL_FLAG_UPDATE_CACHE;
			tier_state_ptr->storage.cache.update_freq.reset();
		}
		if(tier_state_ptr->control.flags & ID_TIER_CONTROL_FLAG_UPDATE_CACHE){
			medium.update_cache(
				tier_state_ptr->id.get_id());
			tier_state_ptr->control.flags &=
				~(static_cast<uint64_t>(
					  ID_TIER_CONTROL_FLAG_UPDATE_CACHE));
		}
	}
}

void id_tier_close(){
	id_tier_loop();
}
