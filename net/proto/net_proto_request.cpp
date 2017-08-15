#include "net_proto.h"
#include "net_proto_request.h"
#include "net_proto_socket.h"
#include "../../id/id_api.h"

#include "../../settings.h"

/*
  This file is responsible for creation and destruction of all requests

  Broadcasting of them is handled inside of outbound/net_proto_outbound_data.cpp
 */

const data_id_transport_rules_t request_ruleset(
	all_mem_cache,
	all_intermediaries);

std::vector<uint8_t> net_proto_request_blacklist =
{
	TYPE_NET_PROTO_ID_REQUEST_T,
	TYPE_NET_PROTO_TYPE_REQUEST_T,
	TYPE_NET_PROTO_LINKED_LIST_REQUEST_T // net_proto_con_req_t is pull, excluded
};

// routine request jargon
static uint64_t last_request_fast_time_micro_s = 0;
static uint64_t last_request_slow_time_micro_s = 0;

static std::vector<uint8_t> routine_request_fast_vector = {
	TYPE_ENCRYPT_PUB_KEY_T,
	TYPE_NET_PROTO_PEER_T,
	TYPE_TV_ITEM_T,
	TYPE_TV_CHANNEL_T,
	TYPE_NET_PROTO_CON_REQ_T
};
static std::vector<uint8_t> routine_request_slow_vector = {
	TYPE_WALLET_SET_T
};

// standard request jargon
// ID to request, number of times it has been requested
static std::vector<std::pair<id_t_, uint64_t> > id_request_buffer;
static std::vector<std::pair<id_t_, int64_t> > linked_list_request_buffer;

net_proto_request_bare_t::net_proto_request_bare_t(){}

net_proto_request_bare_t::~net_proto_request_bare_t(){}

void net_proto_request_bare_t::list_bare_virtual_data(data_id_t *id_){
	id_->add_data_id(&origin_peer_id, 1, request_ruleset);
	id_->add_data_id(&destination_peer_id, 1, request_ruleset);
	id_->add_data_raw((uint8_t*)&request_time, sizeof(request_time), request_ruleset);
	id_->add_data_raw((uint8_t*)&ttl_micro_s, sizeof(ttl_micro_s), request_ruleset);
	id = id_;
}

id_t_ net_proto_request_bare_t::get_origin_peer_id(){
	return origin_peer_id;
}

void net_proto_request_bare_t::set_origin_peer_id(id_t_ origin_peer_id_){
	origin_peer_id = origin_peer_id_;
}

id_t_ net_proto_request_bare_t::get_destination_peer_id(){
	return destination_peer_id;
}
void net_proto_request_bare_t::set_destination_peer_id(id_t_ destination_peer_id_){
	destination_peer_id = destination_peer_id_;
}

// When it was last modified
void net_proto_request_bare_t::update_request_time(){
	request_time = get_time_microseconds();
}

// When it was last broadcast
void net_proto_request_bare_t::update_broadcast_time_micro_s(){
	broadcast_time_micro_s = get_time_microseconds();
}

net_proto_request_set_t::net_proto_request_set_t(){}

net_proto_request_set_t::~net_proto_request_set_t(){}

void net_proto_request_set_t::list_set_virtual_data(data_id_t *id_){
	// id->add_data_one_byte_vector(&ids, ~0);
	id_->add_data_one_byte_vector(&ids, ~0, request_ruleset);
	id_->add_data_eight_byte_vector(&mod_inc, 65536, request_ruleset);
}

void net_proto_request_set_t::set_ids(std::vector<id_t_> ids_){
	mod_inc.clear();
	for(uint64_t i = 0;i < ids_.size();i++){
		try{
			ASSERT_VALID_ID(ids_[i]);
			data_id_t *id_ptr =
				PTR_ID_FAST(ids_[i], );
			if(likely(id_ptr == nullptr)){
				mod_inc.push_back(0);
			}else{
				mod_inc.push_back(
					id_ptr->get_mod_inc());
			}
		}catch(...){}
	}
	ids = compact_id_set(
		ids_, true);
}

std::vector<id_t_> net_proto_request_set_t::get_ids(){
	return expand_id_set(ids);
}

std::vector<uint64_t> net_proto_request_set_t::get_mod_inc(){
	if(mod_inc.size() != ids.size()){
		print("mod_inc is a different size than ids, assuming all old", P_ERR);
		return {}; 
	}
	return mod_inc;
}

// IDs

