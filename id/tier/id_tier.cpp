#include "id_tier.h"
#include "memory/id_tier_memory.h"
#include "memory/id_tier_memory_helper.h"
#include "id_tier_cache.h"
#include "disk/id_tier_disk.h"

// default directory relies on HOME path
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

std::vector<type_t_> memory_locked = {
	TYPE_ID_TIER_STATE_T
};

std::vector<std::pair<uint8_t, uint8_t> > all_tiers = {
	{ID_TIER_MAJOR_MEM, 0},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_UNENCRYPTED_UNCOMPRESSED},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_UNENCRYPTED_COMPRESSED},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_ENCRYPTED_COMPRESSED}//,
	// {ID_TIER_MAJOR_DISK, 0}
};

std::vector<id_tier_medium_t> id_tier_medium = {
	id_tier_medium_t(
		id_tier_mem_init_state,
		id_tier_mem_del_state,
		id_tier_mem_add_data,
		id_tier_mem_del_id,
		id_tier_mem_get_id,
		id_tier_mem_update_cache),
	id_tier_medium_t(
		id_tier_cache_init_state,
		id_tier_cache_del_state,
		id_tier_cache_add_data,
		id_tier_cache_del_id,
		id_tier_cache_get_id,
		id_tier_cache_update_cache),
	id_tier_medium_t(
		id_tier_disk_init_state,
		id_tier_disk_del_state,
		id_tier_disk_add_data,
		id_tier_disk_del_id,
		id_tier_disk_get_id,
		id_tier_disk_update_cache)
};

id_tier_medium_t id_tier::get_medium(uint8_t medium_type){
	ASSERT(medium_type == ID_TIER_MEDIUM_MEM ||
	       medium_type == ID_TIER_MEDIUM_CACHE ||
	       medium_type == ID_TIER_MEDIUM_DISK, P_ERR);
	       
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
			id_tier_state_ptr->get_id_buffer();
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

	id_tier_medium_t first_medium =
		id_tier::get_medium(
			start_state_ptr->get_medium());
	id_tier_medium_t second_medium =
		id_tier::get_medium(
			end_state_ptr->get_medium());
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
			print("can't export data from tier 0 we can't encrypt", P_DEBUG);
			continue;
		}
		if(std::find(
			   first_buffer.begin(),
			   first_buffer.end(),
			   (*id_vector)[i]) != first_buffer.end()){
			std::vector<uint8_t> shift_payload;
			try{
				std::vector<uint8_t> allowed_extra =
					end_state_ptr->get_allowed_extra();
				ASSERT(allowed_extra.size() > 0, P_ERR);
				shift_payload =
					first_medium.get_id(
						start_state_ptr->id.get_id(),
						(*id_vector)[i]);
				if(shift_payload.size() == 0){
					continue;
				}
				if(get_id_hash((*id_vector)[i]) ==
				   get_id_hash(production_priv_key_id)){
					// can't re-encrypt data we don't have
					shift_payload =
						id_api::raw::strip_to_only_rules(
							shift_payload,
							std::vector<uint8_t>({}),
							std::vector<uint8_t>({
								ID_DATA_EXPORT_RULE_ALWAYS}),
							std::vector<uint8_t>({}));
				} // don't care, can't reconstruct
				// TODO: can probably decrypt and check, blacklist against DDoS?
				if(shift_payload.size() == 0){
					// probably set to completely non-exportable
					continue;
				}
				shift_payload =
					id_api::raw::force_to_extra(
						shift_payload,
						allowed_extra[0]); // TODO: compute this stuff better (?)
				try{
					second_medium.add_data(
						end_state_ptr->id.get_id(),
						shift_payload);
					id_vector->erase(
						id_vector->begin()+i);
				}catch(...){
					print("couldn't shift id " + id_breakdown((*id_vector)[i]) + " over to new device (set)", P_WARN);
				}
			}catch(...){
				print("couldn't shift id " + id_breakdown((*id_vector)[i]) + " over to new device (get)", P_WARN);
			}
		}
	}
}

