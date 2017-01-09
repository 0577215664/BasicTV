#include "net_proto_inbound_data.h"
// meta has apply and unapply functions for DEV_CTRL_1
#include "../net_proto_meta.h"

// static std::vector<uint8_t> net_proto_read_struct_segment(uint8_t *data,
// 							  uint64_t data_size){
// 	// size is the rest of data, not the size of the read data
// 	net_proto_standard_size_t size = 0;
// 	net_proto_standard_ver_t ver_major = 0;
// 	net_proto_standard_ver_t ver_minor = 0;
// 	net_proto_standard_ver_t ver_patch = 0;
// 	net_proto_standard_macros_t macros = 0;
// 	net_proto_standard_unused_t unused = 0;
// 	net_proto_read_packet_metadata(data,
// 				       data_size,
// 				       &size,
// 				       &ver_major,
// 				       &ver_minor,
// 				       &ver_patch,
// 				       &macros,
// 				       &unused);
// 	if(unused != 0){
// 		print("received a packet that utilized currently unused space, you might be running an old version", P_WARN);
// 	}
// 	if(data_size < size){
// 		// not enough data has been read yet
// 		return {};
// 	}
// 	return std::vector<uint8_t>(data, data+size);
// }

// /*
//   Reads all readable data from packet buffer, and returns a vector of the
//   RAW strings from the socket (needs to be iterated over with the unapply
//   function)

//   This code hasn't been tested at all, and should be cleaned up a lot before
//   actual use takes place

//   All of the struct segments should be ran through a decoder to remove the
//   extra DEV_CTRL_1 entries. The extra characters that are written should be
//   taken into account when computing the payload length.

//   TODO: offset the work of importing to the net_socket_t. Assuming all
//   sockets created follow this rule of only sending this sort of information,
//   that would make everything nicer (I can't think of any other case right now)
//  */

// #define EFFECTIVE_LENGTH() (buffer.size()-(i+1)-NET_PROTO_META_LENGTH)

// static std::vector<std::vector<uint8_t> > net_proto_get_struct_segments(net_socket_t *socket){
// 	std::vector<std::vector<uint8_t> > retval;
// 	std::vector<uint8_t> buffer =
// 		socket->recv(
// 			-socket->get_backwards_buffer_size(),
// 			NET_SOCKET_RECV_NO_HANG);
// 	uint64_t i = 0;
// 	// we don't know how nested it is
// 	for(;i < buffer.size();i++){
// 		if(buffer[i] != NET_PROTO_DEV_CTRL_1){
// 			break;
// 		}
// 	}
// 	for(;i < buffer.size();i++){
// 		try{
// 			if(buffer.at(i-1) != NET_PROTO_DEV_CTRL_1 &&
// 			   buffer.at(i+0) == NET_PROTO_DEV_CTRL_1 &&
// 			   buffer.at(i+1) != NET_PROTO_DEV_CTRL_1){
// 				net_proto_standard_size_t payload_size = 0;
// 				net_proto_read_packet_metadata(&(buffer[i]),
// 							       buffer.size()-i,
// 							       &payload_size,
// 							       nullptr,
// 							       nullptr,
// 							       nullptr,
// 							       nullptr,
// 							       nullptr);
// 				if(EFFECTIVE_LENGTH() < payload_size){
// 					/*
// 					  Unless the read buffer becomes large
// 					  enough, I need to precisely measure
// 					  how much data should be read and only
// 					  read that much to prevent chopping off
// 					  additional metadata
// 					 */
// 					std::vector<uint8_t> new_data =
// 						socket->recv(
// 							payload_size-EFFECTIVE_LENGTH(),
// 							NET_SOCKET_RECV_NO_HANG);
// 					buffer.insert(
// 						buffer.end(),
// 						new_data.begin(),
// 						new_data.end());
// 				}
// 				if(payload_size <= EFFECTIVE_LENGTH()){
// 					retval.push_back(
// 						net_proto_read_struct_segment(
// 							&(buffer[i+1]),
// 							payload_size));
// 				}
// 				i += payload_size;
// 			}
// 		}catch(...){}
// 	}
// 	return retval;
// }

