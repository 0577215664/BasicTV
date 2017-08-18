#include "id_tier_memory.h"
#include "id_tier_memory_special.h"
#include "id_tier_memory_helper.h"
#include "../id_tier.h"

#include "../../id.h"
#include "../../id_api.h"

// new/delete
#include "../../../tv/tv_dev_audio.h"
#include "../../../tv/tv_dev_video.h"
#include "../../../tv/tv_frame_audio.h"
#include "../../../tv/tv_frame_video.h"
#include "../../../tv/tv_frame_caption.h"
#include "../../../tv/tv_channel.h"
#include "../../../tv/tv_window.h"
#include "../../../tv/tv_item.h"
#include "../../../net/proto/inbound/net_proto_inbound_data.h"
#include "../../../net/proto/outbound/net_proto_outbound_data.h"
#include "../../../net/proto/net_proto_con_req.h"
#include "../../../net/proto/net_proto_peer.h"
#include "../../../net/proto/net_proto_con_req.h"
#include "../../../net/proto/net_proto.h"
#include "../../../net/proto/net_proto_socket.h"
#include "../../../net/net_cache.h"
#include "../../../net/net.h"
#include "../../../input/input.h"
#include "../../../input/input_ir.h"
#include "../../../settings.h"
#include "../../../system.h"
#include "../../../cryptocurrency.h"
#include "../../../compress/compress.h"
#include "../../../encrypt/encrypt.h"

// not static for id_tier_memory_helper.cpp only, not declared extern in header
std::array<std::vector<data_id_t*>, TYPE_COUNT+1> id_vector; // +1 for NOTYPE
std::vector<std::pair<id_t_, mod_inc_t_> > id_buffer;

/*
  This is guaranteed to be called for every data_id_t created when
  deployed, so we can safely call both id and id_buffer with proper
  try-catch blocks and keep them coupled together.
 */

#define GET_TIER_STATE()					\
	id_tier_state_t *tier_state_ptr = nullptr;		\
	if(true){						\
		data_id_t *id_ptr =				\
			mem_helper::lookup::id(			\
				state_id);			\
		ASSERT(id_ptr != nullptr, P_ERR);		\
		tier_state_ptr =				\
			(id_tier_state_t*)id_ptr->get_ptr();	\
	}

ID_TIER_INIT_STATE(mem){
	id_tier_state_t *tier_state_ptr =
		new id_tier_state_t;
	tier_state_ptr->set_medium(
		ID_TIER_MEDIUM_MEM);
	tier_state_ptr->set_payload(
		nullptr); // id_tier_memory_t isn't used
	id_tier_mem_update_state_cache(
		tier_state_ptr);
	return tier_state_ptr->id.get_id();
}

ID_TIER_DEL_STATE(mem){
	GET_TIER_STATE();
	ASSERT(tier_state_ptr != nullptr, P_ERR);
	// should probably do something with this?
}

#define CHECK_TYPE(a)					\
	if(convert::type::from(type) == #a){		\
		loaded = true;				\
		print("importing data", P_SPAM);	\
		a* tmp = nullptr;			\
		try{					\
			tmp = new a;			\
			tmp->id.import_data(data);	\
		}catch(...){				\
			if(tmp != nullptr){		\
				delete tmp;		\
				tmp = nullptr;		\
			}				\
		}					\
	}						\