net_proto_id_request_t::net_proto_id_request_t() : id(this, TYPE_NET_PROTO_ID_REQUEST_T){
	list_set_virtual_data(&id);
	list_bare_virtual_data(&id);
}

net_proto_id_request_t::~net_proto_id_request_t(){}

// Type

net_proto_type_request_t::net_proto_type_request_t() : id(this, TYPE_NET_PROTO_TYPE_REQUEST_T){
	list_set_virtual_data(&id);
	list_bare_virtual_data(&id);
	id.add_data_one_byte_vector(
		&type,
		~0,
		cache_ruleset);
	id.add_data_raw(
		&(hash[0]),
		sizeof(hash),
		cache_ruleset);
}	

net_proto_type_request_t::~net_proto_type_request_t(){}

// Linked list subscription

net_proto_linked_list_request_t::net_proto_linked_list_request_t() : id(this, TYPE_NET_PROTO_LINKED_LIST_REQUEST_T){
	list_bare_virtual_data(&id);
	ADD_DATA(start_id, cache_ruleset);
	ADD_DATA(start_length, cache_ruleset);
	ADD_DATA(curr_id, mem_ruleset);
	ADD_DATA(curr_length, mem_ruleset);
}

net_proto_linked_list_request_t::~net_proto_linked_list_request_t(){
}

void net_proto_linked_list_request_t::set_curr_id(id_t_ id_, uint32_t length){
	curr_id = id_;
	curr_length = length;
}

void net_proto_linked_list_request_t::increase_id(){
	if(curr_length == 0){
		return;
	}
	data_id_t *id_ =
		PTR_ID(curr_id, );
	if(id_ == nullptr){
		return;
	}
	std::pair<std::vector<id_t_>, std::vector<id_t_> > linked_list =
		id_->get_linked_list();
	if(linked_list.second.size() == 0){
		curr_id = ID_BLANK_ID;
	}else{
		curr_id = linked_list.second[0];
	}
	curr_length--;
}

id_t_ net_proto_linked_list_request_t::get_curr_id(){
	return curr_id;
}

// TODO: combine this file and routine_requests

void net_proto::request::add_fast_routine_type(std::string type){
	routine_request_fast_vector.push_back(
		convert::type::to(
			type));
}

void net_proto::request::add_slow_routine_type(std::string type){
	routine_request_slow_vector.push_back(
		convert::type::to(
			type));
}

void net_proto::request::del_fast_routine_type(std::string type){
	auto iterator =
		std::find(
			routine_request_fast_vector.begin(),
			routine_request_fast_vector.end(),
			convert::type::to(type));
	if(iterator != routine_request_fast_vector.end()){
		routine_request_fast_vector.erase(
			iterator);
	}
}

void net_proto::request::del_slow_routine_type(std::string type){
	auto iterator =
		std::find(
			routine_request_slow_vector.begin(),
			routine_request_slow_vector.end(),
			convert::type::to(type));
	if(iterator != routine_request_slow_vector.end()){
		routine_request_slow_vector.erase(
			iterator);
	}
}

void net_proto::request::add_id(id_t_ id){
	ASSERT_VALID_ID(id);
	if(id == ID_BLANK_ID){
		print("not adding a blank ID", P_SPAM);
		return;
	}
	if(PTR_ID(id, ) != nullptr){
		return;
	}
	// could probably speed this up
	for(uint64_t i = 0;i < id_request_buffer.size();i++){
		if(id == std::get<0>(id_request_buffer[i])){
			std::get<1>(id_request_buffer[i])++;
			return;
		}
	}
	/*
	  As it stands right now, if there is one ID request in existence, then
	  we just assume that it's going to the right place and that lower level
	  code can handle copying that over to other interfaces as well.
	 */
	std::vector<id_t_> id_request_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_PROTO_ID_REQUEST_T);
	for(uint64_t i = 0;i < id_request_vector.size();i++){
		net_proto_id_request_t *id_request_ptr =
			PTR_DATA(id_request_vector[i],
				 net_proto_id_request_t);
		CONTINUE_IF_NULL(id_request_ptr, P_WARN);
		std::vector<id_t_> id_request_id_vector =
			id_request_ptr->get_ids();
		if(std::find(id_request_id_vector.begin(),
			     id_request_id_vector.end(),
			     id) != id_request_vector.end()){
			// pending request exists for that ID
			return;
		}
	}
	id_request_buffer.push_back(
		std::make_pair(
			id,
			1));
}

void net_proto::request::add_id(std::vector<id_t_> id){
	for(uint64_t i = 0;i < id.size();i++){
		add_id(id[i]);
	}
}

