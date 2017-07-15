#include "id_tier.h"
#include "id_tier_memory.h"

#include "../id.h"
#include "../id_api.h"

// new/delete
#include "../../tv/tv_dev_audio.h"
#include "../../tv/tv_dev_video.h"
#include "../../tv/tv_frame_audio.h"
#include "../../tv/tv_frame_video.h"
#include "../../tv/tv_frame_caption.h"
#include "../../tv/tv_channel.h"
#include "../../tv/tv_window.h"
#include "../../tv/tv_menu.h"
#include "../../tv/tv_item.h"
#include "../../net/proto/inbound/net_proto_inbound_data.h"
#include "../../net/proto/outbound/net_proto_outbound_data.h"
#include "../../net/proto/net_proto_con_req.h"
#include "../../net/proto/net_proto_peer.h"
#include "../../net/proto/net_proto_con_req.h"
#include "../../net/proto/net_proto.h"
#include "../../net/proto/net_proto_socket.h"
#include "../../net/net_cache.h"
#include "../../net/net.h"
#include "../../input/input.h"
#include "../../input/input_ir.h"
#include "../../settings.h"
#include "../../system.h"
#include "../../cryptocurrency.h"
#include "../../compress/compress.h"
#include "../../encrypt/encrypt.h"

static std::vector<data_id_t*> id_vector;
static std::vector<std::pair<id_t_, mod_inc_t_> > id_buffer;

/*
  This is guaranteed to be called for every data_id_t created when
  deployed, so we can safely call both id and id_buffer with proper
  try-catch blocks and keep them coupled together.
 */


static void id_tier_mem_update_state_cache(
	id_tier_state_t *tier_state_ptr){
	// Probably could use some pointer magic
	ASSERT(tier_state_ptr != nullptr, P_ERR);
	tier_state_ptr->set_id_buffer(
		id_buffer);
}

// special status, called by ID constructors ONLY

void mem_add_id(data_id_t *ptr){
	id_vector.push_back(ptr);
	id_buffer.push_back(
		std::make_pair(
			ptr->get_id(),
			ptr->get_mod_inc()));
	id_tier_mem_update_state_cache(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(0, 0),
			 id_tier_state_t));
}

void mem_del_id(data_id_t *ptr){
	auto id_pos =
		std::find(
			id_vector.begin(),
			id_vector.end(),
			ptr);
	if(id_pos != id_vector.end()){
		id_vector.erase(
			id_pos);
	}
	for(uint64_t i = 0;i < id_buffer.size();i++){
		if(id_buffer[i].first == ptr->get_id()){
			id_buffer.erase(
				id_buffer.begin()+i);
			break;
		}
	}
	id_tier_mem_update_state_cache(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(0, 0),
			 id_tier_state_t));
}


static data_id_t *mem_lookup(id_t_ id){
	for(uint64_t i = 0;i < id_vector.size();i++){
		try{
			const id_t_ list_id =
				id_vector[i]->get_id(true);
			const bool matching_nonhash =
				get_id_uuid(list_id) == get_id_uuid(id) &&
				get_id_type(list_id) == get_id_type(id);
			const bool matching_hash =
				get_id_hash(list_id) == get_id_hash(id);
			const bool list_hash_blank =
				get_id_hash(list_id) == blank_hash;
			const bool param_hash_blank =
				get_id_hash(id) == blank_hash;
			if(matching_nonhash){
				if((list_hash_blank || param_hash_blank) ||
				   matching_hash){
					return id_vector[i];
				}
			}
		}catch(...){}
	}
	return nullptr;
}

id_tier_state_t *mem_tier_state_lookup(
	uint8_t major,
	uint8_t minor){
	for(uint64_t i = 0;i < id_vector.size();i++){
		if(unlikely(id_vector[i]->get_type_byte() == TYPE_ID_TIER_STATE_T)){
			id_tier_state_t *tier_state_ptr =
				(id_tier_state_t*)id_vector[i]->get_ptr();
			ASSERT(tier_state_ptr != nullptr, P_ERR);
			if(tier_state_ptr->get_tier_major() == major &&
			   tier_state_ptr->get_tier_minor() == minor){
				return tier_state_ptr;
			}
		}
	}
	return nullptr;
}

static void id_tier_mem_add_ptr(id_t_ state_id, data_id_t *ptr){
	ASSERT(ptr != nullptr, P_ERR);
	mem_add_id(ptr);
}

#define GET_TIER_STATE()					\
	id_tier_state_t *tier_state_ptr = nullptr;		\
	if(true){						\
		data_id_t *id_ptr =				\
			id_tier::mem::get_id_ptr(		\
				state_id,			\
				TYPE_ID_TIER_STATE_T,		\
				0,				\
				0);				\
		ASSERT(id_ptr != nullptr, P_ERR);		\
		tier_state_ptr =				\
			(id_tier_state_t*)id_ptr->get_ptr();	\
	}

ID_TIER_INIT_STATE(mem){
	id_tier_state_t *tier_state_ptr =
		new id_tier_state_t;
	id_tier_mem_update_state_cache(
		tier_state_ptr);
	return tier_state_ptr->id.get_id();
}