void id_tier::operation::del_id_from_state(
	std::vector<id_t_> state_id,
	std::vector<id_t_> id){
	for(uint64_t i = 0;i < state_id.size();i++){
		try{
			id_tier_state_t *tier_state_ptr =
				PTR_DATA(state_id[i],
					 id_tier_state_t);
			PRINT_IF_NULL(tier_state_ptr, P_ERR);
			id_tier_medium_t medium =
				id_tier::get_medium(
					tier_state_ptr->get_medium());
			for(uint64_t c = 0;c < id.size();c++){
				try{
					medium.del_id(
						tier_state_ptr->id.get_id(),
						id[c]);
				}catch(...){}
			}
		}catch(...){}
	}
}

std::vector<std::vector<uint8_t> > id_tier::operation::get_data_from_state(
	std::vector<id_t_> state_id,
	std::vector<id_t_> id_vector){
	std::vector<std::vector<uint8_t> > retval;
	for(uint64_t c = 0;c < id_vector.size();c++){
		try{
			for(uint64_t i = 0;i < state_id.size();i++){
				try{
					id_tier_state_t *tier_state_ptr =
						PTR_DATA(state_id[i],
							 id_tier_state_t);
					CONTINUE_IF_NULL(tier_state_ptr, P_WARN);
					std::vector<id_t_> state_cache =
						id_tier::lookup::ids::from_state(
							tier_state_ptr);
					id_tier_medium_t medium =
						id_tier::get_medium(
							tier_state_ptr->get_medium());
					std::vector<uint8_t> id_exp =
						medium.get_id(
							tier_state_ptr->id.get_id(),
							id_vector[c]);
					if(id_exp.size() > 0){
						retval.push_back(
							id_exp);
						break;
					}
				}catch(...){}
			}
		}catch(...){}
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
				id_tier_medium_t medium =
					id_tier::get_medium(
						tier_state_ptr->get_medium());
				const extra_t_ extra_byte =
					tier_state_ptr->get_allowed_extra().at(0);
				if(extra_vector[extra_byte].size() == 0){
					extra_vector[extra_byte] =
						id_api::raw::force_to_extra(
							data_vector[i],
							extra_byte);
				}
				medium.add_data(
					tier_state_ptr->id.get_id(),
					extra_vector[extra_byte]);
			}catch(...){
				print("couldn't insert " + id_breakdown(id_api::raw::fetch_id(data_vector[i])) + " into " +
				      id_breakdown(state_id[c]), P_WARN);
			}
		}
	}
}

id_tier_state_t::id_tier_state_t() : id(this, TYPE_ID_TIER_STATE_T){
}

id_tier_state_t::~id_tier_state_t(){
}

bool id_tier_state_t::is_allowed_extra(extra_t_ extra_, id_t_ id_){
	return std::find(
		allowed_extra.begin(),
		allowed_extra.end(),
		extra_) != allowed_extra.end() ||
		std::find(
			encrypt_blacklist.begin(),
			encrypt_blacklist.end(),
			get_id_type(id_)) != encrypt_blacklist.end();
	// anything that shouldn't be encrypted overrides the is_allowed_extra,
	// but should probably enforce compression
}

void id_tier_state_t::del_id_buffer(id_t_ id_){
	for(uint64_t i = 0;i < id_buffer.size();i++){
		if(id_buffer[i].first == id_){
			id_buffer.erase(
				id_buffer.begin()+i);
			break;
		}
	}
}


// creation of tier states happens in the main init()

static void id_tier_init_disk(){
	id_tier_medium_t disk_medium_ptr =
		id_tier::get_medium(
			ID_TIER_MEDIUM_DISK);
	id_tier_state_t *tier_state_ptr =
		PTR_DATA(disk_medium_ptr.init_state(), id_tier_state_t);
	tier_state_ptr->add_allowed_extra(
		ID_EXTRA_ENCRYPT & ID_EXTRA_COMPRESS);
	tier_state_ptr->set_medium(
		ID_TIER_MEDIUM_DISK);
	tier_state_ptr->set_tier_major(
		ID_TIER_MAJOR_DISK);
	tier_state_ptr->set_tier_minor(
		0);

	id_tier_disk_state_t *disk_state_ptr =
		reinterpret_cast<id_tier_disk_state_t*>(
			tier_state_ptr->get_payload());
	ASSERT(disk_state_ptr != nullptr, P_ERR);

#ifdef __linux
	struct passwd *pw = getpwuid(getuid());
	const std::string home_path =
		pw->pw_dir;
#else
	const std::string home_path =
		"";
#endif
	const std::string full_path =
		home_path + "/.BasicTV/";
	P_V_S(full_path, P_VAR);
	disk_state_ptr->path =
		convert::string::to_bytes(
			full_path);
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
		tier_state_ptr->add_allowed_extra(
			std::get<1>(cache_data[i]));
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
	id_tier_init_cache();
	// disk seems to be working fine, but tier shfiting code doesn't
	// debug it with cache tiers first, then enable disk
	// id_tier_init_disk();
}

