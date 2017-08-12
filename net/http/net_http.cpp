#include "net_http.h"
#include "file/driver/net_http_file_driver.h"
#include "net_http_parse.h"
#include "../net.h"
#include <tuple>
#include <algorithm>

#include "../../settings.h"

#define NET_HTTP_FILE_DRIVER_FUNCTION(function) void net_http_##function(net_http_t *http_data_ptr, net_http_file_driver_state_t *file_driver_state_ptr, net_http_file_driver_medium_t file_driver_medium, net_socket_t *socket_ptr)

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
			new_net_socket->id.get_id());
	}
}

static void net_http_push_conn_to_file(
	net_http_t *http_data_ptr){
	std::vector<id_t_> non_bound_sockets =
		http_data_ptr->get_non_bound_sockets();
	std::vector<id_t_> http_file_driver_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_HTTP_FILE_DRIVER_STATE_T);
	for(uint64_t c = 0;c < non_bound_sockets.size();c++){
		try{
			net_socket_t *socket_ptr =
				PTR_DATA(non_bound_sockets[c],
					 net_socket_t);
			CONTINUE_IF_NULL(socket_ptr, P_WARN);
			std::vector<std::vector<std::string> > header =
					http::header::pull_from_socket(
						socket_ptr);
			if(header.size() == 0){
				continue;
			}
			// std::raise(SIGINT);
			std::string url =
				http::header::pull_value(
					header,
					"GET",
					1); // first entry after "GET"
			id_t_ target_id = ID_BLANK_ID;
			try{
				const std::string target_id_str =
					http::header::get::value_from_var_list(
						http::header::get::var_list(
							url),
						"channel_id");
				if(target_id_str != ""){
					target_id =
						convert::array::id::from_hex(
							target_id_str);
				}
			}catch(...){}
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
					target_id,
					non_bound_sockets[c]);
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
	if(file_driver_state_ptr->get_payload_status() == NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE){
		return;
	}
	std::pair<std::vector<uint8_t>, uint8_t> pull_data =
		file_driver_medium.pull(
			file_driver_state_ptr);
	if(pull_data.first.size() == 0){
		return;
	}
	ASSERT(file_driver_state_ptr->get_payload_status() == NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE, P_CRIT);
	print("dishing out file driver data " + convert::array::id::to_hex(file_driver_state_ptr->id.get_id()), P_DEBUG);
	std::vector<uint8_t> http_packet =
		convert::string::to_bytes(
			http::header::make_header(
				file_driver_state_ptr->get_medium(),
				file_driver_state_ptr->get_mime_type(),
				pull_data.second,
				pull_data.first.size()));
	http_packet.insert(
		http_packet.end(),
		pull_data.first.begin(),
		pull_data.first.end());
	socket_ptr->send(
		http_packet);
}

static NET_HTTP_FILE_DRIVER_FUNCTION(remove_stale){
	const bool serviced =
		file_driver_state_ptr->get_payload_status() ==
		NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE;
	const bool emptied =
		socket_ptr->get_const_ptr_send_buffer()->size() == 0;	
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