/*
  Signed-ness is important for dictating direction
 */

void net_proto::request::add_id_linked_list(id_t_ id, int64_t length){
	for(uint64_t i = 0;i < linked_list_request_buffer.size();i++){
		if(unlikely(get_id_hash(linked_list_request_buffer[i].first) ==
			    get_id_hash(id))){
			std::pair<id_t_, int64_t> request_entry =
				std::make_pair(
					id, length);
			linked_list_request_buffer.insert(
				linked_list_request_buffer.begin()+i,
				request_entry);
			return;
		}
	}
	linked_list_request_buffer.push_back(
		std::make_pair(
			id, length));
}

void net_proto::request::del_id(id_t_ id){
	std::vector<id_t_> id_request_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_PROTO_ID_REQUEST_T);
	for(uint64_t i = 0;i < id_request_vector.size();i++){
		net_proto_id_request_t *id_request =
			PTR_DATA(id_request_vector[i],
				 net_proto_id_request_t);
		if(id_request == nullptr){
			continue;
		}
		std::vector<id_t_> id_vector =
			id_request->get_ids();
		auto id_ptr =
			std::find(
				id_vector.begin(),
				id_vector.end(),
				id);
		if(id_ptr != id_vector.end()){
			id_vector.erase(
				id_ptr);
			if(id_vector.size() == 0){
				delete id_request;
				id_request = nullptr;
			}else{
				id_request->set_ids(
					id_vector);
			}
			return;
		}
	}
	// This is defined behavior, but only in the beginning...
	print("cannot delete request for ID I didn't request", P_SPAM);
}

static void net_proto_routine_request_create(
	std::vector<type_t_> type_vector,
	uint64_t request_interval_micro_s,
	uint64_t *last_request_time_micro_s){
	const uint64_t time_micro_s =
		get_time_microseconds();
	if(time_micro_s-(*last_request_time_micro_s) > request_interval_micro_s){
		id_t_ recv_peer_id =
			net_proto::peer::random_connected_peer_id();
		if(recv_peer_id == ID_BLANK_ID){
			return;
		}
		print("creating routine request with frequency " + std::to_string(request_interval_micro_s) + "micro_s", P_SPAM);

		net_proto_type_request_t *type_request =
			new net_proto_type_request_t;
		type_request->set_type(
			type_vector);
		type_request->set_ttl_micro_s(
			request_interval_micro_s);
		type_request->set_destination_peer_id(
			recv_peer_id);
		type_request->set_origin_peer_id(
			net_proto::peer::get_self_as_peer());
		type_request->update_request_time();
		
		*last_request_time_micro_s = time_micro_s;
 	}
}

static void net_proto_routine_request_loop(){
 	net_proto_routine_request_create(
 		routine_request_fast_vector,
		settings::get_setting_unsigned_def(
			"net_proto_routine_request_fast_interval_micro_s", 15*1000*1000),
 		&last_request_fast_time_micro_s);
 	net_proto_routine_request_create(
 		routine_request_slow_vector,
		settings::get_setting_unsigned_def(
			"net_proto_routine_request_slow_interval_micro_s", 60*1000*1000),
 		&last_request_slow_time_micro_s);
}