#define COPY_UP 1
#define COPY_DOWN 2

// GC should be in another function
// #define MOVE_DOWN 3

// ID (first) is copied/moved (fourth) from state A (lower tier, second) to
// state B (higher tier, third). If the movement type (1-3) doesn't match the
// A/B, there's an internal error

static std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > tier_move_logic(
	id_t_ first_id,
	id_t_ second_id){
	std::vector<std::tuple<id_t_, id_t_, id_t_, uint8_t> > retval;
	
	id_tier_state_t *first_state_ptr =
		PTR_DATA(first_id,
			 id_tier_state_t);
	ASSERT(first_state_ptr != nullptr, P_ERR);
	id_tier_state_t *second_state_ptr =
		PTR_DATA(second_id,
			 id_tier_state_t);
	ASSERT(second_state_ptr != nullptr, P_ERR);

	// We don't bother with cache right now, since that's more hinting
	// and pre-loading (which isn't an optimization until MT gets going)
	if(first_state_ptr->get_tier_major() == second_state_ptr->get_tier_major()){
		return retval;
	}

	if(first_state_ptr->get_tier_major() > second_state_ptr->get_tier_major()){
		std::swap(first_state_ptr, second_state_ptr);
		std::swap(first_id, second_id);
	}

	const std::vector<std::pair<id_t_, mod_inc_t_> > first_id_buffer =
		id_tier::lookup::id_mod_inc::from_state(
			first_state_ptr);
	const std::vector<std::pair<id_t_, mod_inc_t_> > second_id_buffer =
		id_tier::lookup::id_mod_inc::from_state(
			second_state_ptr);
	for(uint64_t a = 0;a < first_id_buffer.size();a++){
		bool found = false;
		const id_t_ a_id = std::get<0>(first_id_buffer[a]);
		if(get_id_type(a_id) == TYPE_ID_TIER_STATE_T){
			continue;
		}
		for(uint64_t b = 0;b < second_id_buffer.size();b++){
			if(std::get<0>(second_id_buffer[b]) == a_id){
				const mod_inc_t_ a_mod_inc =
					std::get<1>(first_id_buffer[a]);
				const mod_inc_t_ b_mod_inc =
					std::get<1>(second_id_buffer[b]);
				found = true;
				if(a_mod_inc > b_mod_inc){
					retval.push_back(
						std::make_tuple(
							a_id,
							first_id,
							second_id,
							COPY_UP));
				}else if(a_mod_inc < b_mod_inc){
					retval.push_back(
						std::make_tuple(
							a_id,
							first_id,
							second_id,
							COPY_DOWN));
				}
				break;
			}
		}
		if(found == false){ // push it down if it isn't already there
			retval.push_back(
				std::make_tuple(
					a_id,
					first_id,
					second_id,
					COPY_UP));
		}
	}
	return retval;
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
				id_tier::operation::shift_data_to_state(
					from_id,
					to_id,
					&shift_operation);
				if(shift_operation.size() == 1){
					// completely normal behavior, since we
					// let shift_data_to_state handle a lot
					// of the rules
					// print("couldn't shift data over", P_SPAM);
				}else{
					print("shifted data from tier " + id_breakdown(from_id) + " to " + id_breakdown(to_id), P_SPAM);
				}
			}
		}
	}
}

#undef COPY_UP
#undef COPY_DOWN
#undef MOVE_DOWN

void id_tier_close(){
	id_tier_loop();
}
