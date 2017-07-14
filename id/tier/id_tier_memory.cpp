#include "id_tier.h"
#include "id_tier_memory.h"

#include "../id.h"
#include "../id_api.h"
#include "../id_disk.h"

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

// special status, called by ID constructors ONLY

void mem_add_id(data_id_t *ptr){
	id_vector.push_back(ptr);
	id_buffer.push_back(
		std::make_pair(
			ptr->get_id(),
			ptr->get_mod_inc()));
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
}

static data_id_t *mem_lookup(id_t_ id){
	for(uint64_t i = 0;i < id_vector.size();i++){
		if(unlikely(id_vector[i]->get_id() == id)){
			return id_vector[i];
		}
	}
	return nullptr;
}

static void id_tier_mem_add_ptr(id_t_ state_id, data_id_t *ptr){
	ASSERT(ptr != nullptr, P_ERR);
	mem_add_id(ptr);
}

/*
  TODO: it probably makes way more sense to just lookup the ID in a second,
  safer, and more restrictive function so we don't have Catch-22s, ID_BLANK_ID
  is guaranteed to be invalid at that point
 */

ID_TIER_INIT_STATE(mem){
}

ID_TIER_DEL_STATE(mem){
}

#define CHECK_TYPE(a)					\
	if(convert::type::from(type) == #a){		\
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
	print("type " + convert::type::from(type) + " needs a loader", P_CRIT);

}

#undef CHECK_TYPE

#define DELETE_TYPE_2(a) if(ptr->get_type() == #a){print("deleting " + (std::string)(#a) + id_breakdown(ptr->get_id()), P_DEBUG);delete (a*)ptr->get_ptr();ptr = nullptr;return;}


ID_TIER_DEL_ID(mem){
	// Again, data_id_t destructors call mem_del_id
	data_id_t *ptr =
		PTR_ID_PRE(id, );
	if(ptr == nullptr){
		print("already destroying a destroyed type " + id_breakdown(id), P_SPAM);
		return;
	}
	// TODO: convert this over into a jump table with type_t_
	
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

	DELETE_TYPE_2(id_disk_index_t);
	
       	DELETE_TYPE_2(wallet_set_t);

	// Math
	DELETE_TYPE_2(math_number_set_t);

	DELETE_TYPE_2(net_interface_ip_address_t);
	
	print("No proper type was found for clean deleting, cutting losses "
	      "and delisting it, memory leak occuring: " + ptr->get_type(), P_ERR);
	
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
	data_id_t *retval =
		mem_lookup(id);
	if(retval != nullptr){
		std::vector<std::tuple<id_t_, uint8_t, uint8_t> > tier_data =
			id_tier::operation::valid_state_with_id(
				id);
		const id_t_ mem_state_id =
			id_tier::only_state_of_tier(
				0, 0);
		for(uint64_t i = 0;i < tier_data.size();i++){
			const uint8_t tier_major =
				std::get<1>(tier_data[i]);
			const uint8_t tier_minor =
				std::get<2>(tier_data[i]);
			const bool lower_major =
				tier_major <= high_major;
			const bool lower_minor =
				tier_minor <= high_minor;
			if(likely((lower_major && lower_minor))){
				try{
					id_tier::operation::shift_data_to_state(
						std::get<0>(tier_data[i]),
						mem_state_id,
						{id});
					if((retval = mem_lookup(id)) != nullptr){
						break;
					}
				}catch(...){
					P_V(high_major, P_WARN);
					P_V(high_minor, P_WARN);
					print("reading error", P_ERR);
				}
			}
		}
	}
	return retval;
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
