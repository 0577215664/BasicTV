#include "../../main.h"
#include "../../id/id_api.h"
#include "../../util.h"
#include "../net.h"
#include "../net_socket.h"
#include "net_proto_meta.h"
#include "net_proto_socket.h"
#include "../../id/id_api.h"
#include "../../encrypt/encrypt.h"
#include "../../escape.h"

/*
  New transport model:
  TCP and UDP will both use encapsulation of data provided by escape.cpp
  to handle the payload and the metadata. Each chunk of data should look
  something like this:
  [STANDARD DATA #1] [PAYLOAD #1] [STANDARD DATA #2] [PAYLOAD #2]

  No spaces nor numbers are used, they are purely visual.

  If a payload is received without a corresponding standard data block, it
  is disregarded (TODO: is this good behavior?). 
 */

net_proto_socket_t::net_proto_socket_t() : id(this, TYPE_NET_PROTO_SOCKET_T){
	id.add_data_id(&socket_id, 1, mem_ruleset);
	id.add_data_id(&peer_id, 1, mem_ruleset);
	id.add_data_raw(&flags, sizeof(flags), mem_ruleset);
	id.add_data_raw(&last_recv_micro_s, sizeof(last_recv_micro_s), mem_ruleset);
	id.add_data_one_byte_vector(&working_buffer, ~0, mem_ruleset);

	net_proto_standard_data_t std_data_;
	std_data_.ver_major = VERSION_MAJOR;
	std_data_.ver_minor = VERSION_MINOR;
	std_data_.ver_patch = VERSION_REVISION;
	std_data_.macros = 0;
	std_data_.unused = 0;
	std_data_.peer_id = net_proto::peer::get_self_as_peer();
	std_data =
		net_proto_write_packet_metadata(
			std_data_);
	ID_MAKE_TMP(id.get_id());
}

net_proto_socket_t::~net_proto_socket_t(){
}

void net_proto_socket_t::update_working_buffer(){
	net_socket_t *socket_ptr =
		PTR_DATA(socket_id,
			 net_socket_t);
	if(socket_ptr == nullptr){
		print("socket is a nullptr", P_ERR);
	}
	std::vector<uint8_t> buffer;
	try{
		buffer = socket_ptr->recv_all_buffer();
	}catch(...){
		print("socket has closed, deleting and self-terminating", P_DEBUG);
		ID_TIER_DESTROY(socket_ptr->id.get_id());
		socket_ptr = nullptr;
		print("throwing for caller to delete me", P_UNABLE);
	}
	if(buffer.size() != 0){
		P_V(buffer.size(), P_VAR);
		last_recv_micro_s = get_time_microseconds();
		working_buffer.insert(
			working_buffer.end(),
			buffer.begin(),
			buffer.end());
	}
}

void net_proto_socket_t::update_block_buffer(){
	std::pair<std::vector<uint8_t>, std::vector<uint8_t> > block_data;
	while((block_data = unescape_vector(
		       working_buffer,
		       NET_PROTO_ESCAPE)).first.size() != 0){
		print("received another block of data", P_SPAM);
		working_buffer = block_data.second;
		if(block_data.first.size() != 0){
			if(block_buffer.size() == 0){
				block_buffer.push_back(
					std::make_pair(
						std::vector<uint8_t>(),
						std::vector<uint8_t>()));
			}
			if(block_buffer[block_buffer.size()-1].first.size() == 0){
				block_buffer[block_buffer.size()-1].first =
					block_data.first;
			}else if(block_buffer[block_buffer.size()-1].second.size() == 0){
				block_buffer[block_buffer.size()-1].second =
					block_data.first;
			}else{
				block_buffer.push_back(
					std::make_pair(
						block_data.first,
						std::vector<uint8_t>({})));
			}
		}else{
			print("socket appears up to date (no full escaped vectors to read)", P_SPAM);
		}
	}
}