// TODO: make this search for valid data first
ID_TIER_ADD_DATA(mem){	
	GET_TIER_STATE();
	ASSERT(tier_state_ptr->is_allowed_extra(
		       id_api::raw::fetch_extra(
			       data),
		       id_api::raw::fetch_id(
			       data)), P_ERR);
	id_t_ id = ID_BLANK_ID;
	type_t_ type = 0;
	try{
		id = id_api::raw::fetch_id(data);
		type = get_id_type(id_api::raw::fetch_id(data));
		P_V_S(convert::array::id::to_hex(id), P_VAR);
		P_V_S(convert::type::from(type), P_VAR);
	}catch(std::exception &e){
		print("can't import id and type from raw data", P_ERR);
		throw e;
	}
	// initializer of data_id_t calls mem_add_id automatically
	bool loaded = false;
	CHECK_TYPE(tv_frame_video_t);
	CHECK_TYPE(tv_frame_audio_t);
	CHECK_TYPE(tv_window_t);
	CHECK_TYPE(tv_channel_t);
	CHECK_TYPE(tv_item_t);
	
	// net (proto and standard)
	CHECK_TYPE(net_socket_t);
	CHECK_TYPE(net_proto_peer_t);
	CHECK_TYPE(net_proto_socket_t);
	CHECK_TYPE(net_proto_type_request_t);
	CHECK_TYPE(net_proto_id_request_t);
	CHECK_TYPE(net_proto_linked_list_request_t);
	CHECK_TYPE(net_proto_con_req_t);

	// IR
	CHECK_TYPE(ir_remote_t);

	// input
	CHECK_TYPE(input_dev_standard_t);

	// cryptography
	CHECK_TYPE(encrypt_priv_key_t);
	CHECK_TYPE(encrypt_pub_key_t);

       	CHECK_TYPE(wallet_set_t);

	// Math
	CHECK_TYPE(math_number_set_t);
	CHECK_TYPE(net_interface_ip_address_t);
	if(loaded == false){
		print("can't add a type we don't have a macro creator for: " + convert::type::from(type), P_ERR);
	}
}


#undef CHECK_TYPE

// I remember MSVC++ switching ifs like this one, that'd be a problem
#define DELETE_TYPE_2(a) if(deleted == false && ptr->get_type() == #a){print("deleting " + (std::string)(#a) + id_breakdown(ptr->get_id()), P_DEBUG);delete (a*)ptr->get_ptr();ptr = nullptr;deleted = true;}


ID_TIER_DEL_ID(mem){
	GET_TIER_STATE();
	// Again, data_id_t destructors call mem_del_id
	data_id_t *ptr =
		mem_helper::lookup::id(id);
	if(ptr == nullptr){
		print("already destroying a destroyed type " + id_breakdown(id), P_SPAM);
		return;
	}
	// TODO: convert this over into a jump table with type_t_
	bool deleted = false;
	// TV subsystem
	DELETE_TYPE_2(tv_frame_video_t);
	DELETE_TYPE_2(tv_frame_audio_t);
	DELETE_TYPE_2(tv_frame_caption_t);
	DELETE_TYPE_2(tv_window_t);
	DELETE_TYPE_2(tv_channel_t);
	DELETE_TYPE_2(tv_item_t);
	
	// net (proto and standard)
	DELETE_TYPE_2(net_socket_t);
	DELETE_TYPE_2(net_proto_peer_t);
	DELETE_TYPE_2(net_proto_socket_t);
	DELETE_TYPE_2(net_proto_type_request_t);
	DELETE_TYPE_2(net_proto_id_request_t);
	DELETE_TYPE_2(net_proto_linked_list_request_t);
	DELETE_TYPE_2(net_proto_con_req_t);
	DELETE_TYPE_2(net_cache_t);

	// IR
	DELETE_TYPE_2(ir_remote_t);

	// input
	DELETE_TYPE_2(input_dev_standard_t);

	// cryptography
	DELETE_TYPE_2(encrypt_priv_key_t);
	DELETE_TYPE_2(encrypt_pub_key_t);

       	DELETE_TYPE_2(wallet_set_t);

	// Math
	DELETE_TYPE_2(math_number_set_t);
	DELETE_TYPE_2(net_interface_ip_address_t);

	// can't destroy an id_tier_state_t (at least not tier 0), since
	// that's pretty essential to core ID function
	// // Tier
	// DELETE_TYPE_2(id_tier_state_t);
	
	if(deleted == false &&
	   ptr->get_type_byte() != TYPE_ID_TIER_STATE_T){
		auto pos =
			std::find(
				id_vector[get_id_type(id)].begin(),
				id_vector[get_id_type(id)].end(),
				ptr);
		if(pos != id_vector[get_id_type(id)].end()){
			id_vector[get_id_type(id)].erase(
				pos);
		}
		print("No proper type was found for clean deleting" + ptr->get_type(), P_WARN);
	}
	id_tier_mem_regen_state_cache();
	id_tier_mem_update_state_cache(
		tier_state_ptr);
}