ID_TIER_DEL_STATE(mem){
	GET_TIER_STATE();
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
	CHECK_TYPE(tv_channel_t);
	CHECK_TYPE(tv_frame_audio_t);
	CHECK_TYPE(tv_frame_video_t);
	CHECK_TYPE(net_proto_peer_t);
	CHECK_TYPE(net_proto_id_request_t);
	CHECK_TYPE(net_proto_type_request_t);
	CHECK_TYPE(net_proto_linked_list_request_t);
	CHECK_TYPE(net_proto_con_req_t);
	CHECK_TYPE(encrypt_pub_key_t);
	CHECK_TYPE(encrypt_priv_key_t);
	CHECK_TYPE(wallet_set_t);
	CHECK_TYPE(net_socket_t);
	CHECK_TYPE(math_number_set_t);
	CHECK_TYPE(net_interface_ip_address_t);
	if(loaded == false){
		print("can't add a type we don't have a macro creator for: " + convert::type::from(type), P_ERR);
	}
}


#undef CHECK_TYPE

#define DELETE_TYPE_2(a) if(ptr->get_type() == #a){print("deleting " + (std::string)(#a) + id_breakdown(ptr->get_id()), P_DEBUG);delete (a*)ptr->get_ptr();ptr = nullptr;deleted = true;}


ID_TIER_DEL_ID(mem){
	GET_TIER_STATE();
	// Again, data_id_t destructors call mem_del_id
	data_id_t *ptr =
		PTR_ID_PRE(id, );
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
	DELETE_TYPE_2(tv_menu_entry_t);
	DELETE_TYPE_2(tv_menu_t);
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
	
	if(deleted == false){
		print("No proper type was found for clean deleting" + ptr->get_type(), P_ERR);
	}
}

#undef DELETE_TYPE_2

ID_TIER_GET_ID(mem){
	if(get_id_hash(id) !=
	   get_id_hash(production_priv_key_id)){
		// Technically we CAN, but anything we have in memory should
		// be stored encrypted on the cache
		print("cannot/shouldn't supply data I do not own directly from memory, refer to tier 1 or higher", P_ERR);
	}
	for(uint64_t i = 0;i < id_vector.size();i++){
		if(id_vector[i]->get_id() == id){
			return id_vector[i]->export_data(
				0, 0, 0, 0, 0);
		}
	}
	return std::vector<uint8_t>({});
}

ID_TIER_GET_ID_MOD_INC(mem){
	for(uint64_t i = 0;i < id_buffer.size();i++){
		if(id_buffer[i].first == id){
			return id_buffer[i].second;
		}
	}
	return 0;
}

/*
  The local version and the id_tier_state_t version are identical, since they
  can only change through this function
 */

ID_TIER_GET_ID_BUFFER(mem){
	return id_buffer;
}

ID_TIER_UPDATE_ID_BUFFER(mem){
	// Buffer is guaranteed to be up-to-date
}

/*
  This isn't beholden to calling rules that id_tier_mem_* does, since
  this is globally callable anywhere in the program, whereas the upper
  cannot call id_tier (out of fear of an infinite loop)
 */

#pragma warning("disabled all non-memory reading for now, undo the following comment block")

static std::vector<id_t_> lookup_vector;

data_id_t *id_tier::mem::get_id_ptr(
	id_t_ id,
	type_t_ type,
	uint8_t high_major,
	uint8_t high_minor){
	ASSERT(high_minor == 0, P_ERR); // actually write this in later
	if(id == ID_BLANK_ID){
		return nullptr;
	}
	ASSERT(get_id_type(id) == type, P_ERR);
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
			mem_lookup(id);
	}catch(...){
	}
	lookup_vector.erase(lookup_vector.end()-1);
	// if(retval != nullptr){
	// 	std::vector<std::tuple<id_t_, uint8_t, uint8_t> > tier_data =
	// 		id_tier::operation::valid_state_with_id(
	// 			id);
	// 	const id_t_ mem_state_id =
	// 		id_tier::state_tier::only_state_of_tier(
	// 			0, 0);
	// 	for(uint64_t i = 0;i < tier_data.size();i++){
	// 		const uint8_t tier_major =
	// 			std::get<1>(tier_data[i]);
	// 		const uint8_t tier_minor =
	// 			std::get<2>(tier_data[i]);
	// 		const bool lower_major =
	// 			tier_major <= high_major;
	// 		const bool lower_minor =
	// 			tier_minor <= high_minor;
	// 		if(likely((lower_major && lower_minor))){
	// 			try{
	// 				id_tier::operation::shift_data_to_state(
	// 					std::get<0>(tier_data[i]),
	// 					mem_state_id,
	// 					{id});
	// 				if((retval = mem_lookup(id)) != nullptr){
	// 					break;
	// 				}
	// 			}catch(...){
	// 				P_V(high_major, P_WARN);
	// 				P_V(high_minor, P_WARN);
	// 				print("reading error", P_ERR);
	// 			}
	// 		}
	// 	}
	// }
	return retval;
}

void *id_tier::mem::get_ptr(
	id_t_ id,
	std::string type,
	uint8_t high_major,
	uint8_t high_minor){
	data_id_t *id_ptr =
		get_id_ptr(id,
		       convert::type::to(type),
		       high_major,
		       high_minor);
	if(id_ptr == nullptr){
		return nullptr;
	}
	return id_ptr->get_ptr();
}


data_id_t *id_tier::mem::get_id_ptr(
	id_t_ id,
	std::string type,
	uint8_t high_major,
	uint8_t high_minor){
	return id_tier::mem::get_id_ptr(
		id,
		convert::type::to(type),
		high_major,
		high_minor);
}