void net_proto_socket_t::send_id(id_t_ id_){
	data_id_t *id_tmp =
		PTR_ID(id_, );
	if(id_tmp == nullptr){
		print("id to send is a nullptr", P_ERR);
	}	
	net_socket_t *socket_ptr =
		PTR_DATA(socket_id,
			 net_socket_t);
	if(socket_ptr == nullptr){
		print("socket is a nullptr", P_ERR);
	}
	std::vector<std::vector<uint8_t> > payload =
		id_tier::operation::get_data_from_state(
			id_tier::state_tier::optimal_state_vector_of_tier_vector(
			        all_tiers),
			{id_});
	ASSERT(payload.size() > 0, P_UNABLE);
	for(uint64_t i = 0;i < payload.size();i++){
		payload[i] = id_api::raw::force_to_extra(
			payload[i],
			ID_EXTRA_ENCRYPT & ID_EXTRA_COMPRESS);
		if(payload[i].size() == 0){
			print("exported size of ID is zero, not sending", P_NOTE);
			return;
		}
		// can simplify to one vector, not done for debugging reasons
		P_V(payload[i].size(), P_VAR);
		std::vector<uint8_t> std_data_postescape =
			escape_vector(std_data, NET_PROTO_ESCAPE);
		std::vector<uint8_t> payload_postescape =
			escape_vector(payload[i], NET_PROTO_ESCAPE);
		P_V(std_data_postescape.size(), P_VAR);
		P_V(payload_postescape.size(), P_VAR);
		socket_ptr->send(std_data_postescape);
		socket_ptr->send(payload_postescape);
	}
}

void net_proto_socket_t::send_id_vector(std::vector<id_t_> id_vector){
	for(uint64_t i = 0;i < id_vector.size();i++){
		send_id(id_vector[i]);
		// TODO: should optimize this somehow...
	}
}

/*
  LOAD_BLOCKS is somehow breaking it
 */

void net_proto_socket_t::load_blocks(){
	for(uint64_t i = 0;i < block_buffer.size();i++){
		if(block_buffer[i].first.size() != 0 &&
		   block_buffer[i].second.size() != 0){
			/*
			  They have to at least be complete blocks because of
			  the encapsulation of the data blocks being sent and
			  the automatic movement and decapsulation from
			  working_buffer

			  Since link layer encryption isn't a thing right now,
			  i'm not worried about implementing decoding code,
			  although that wouldn't be a bad idea
			 */
			print("found a complete block_buffer set", P_SPAM);
			net_proto_standard_data_t inbound_std_data;
			net_proto_read_packet_metadata(
				block_buffer[i].first,
				&inbound_std_data);
			if(inbound_std_data.peer_id != peer_id){
				print("sent peer ID and current peer ID do not"
				      " match, assume this is a bootstrap", P_NOTE);
 				/*
				  Because bootstrapping is creating a new
				  peer ID for the other client, one with 
				  my hash
				 */
				net_proto_peer_t *wrong_peer_ptr =
					PTR_DATA(peer_id,
						 net_proto_peer_t);
				delete wrong_peer_ptr;
				wrong_peer_ptr = nullptr;
				peer_id = inbound_std_data.peer_id;
			}
			P_V(block_buffer[i].second.size(), P_VAR); // temporary
			id_api::add_data(
				block_buffer[i].second);
			const id_t_ inbound_id =
				id_api::raw::fetch_id(
					block_buffer[i].second);
			print("received ID " +
			      convert::array::id::to_hex(inbound_id) + " of type " +
			      convert::type::from(
				      get_id_type(
					      inbound_id)), P_DEBUG);
			if(get_id_type(inbound_id) != TYPE_NET_PROTO_ID_REQUEST_T &&
			   get_id_type(inbound_id) != TYPE_NET_PROTO_TYPE_REQUEST_T &&
			   get_id_type(inbound_id) != TYPE_NET_PROTO_LINKED_LIST_REQUEST_T &&
			   get_id_type(inbound_id) != TYPE_NET_PROTO_CON_REQ_T){
				net_proto::request::del_id(
					inbound_id);
			} // All requests for data are on a push
			block_buffer.erase(
				block_buffer.begin()+i);
			i--;
		}
	}
}

void net_proto_socket_t::check_state(){
	// turns out this doesn't solve the problem, but the function
	// and a state system for sockets can still be useful, so i'm leaving
	// this here for now
}

void net_proto_socket_t::update(){
	update_working_buffer();
	update_block_buffer();
	load_blocks();
	check_state();
 }