#undef DELETE_TYPE_2

ID_TIER_GET_ID(mem){
	ASSERT(state_id != ID_BLANK_ID, P_ERR);
	if(get_id_hash(id) !=
	   get_id_hash(production_priv_key_id)){
		// Technically we CAN, but anything we have in memory should
		// be stored encrypted on the cache
		P_V_S(id_breakdown(id), P_VAR);
		P_V_S(id_breakdown(production_priv_key_id), P_VAR);
		print("cannot/shouldn't supply data I do not own directly from memory, refer to tier 1 or higher", P_UNABLE);
	}
	for(uint64_t i = 0;i < id_vector[get_id_type(id)].size();i++){
		if(id_vector[get_id_type(id)][i]->get_id() == id){
			return id_vector[get_id_type(id)][i]->export_data(
				0);
		}
	}
	return std::vector<uint8_t>({});
}

ID_TIER_UPDATE_CACHE(mem){
	ASSERT(state_id != ID_BLANK_ID, P_ERR);
	return;
}

/*
  This isn't beholden to calling rules that id_tier_mem_* does, since
  this is globally callable anywhere in the program, whereas the upper
  cannot call id_tier (out of fear of an infinite loop)
 */

#pragma message("disabled all non-memory reading for now, undo the following comment block")

static std::vector<id_t_> lookup_vector;

data_id_t *id_tier::mem::get_id_ptr(
	id_t_ id,
	type_t_ type,
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector){
	if(id == ID_BLANK_ID){
		return nullptr;
	}
	ASSERT(get_id_type(id) == type ||
	       type == 0, P_ERR); // type == 0 is with ""
	if(std::find(
		   lookup_vector.begin(),
		   lookup_vector.end(),
		   id) != lookup_vector.end()){
		return nullptr;
	}
	data_id_t *retval =
		nullptr;
	lookup_vector.push_back(id);
	try{
		retval =
			mem_helper::lookup::id(id);
		if(retval != nullptr){
			lookup_vector.erase(lookup_vector.end()-1);
			return retval;
		}
		std::vector<id_tier_state_t*> tier_state_vector =
			mem_helper::lookup::tier_state(
				tier_vector);
		id_tier_state_t *mem_state_ptr =
			mem_helper::lookup::tier_state(
				std::vector<std::pair<uint8_t, uint8_t> >(
					{std::make_pair(0, 0)})).at(0);
		ASSERT(mem_state_ptr != nullptr, P_ERR);
		std::vector<id_t_> shift_payload({id});
		for(uint64_t i = 0;i < tier_state_vector.size() && shift_payload.size() != 0;i++){
			if(tier_state_vector[i]->get_tier_major() == 0){
				continue;
			}
			id_tier::operation::shift_data_to_state(
				tier_state_vector[i],
				mem_state_ptr,
				&shift_payload);
		}
		retval = mem_helper::lookup::id(id);
		if(retval == nullptr &&
		   tier_vector == all_tiers &&
		   production_priv_key_id != ID_BLANK_ID){
			// TODO: direct comparison to all_tiers doesn't consider order
			if(std::find(
				   net_proto_request_blacklist.begin(),
				   net_proto_request_blacklist.end(),
				   get_id_type(id)) == net_proto_request_blacklist.end()){
				net_proto::request::add_id(
					id);
			}else{
				print("not listing net_proto_request_blacklist type in ID request vector", P_SPAM);
			}
		}
	}catch(...){
	}
	lookup_vector.erase(lookup_vector.end()-1);
	return retval;
}

void *id_tier::mem::get_ptr(
	id_t_ id,
	std::string type,
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector){
	data_id_t *id_ptr =
		get_id_ptr(
			id,
			convert::type::to(type),
		        tier_vector);
	if(id_ptr == nullptr){
		return nullptr;
	}
	return id_ptr->get_ptr();
}


data_id_t *id_tier::mem::get_id_ptr(
	id_t_ id,
	std::string type,
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector){
	return id_tier::mem::get_id_ptr(
		id,
		(type == "") ? 0 : convert::type::to(type),
	        tier_vector);
}
