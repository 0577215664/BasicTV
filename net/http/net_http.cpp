#include "net_http.h"
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

static void net_http_add_file_conn(
	net_http_t *http_data_ptr,
	net_socket_t *socket_ptr){
	TCPsocket conn_socket =
		socket_ptr->get_tcp_socket();
	TCPsocket new_socket =
		nullptr;
	std::vector<id_t_> http_file_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_HTTP_FILE_T);
	while((new_socket = SDLNet_TCP_Accept(conn_socket)) != nullptr){
		net_http_file_conn_instance_t conn_instance;
		net_socket_t *new_net_socket =
			new net_socket_t;
		new_net_socket->set_tcp_socket(
			new_socket);
		conn_instance.set_socket_id(
			new_net_socket->id.get_id());
		
		for(uint64_t i = 0;i < http_file_vector.size();i++){
			net_http_file_t *http_file_ptr =
				PTR_DATA(http_file_vector[i],
					 net_http_file_t);
			CONTINUE_IF_NULL(http_file_ptr, P_WARN);
			std::vector<uint8_t> min_file_needed =
				http_file_ptr->get_min_path_needed();
		}
		print("finish me", P_CRIT);
	}
}

static void net_http_send_file_conn(
	net_http_t *http_data_ptr,
	net_socket_t *socket_ptr){
}

static void net_http_update_buffer(
	net_http_file_conn_instance_t *http_conn_ptr,
	net_socket_t *socket_ptr){
	std::vector<uint8_t> socket_buffer =
		socket_ptr->recv_all_buffer();
	if(socket_buffer.size() != 0){
		std::vector<uint8_t> http_buffer =
			http_conn_ptr->get_http_buffer();
		http_buffer.insert(
			http_buffer.end(),
			socket_buffer.begin(),
			socket_buffer.end());
		http_conn_ptr->set_http_buffer(
			http_buffer);
	}
}

static void net_http_pull_header(
	net_http_file_conn_instance_t *http_conn_ptr,
	net_socket_t *socket_ptr){
	if(http_conn_ptr->get_header().size() != 0){
		return;
	}
	std::vector<uint8_t> socket_data =
		http_conn_ptr->get_http_buffer();
	std::vector<uint8_t> retval;
	for(uint64_t i = 0;i < socket_data.size();i++){
		if(socket_data[i] == 13 &&
		   socket_data[i+1] == 10){
			retval.insert(
				retval.begin(),
				socket_data.begin(),
				socket_data.begin()+i);
			socket_data.erase(
				socket_data.begin(),
				socket_data.begin()+i+2); // removes CRLF
						  
		}
	}
	std::vector<std::vector<uint8_t> > formatted_buffer;
	for(uint64_t i = 0;i < retval.size();i++){
		if(retval[i] == '\n'){
			formatted_buffer.push_back(
				std::vector<uint8_t>({}));
		}else{
			formatted_buffer[formatted_buffer.size()-1].push_back(
				retval[i]);
		}
	}
	// maybe sanitize this data even more?
	http_conn_ptr->set_header(
		formatted_buffer);
	
}

static void net_http_pull_path_from_header(
	net_http_file_conn_instance_t *http_conn_ptr,
	net_socket_t *socket_ptr){
	const std::vector<std::vector<uint8_t> > header =
		http_conn_ptr->get_header();
	if(header.size() == 0){
		return;

	}
	// TODO: probably should get a better parsing system
	std::stringstream ss(
		convert::string::from_bytes(
			header.at(0)));
	std::string request, path, proto_version;
	ss >> request >> path >> proto_version;
	P_V_S(request, P_VAR);
	P_V_S(path, P_VAR);
	P_V_S(proto_version, P_VAR);
	ASSERT(request == "GET", P_ERR);
	ASSERT(path.size() > 0, P_ERR);
	if(path[1] == '/'){
		path.erase(0);
	}
	http_conn_ptr->set_full_path(
		convert::string::to_bytes(
			path));
}

static void net_http_service_or_delete(
	net_http_file_conn_instance_t *http_conn_ptr,
	net_socket_t *socket_ptr){
	print("implement me", P_CRIT);
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
		net_http_add_file_conn(
			http_data_ptr,
			socket_ptr);
	}
	/*
	  All HTTP code exists in the following loop, the former only handles
	  creating connections (only thing we can do from a net_http_t instance)
	 */
	std::vector<id_t_> http_file_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_HTTP_FILE_T);
	for(uint64_t i = 0;i < http_file_vector.size();i++){
		net_http_file_t *http_file_ptr =
			PTR_DATA(http_file_vector[i],
				 net_http_file_t);
		CONTINUE_IF_NULL(http_file_ptr, P_WARN);
		std::vector<net_http_file_conn_instance_t> http_conn_vector =
			http_file_ptr->get_conn_vector();
		for(uint64_t c = 0;c < http_conn_vector.size();c++){
			try{
				// functions have checks against double-runs
				net_socket_t *socket_ptr =
					PTR_DATA(http_conn_vector[c].get_socket_id(),
						 net_socket_t);
				CONTINUE_IF_NULL(socket_ptr, P_WARN);
				net_http_update_buffer(
					&http_conn_vector[c],
					socket_ptr);
				net_http_pull_header(
					&http_conn_vector[c],
					socket_ptr);
				net_http_pull_path_from_header(
					&http_conn_vector[c],
					socket_ptr);
				net_http_service_or_delete(
					&http_conn_vector[c],
					socket_ptr);
			}catch(...){}
		}
	}
}

void net_http_close(){
}
