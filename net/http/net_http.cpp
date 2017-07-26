#include "net_http.h"
#include "../net.h"
#include <tuple>
#include <algorithm>

#include "../../settings.h"

net_http_t::net_http_t() : id(this, TYPE_NET_HTTP_T){
}

net_http_t::~net_http_t(){
}

void net_http_init(){
	// TODO: make this a setting
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

static void net_http_send_file_conn(
	net_http_t *http_data_ptr,
	net_socket_t *socket_ptr){
	TCPsocket conn_socket =
		socket_ptr->get_tcp_socket();
	TCPsocket new_socket =
		nullptr;
	while((new_socket = SDLNet_TCP_Accept(conn_socket)) != nullptr){
		net_http_file_conn_instance_t conn_instance;
		net_socket_t *new_net_socket =
			new net_socket_t;
		new_net_socket->set_tcp_socket(
			new_socket);
		conn_instance.set_socket_id(
			new_net_socket->id.get_id());
		// Looks up the proper file per the HTTP GET
		// Add net_http_file_conn_instance_t to the file entry
		print("finish me", P_CRIT);
	}
}

static void net_http_add_file_conn(
	net_http_t *http_data_ptr,
	net_socket_t *socket_ptr){

}

void net_http_loop(){
	std::vector<id_t_> http_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_HTTP_T);
	for(uint64_t i = 0;i < http_vector.size();i++){
		net_http_t *http_data_ptr =
			PTR_DATA(http_vector[i],
				 net_http_t);
		CONTINUE_IF_NULL(http_data_ptr, P_WARN);
		net_socket_t *socket_ptr =
			PTR_DATA(http_data_ptr->get_conn_socket_id(),
				 net_socket_t);
		CONTINUE_IF_NULL(socket_ptr, P_WARN);
		net_http_send_file_conn(
			http_data_ptr,
			socket_ptr);
		net_http_add_file_conn(
			http_data_ptr,
			socket_ptr);
	}
}

void net_http_close(){
}