static void net_proto_create_id_request_loop(){
	std::vector<std::pair<std::vector<std::pair<id_t_, uint64_t> >, id_t_> > id_peer_pair;
	for(uint64_t i = 0;i < id_request_buffer.size();i++){
		if(id_request_buffer[i].first == ID_BLANK_ID){
			continue;
		}
		const id_t_ preferable_peer_id =
			net_proto::peer::optimal_peer_for_id(
				id_request_buffer[i].first);
		if(preferable_peer_id == ID_BLANK_ID){
			print("preferable_peer_id is blank", P_ERR);
		}
		// This means we don't have any other peers whatsoever (probably)
		ASSERT(preferable_peer_id != net_proto::peer::get_self_as_peer(), P_WARN);
		bool wrote = false;
		for(uint64_t c = 0;c < id_peer_pair.size();c++){
			if(id_peer_pair[c].second == preferable_peer_id){
				id_peer_pair[c].first.push_back(
					id_request_buffer[i]);
				wrote = true;
				break;
			}
		}
		if(wrote == false){
			id_peer_pair.push_back(
				std::make_pair(
					std::vector<std::pair<id_t_, uint64_t> > ({
							id_request_buffer[i]}),
					preferable_peer_id));
		}
	}
	if(id_peer_pair.size() != 0){
		print("sending " + std::to_string(id_peer_pair.size()) + " requests, totalling " + std::to_string(id_request_buffer.size()) + " IDs", P_NOTE);
	}
	for(uint64_t i = 0;i < id_peer_pair.size();i++){
		std::sort(
			id_peer_pair[i].first.begin(),
			id_peer_pair[i].first.end(),
			[](const std::pair<id_t_, uint64_t> first_elem,
			   const std::pair<id_t_, uint64_t> second_elem){
				return first_elem.second > second_elem.second;
			});
	}
	id_request_buffer.clear();
	const id_t_ self_peer_id =
		net_proto::peer::get_self_as_peer();
	for(uint64_t i = 0;i < id_peer_pair.size();i++){
		try{
			net_proto_id_request_t *id_request_ptr =
				new net_proto_id_request_t;
			id_request_ptr->set_ttl_micro_s(
				30*1000*1000); // TODO: should make this a setting
			std::vector<id_t_> id_request_id_set;
			for(uint64_t c = 0;c < id_peer_pair[i].first.size();c++){
				id_request_id_set.push_back(
					id_peer_pair[i].first[c].first);
			}
			id_request_ptr->set_ids(
				id_request_id_set);
			id_request_ptr->set_origin_peer_id(
				self_peer_id);
			id_request_ptr->set_destination_peer_id(
				id_peer_pair[i].second);
			id_request_ptr->update_request_time();
		}catch(...){
			print("failed to create net_proto_id_request_t", P_ERR);
			// not sure how this would happen...
			id_request_buffer.insert(
				id_request_buffer.end(),
				id_peer_pair[i].first.begin(),
				id_peer_pair[i].first.end());
		}
	}
}

#pragma message("net_proto_create_linked_list_request_loop is not implemented")

static void net_proto_create_linked_list_request_loop(){
}

/*
  The only way that type requests are sent out to the network would be through
  routine requests, which is only referenced in this block, doesn't have a
  buffer associated with it, and is simpler to implement.
 */

static void net_proto_simple_request_loop(){
	net_proto_create_id_request_loop();
	//net_proto_create_linked_list_request_loop();
}


template <typename T>
bool net_proto_obsolete_request(T request, uint64_t timeout_micro_s){
	ASSERT(request != nullptr, P_ERR);
	const uint64_t time_micro_s =
		get_time_microseconds();
	if(timeout_micro_s > 15*1000*1000){
		print("timeout_micro_s is larger than the defined sane maximum, might cause some memory bloat", P_WARN);
		// continue it anyways
	}
	const bool timeout =
		time_micro_s-request->get_request_time() > timeout_micro_s;
	return timeout;
}

#define NET_PROTO_REQUEST_CLEANUP_CREATOR(type)				\
	if(true){							\
		std::vector<id_t_> vector =				\
			ID_TIER_CACHE_GET(#type);			\
		for(uint64_t i = 0;i < vector.size();i++){		\
			try{						\
				if(net_proto_obsolete_request(		\
					   PTR_DATA(vector[i],		\
						    type),		\
					   15*1000*1000)){		\
					ID_TIER_DESTROY(vector[i]);	\
				}					\
			}catch(...){}					\
		}							\
	}								\
	
static void net_proto_all_request_cleanup(){
	NET_PROTO_REQUEST_CLEANUP_CREATOR(net_proto_id_request_t);
	NET_PROTO_REQUEST_CLEANUP_CREATOR(net_proto_type_request_t);
	NET_PROTO_REQUEST_CLEANUP_CREATOR(net_proto_linked_list_request_t);
}


void net_proto_requests_loop(){
	net_proto_all_request_cleanup();
	net_proto_routine_request_loop();
	net_proto_simple_request_loop();
	net_proto_create_linked_list_request_loop();
}

void net_proto::request::add_type_hash_whitelist(
	std::vector<type_t_> type,
	hash_t_ hash){
	net_proto_type_request_t *type_request =
		new net_proto_type_request_t;
	type_request->set_type(
		type);
	type_request->set_hash(
		hash); // one hash per type request
	type_request->set_ttl_micro_s(
		10*1000*1000); // arbitrary
	id_t_ arbitrary_id;
	set_id_uuid(&arbitrary_id, 0);
	set_id_hash(&arbitrary_id, hash);
	set_id_type(&arbitrary_id, 0);
	type_request->set_destination_peer_id(
		net_proto::peer::optimal_peer_for_id(
			arbitrary_id));
	type_request->set_origin_peer_id(
		net_proto::peer::get_self_as_peer());
	type_request->update_request_time();
	
}
