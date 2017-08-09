#include "net_http.h"
#include "net_http_file_driver.h"
#include "../net.h"
#include <tuple>
#include <algorithm>

#include "../../settings.h"

net_http_t::net_http_t() : id(this, TYPE_NET_HTTP_T){
}

net_http_t::~net_http_t(){
}

// convert::vector::vectorize_string_with_divider \n is the first dimension
// convert::vector::vectorize_string_with_divider ' ' is the second dimension
typedef std::vector<std::vector<std::string> > net_http_data_t;

std::array<std::string, 3> net_http_pull_get(
	std::vector<std::string> payload){
}

// HTTP header is the first part, second part is the packet payload
std::pair<std::vector<std::vector<std::string> >, std::vector<uint8_t> > net_http_standardize_header(
	std::string payload){
	std::pair<std::vector<std::vector<std::string> >, std::vector<uint8_t> > retval;
	ASSERT(payload.find("\r\n") != std::string::npos, P_ERR);
	std::string header =
		payload.substr(
			0,
			payload.find("\r\n"));
	retval.second =
		convert::string::to_bytes(
			payload.substr(
				payload.find("\r\n")+2,
				payload.size()));
	std::vector<std::string> tmp =
		convert::vector::vectorize_string_with_divider(
			header,
			"\n");
	for(uint64_t i = 0;i < tmp.size();i++){
		retval.first.push_back(
			convert::vector::vectorize_string_with_divider(
				tmp[i],
				" "));
	}
	return retval;
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

static void net_http_accept_conn(
	net_http_t *http_data_ptr,
	net_socket_t *socket_ptr){
	TCPsocket conn_socket =
		socket_ptr->get_tcp_socket();
	TCPsocket new_socket =
		nullptr;
	// std::vector<id_t_> http_file_vector =
	// 	ID_TIER_CACHE_GET(
	// 		TYPE_NET_HTTP_FILE_T);
	// while((new_socket = SDLNet_TCP_Accept(conn_socket)) != nullptr){
	// 	net_socket_t *new_net_socket =
	// 		new net_socket_t;
	// 	new_net_socket->set_tcp_socket(
	// 		new_socket);
	// 	http_data_ptr->add_non_bound_sockets(
	// 		new_net_socket->id.get_id());
	// 	for(uint64_t i = 0;i < http_file_vector.size();i++){
	// 		net_http_file_t *http_file_ptr =
	// 			PTR_DATA(http_file_vector[i],
	// 				 net_http_file_t);
	// 		CONTINUE_IF_NULL(http_file_ptr, P_WARN);
	// 		std::vector<uint8_t> min_file_needed =
	// 			http_file_ptr->get_min_path_needed();
	// 	}
	// 	print("finish me", P_CRIT);
	// }
}

// checks to see if we have a full HTTP header
// if we do, read that in, delete it from the socket buffer, and parse
// the HTTP GET to see if we have any matching file drivers
static void net_http_push_conn_to_file(
	net_http_t *http_data_ptr,
	net_socket_t *socket_ptr){
	
}

// 
static void net_http_packetize_file_to_conn(
	net_http_t *http_data_ptr,
	net_socket_t *socket_ptr){
}

static std::vector<uint8_t> net_http_pull_header_from_socket(
	net_socket_t *socket_ptr){
	uint64_t lf_pos = 0;
	std::vector<uint8_t> retval;
	try{
		const bool has_crlf =
			(lf_pos = socket_ptr->find_recv_buffer(10)) != socket_ptr->get_size_recv_buffer() &&
			socket_ptr->get_elem_recv_buffer(socket_ptr->find_recv_buffer(10)-1) == 13 &&
			socket_ptr->get_elem_recv_buffer(socket_ptr->find_recv_buffer(10)-2) == '\n';
		if(has_crlf){
			retval =
				socket_ptr->pull_erase_until_pos_recv_buffer(
					lf_pos-2);
		}
	}catch(...){}
	return retval;
}

void net_http_loop(){
	std::vector<id_t_> http_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_HTTP_T);
	// Creates and destroys sockets, can be done via net_http_t
	for(uint64_t i = 0;i < http_vector.size();i++){
		net_http_t *http_data_ptr =
			PTR_DATA(http_vector[i],
				 net_http_t);
		CONTINUE_IF_NULL(http_data_ptr, P_WARN);
		net_socket_t *socket_ptr =
			PTR_DATA(http_data_ptr->get_conn_socket_id(),
				 net_socket_t);
		CONTINUE_IF_NULL(socket_ptr, P_WARN);

		net_http_accept_conn(
			http_data_ptr,
			socket_ptr);
		net_http_push_conn_to_file(
			http_data_ptr,
			socket_ptr);
	}
}

void net_http_close(){
}