// /*
//   Fetches all incoming data and handle it
//  */

// void net_proto_loop_handle_inbound_data(){
// 	net_socket_t *tmp_socket = nullptr;
// 	std::vector<uint64_t> peer_id_list =
// 		id_api::cache::get("net_peer_t");
// 	for(uint64_t i = 0;i < peer_id_list.size();i++){
// 		net_peer_t *peer =
// 			PTR_DATA(peer_id_list[i], net_peer_t);
// 		for(uint64_t s = 0;s < NET_PROTO_MAX_SOCKET_PER_PEER;s++){
// 			tmp_socket =
// 				PTR_DATA(peer->get_socket_id(s),
// 					 net_socket_t);
// 			if(likely(tmp_socket == nullptr)){
// 				continue;
// 			}
// 			/*
// 			  Ideally, store all of this on a global std::vector
// 			  and create worker threads to parse through it with
// 			  locks.
// 			 */
// 			std::vector<std::vector<uint8_t> > segments =
// 				net_proto_get_struct_segments(tmp_socket);
// 			for(uint64_t seg = 0;seg < segments.size();seg++){
// 				id_api::array::add_data(
// 					net_proto_unapply_dev_ctrl(
// 						std::vector<uint8_t>(
// 							segments[seg].begin()+NET_PROTO_META_LENGTH,
// 							segments[seg].end()
// 							)
// 						)
// 					);
// 			}
// 		}
// 	}
// }


// // reads from all net_socket_t, for testing only
// void net_proto_loop_dummy_read(){
// 	std::vector<uint64_t> all_sockets =
// 		id_api::cache::get("net_socket_t");
// 	for(uint64_t i = 0;i < all_sockets.size();i++){
// 		net_socket_t *socket_ =
// 			PTR_DATA(all_sockets[i], net_socket_t);
// 		if(socket_ == nullptr){
// 			print("socket is nullptr", P_ERR);
// 			continue;
// 		}
// 		if(socket_->get_client_conn().first == ""){
// 			// inbound
// 			continue;
// 		}
// 		if(socket_->activity()){
// 			print("detected activity on a socket", P_SPAM);
// 			std::vector<uint8_t> incoming_data;
// 			while((incoming_data = socket_->recv(1, NET_SOCKET_RECV_NO_HANG)).size() != 0){
// 				P_V_E((uint64_t)incoming_data[0], P_NOTE);
// 			}
// 		}
// 	}
// }

// // reads what we have, returns proper status depending on it

// void net_proto_loop_handle_inbound_requests(){
	
// }

/*
  TODO: copy and convert code as needed to get this working with 
  net_proto_socket_t (hopefuly simplify the decoding).
 */

void net_proto_loop_dummy_read(){
}

void net_proto_process_buffer_vector(id_t_ socket_id, std::vector<std::vector<uint8_t> > buffer_vector){
	for(uint64_t i = 0;i < buffer_vector.size();i++){
		id_t_ imported_data_id =
			id_api::array::add_data(buffer_vector[i]);
		data_id_t *imported_data_ptr =
			PTR_ID(imported_data_id,
			       net_request_t);
		if(imported_data_ptr != nullptr){
			net_request_t *request =
				(net_request_t*)imported_data_ptr->get_ptr();
			request->set_socket_id(socket_id);
		}
	}
}

void net_proto_loop_handle_inbound_data(){
	std::vector<id_t_> proto_sockets =
		id_api::cache::get("net_proto_socket_t");
	for(uint64_t i = 0;i < proto_sockets.size();i++){
		net_proto_socket_t *proto_socket =
			PTR_DATA(proto_sockets[i],
				 net_proto_socket_t);
		if(proto_socket == nullptr){
			print("proto_socket is a nullptr", P_ERR);
		}
		proto_socket->update();
		net_proto_process_buffer_vector(
			proto_sockets[i],
			proto_socket->get_buffer());
	}
}

void net_proto_loop_handle_inbound_requests(){
}
