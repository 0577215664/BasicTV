#include "net_http.h"
#include "file/driver/net_http_file_driver.h"
#include "parse/net_http_parse.h"
#include "../net.h"
#include "../../settings.h"
#include "../../util.h"

#include <tuple>
#include <algorithm>

#define NET_HTTP_FILE_DRIVER_FUNCTION(function) void net_http_##function(net_http_t *http_data_ptr, net_http_file_driver_state_t *file_driver_state_ptr, net_http_file_driver_medium_t file_driver_medium, net_socket_t *socket_ptr)

// also here so i can cleanup the warnings

/*
  TODO: fix me
 */

#define NET_HTTP_FILE_DRIVER_SANE()				\
	ASSERT(http_data_ptr != nullptr, P_ERR);		\
	ASSERT(file_driver_state_ptr != nullptr, P_ERR);	\
	ASSERT(socket_ptr != nullptr, P_ERR);		\
	ASSERT(file_driver_medium.medium != 0, P_ERR);	\
	
net_http_t::net_http_t() : id(this, TYPE_NET_HTTP_T){
}

net_http_t::~net_http_t(){
}

void net_http_init(){
	net_http_t *http_data_ptr =
		new net_http_t;
	net_socket_t *socket_ptr =
		new net_socket_t;
	http_data_ptr->set_conn_socket_id(
		socket_ptr->id.get_id());
	socket_ptr->set_net_ip(
		"", std::stoi(
			settings::get_setting(
				"net_http_port")));
	socket_ptr->connect();
}

static void net_http_accept_conn(
	net_http_t *http_data_ptr){
	net_socket_t *socket_ptr =
		PTR_DATA(http_data_ptr->get_conn_socket_id(),
			 net_socket_t);
	if(socket_ptr == nullptr){
		print("HTTP conn socket is a nullptr, can't make new connections", P_WARN);
		return;
	}
	TCPsocket conn_socket =
		socket_ptr->get_tcp_socket();
	TCPsocket new_socket =
		nullptr;
	while((new_socket = SDLNet_TCP_Accept(conn_socket)) != nullptr){
		net_socket_t *new_net_socket =
			new net_socket_t;
		new_net_socket->set_tcp_socket(
			new_socket);
		http_data_ptr->add_non_bound_sockets(
			std::make_pair(
				new_net_socket->id.get_id(),
				net_http_payload_t()));
	}
}

static void net_http_push_conn_to_file(
	net_http_t *http_data_ptr){
	std::vector<std::pair<id_t_, net_http_payload_t> > non_bound_sockets =
		http_data_ptr->get_non_bound_sockets();
	std::vector<id_t_> http_file_driver_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_HTTP_FILE_DRIVER_STATE_T);
	for(uint64_t c = 0;c < non_bound_sockets.size();c++){
		try{
			http::socket::payload::read(
				&(std::get<1>(non_bound_sockets[c])),
				std::get<0>(non_bound_sockets[c]));
			if(std::get<1>(non_bound_sockets[c]).get_size_chunks() == 0){
				continue;
			}
			std::string url =
				std::get<1>(non_bound_sockets[c]).get_chunks()[0].get_header().fetch_line_from_start("GET").at(1);
			if(url.size() > 0){
				if(url[0] == '/'){
					url.erase(
						url.begin());
				}
			}
			net_http_file_driver_medium_t file_driver_medium =
				net_http_file_driver_get_medium_from_url(
					url);
			net_http_file_driver_state_t *file_driver_state_ptr =
				file_driver_medium.init(
					std::get<1>(non_bound_sockets[c]),
					std::get<0>(non_bound_sockets[c]));
			http_data_ptr->add_bound_file_driver_states(
				file_driver_state_ptr->id.get_id());
		}catch(...){
			print("caught an exception", P_WARN);
			continue;
		}
		non_bound_sockets.erase(
			non_bound_sockets.begin()+c);
	}
	http_data_ptr->set_non_bound_sockets(
		non_bound_sockets);
}

static NET_HTTP_FILE_DRIVER_FUNCTION(packetize_file_to_conn){
	NET_HTTP_FILE_DRIVER_SANE();
	try{
		file_driver_medium.loop(
			file_driver_state_ptr);
	}catch(...){}
	if(file_driver_state_ptr->response_payload.get_size_chunks() == 0){
		print("no data to service", P_WARN);
		return; // no dta
	}
	print("dishing out file driver data " + convert::array::id::to_hex(file_driver_state_ptr->id.get_id()), P_DEBUG);
	http::socket::payload::write(
		&(file_driver_state_ptr->response_payload),
		socket_ptr->id.get_id());
}

static NET_HTTP_FILE_DRIVER_FUNCTION(remove_stale){
	NET_HTTP_FILE_DRIVER_SANE();
	const bool serviced =
		file_driver_state_ptr->response_payload.get_finished();
	bool emptied =
		true;
	const std::vector<net_http_chunk_t> *chunks =
		file_driver_state_ptr->response_payload.get_const_ptr_chunks();
	P_V(serviced, P_VAR);
	for(uint64_t i = 0;i < chunks->size();i++){
		P_V((*chunks)[i].get_sent(), P_VAR);
		if((*chunks)[i].get_sent() == false){
			emptied = false;
		}
	}
	if(serviced && emptied){
		print("removing a finished HTTP connection", P_WARN);
		socket_ptr->disconnect();
		ID_TIER_DESTROY(socket_ptr->id.get_id());
		socket_ptr = nullptr;

		http_data_ptr->del_bound_file_driver_states(
			file_driver_state_ptr->id.get_id());
		file_driver_medium.close(
			file_driver_state_ptr);
		// closer, via state.h, handles deletion
		file_driver_state_ptr = nullptr;
	}
}

// TODO: can refactor this to iterate all driver-centric calls in
// one for loop, macroify them out and continue when a specific condition
// is met (via checking bool retvals)

void net_http_loop(){
	std::vector<id_t_> http_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_HTTP_T);
	for(uint64_t i = 0;i < http_vector.size();i++){
		net_http_t *http_data_ptr =
			PTR_DATA(http_vector[i],
				 net_http_t);
		CONTINUE_IF_NULL(http_data_ptr, P_WARN);
		net_http_accept_conn(
			http_data_ptr);

		net_http_push_conn_to_file(
			http_data_ptr);
		const std::vector<id_t_> bound_file_driver_states =
			http_data_ptr->get_bound_file_driver_states();
		for(uint64_t c = 0;c < bound_file_driver_states.size();c++){
			net_http_file_driver_state_t *file_driver_state_ptr =
				PTR_DATA(bound_file_driver_states[i],
					 net_http_file_driver_state_t);
			CONTINUE_IF_NULL(file_driver_state_ptr, P_WARN);
			net_http_file_driver_medium_t file_driver_medium =
				net_http_file_driver_get_medium(
					file_driver_state_ptr->get_medium());
			net_socket_t *socket_ptr =
				PTR_DATA(file_driver_state_ptr->get_socket_id(),
					 net_socket_t);
			CONTINUE_IF_NULL(socket_ptr, P_WARN);
			net_http_packetize_file_to_conn(
				http_data_ptr,
				file_driver_state_ptr,
				file_driver_medium,
				socket_ptr);
			net_http_remove_stale(
				http_data_ptr,
				file_driver_state_ptr,
				file_driver_medium,
				socket_ptr);
		}
	}
}

void net_http_close(){
}
