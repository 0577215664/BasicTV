#include "../main.h"
#include "../util.h"
#include "net.h"
#include "net_socket.h"
#include "../id/id_api.h"
#include "../math/math.h"

#define NET_LOCK(x) std::lock_guard<std::mutex>lock()
#define NET_UNLOCK(x) x.unlock()

static void recv_to_buffer(
	std::vector<uint8_t> *buffer,
	TCPsocket *socket,
	bool *recv_running,
	net_socket_t *ptr){

	print("recv buffer has started", P_NOTE);
	ASSERT(*recv_running == true, P_ERR);

	uint64_t iterator = 0;
	
	char recv_buffer[65536];
	while(*socket != nullptr &&
	      socket != nullptr &&
	      *recv_running){
		while(ptr->activity()){
			NET_LOCK(ptr->thread_mutex);
			try{
				const int32_t recv_retval = 
					SDLNet_TCP_Recv(
						*socket,
						&(recv_buffer[0]),
						65536);
				if(recv_retval > 0){
						buffer->insert(
							buffer->end(),
							&(recv_buffer[0]),
							&(recv_buffer[recv_retval]));
				}else if(recv_retval == -1){
					print("SDLNet_TCP_Recv failed with: " + (std::string)(SDL_GetError()), P_WARN);
				}
			}catch(...){
				print("exception caught in recv_to_buffer", P_WARN);
			}
			NET_UNLOCK(ptr->thread_mutex);
			sleep_ms(1);
		}
		iterator++;
	}
}

static void send_from_buffer(
	std::vector<uint8_t> *buffer,
	TCPsocket *socket,
	bool *send_running,
	net_socket_t *ptr){

	print("send buffer has started", P_NOTE);
	ASSERT(*send_running == true, P_ERR);

	uint64_t iterator = 0;
	
	std::vector<uint8_t> send_buffer;
	while(*socket != nullptr &&
	      socket != nullptr &&
	      *send_running){
		NET_LOCK(ptr->thread_mutex);
		try{
			send_buffer =
				*buffer;
			buffer->clear();
			if(send_buffer.size() > 0){
				const int64_t sent_bytes =
					SDLNet_TCP_Send(
						*socket,
						send_buffer.data(),
						send_buffer.size());
				send_buffer.clear();
			}
		}catch(...){
			print("exception caught in send thread", P_WARN);
		}
		NET_UNLOCK(ptr->thread_mutex);
		sleep_ms(1);
		iterator++;
	}
}

net_socket_t::net_socket_t() : id(this, TYPE_NET_SOCKET_T){
	id.add_data_raw(&status, sizeof(status));
	id.set_lowest_global_flag_level(
		ID_DATA_NETWORK_RULE_NEVER,
		ID_DATA_EXPORT_RULE_NEVER,
		ID_DATA_PEER_RULE_NEVER);
}

net_socket_t::~net_socket_t(){
	disconnect();
}

void net_socket_t::socket_check(){
	if(is_alive() == false){
		print("socket is null", P_UNABLE);
	}
}

bool net_socket_t::is_alive(){
	return socket != nullptr;
}

void net_socket_t::send(std::vector<uint8_t> data){
	socket_check();
	NET_LOCK(thread_mutex);
	try{
		send_buffer.insert(
			send_buffer.end(),
			data.begin(),
			data.end());
	}catch(...){
		print("exception caught in send", P_ERR);
	}
	NET_UNLOCK(thread_mutex);
}

void net_socket_t::send(std::string data){
	send(std::vector<uint8_t>(
		     data.c_str(),
		     data.c_str()+data.size()));
}

std::vector<uint8_t> net_socket_t::recv(uint64_t byte_count, uint64_t flags){
	std::vector<uint8_t> retval;
	NET_LOCK(thread_mutex);
	try{
		while(!(flags & NET_SOCKET_RECV_NO_HANG) &&
		      recv_buffer.size() < byte_count){
			NET_UNLOCK(thread_mutex);
			NET_LOCK(thread_mutex);
		}
		const uint64_t pull_size =
			(recv_buffer.size() < byte_count) ?
			recv_buffer.size() : byte_count;
		retval =
			std::vector<uint8_t>(
				recv_buffer.begin(),
				recv_buffer.begin()+pull_size);
		recv_buffer.erase(
			recv_buffer.begin(),
			recv_buffer.begin()+pull_size);
	}catch(...){
		print("exception caught in recv", P_ERR);
	}
	NET_UNLOCK(thread_mutex);
	return retval;
}

std::vector<uint8_t> net_socket_t::recv_all_buffer(){
	std::vector<uint8_t> retval;
	NET_LOCK(thread_mutex);
	try{
		retval =
			recv_buffer;
		recv_buffer.clear();
	}catch(...){
		print("exception caught in recv_all_buffer", P_ERR);
	}
	NET_UNLOCK(thread_mutex);
	return retval;
}

void net_socket_t::connect(){
	IPaddress tmp_ip;
	int16_t res_host_retval = 0;
	if(get_net_ip_str() == ""){
		print("opening a listening socket", P_DEBUG);
		res_host_retval = SDLNet_ResolveHost(
			&tmp_ip,
			nullptr,
			get_net_port());
	}else{
		print("opening a standard socket to " +
		      get_net_ip_str() + ":" + std::to_string(get_net_port()), P_DEBUG);
		res_host_retval =
			SDLNet_ResolveHost(
				&tmp_ip,
				get_net_ip_str().c_str(),
				get_net_port());
	}
	if(res_host_retval == -1){
		print((std::string)"cannot resolve host:"+SDL_GetError(),
		      P_ERR);
	}
	socket = SDLNet_TCP_Open(&tmp_ip);
	if(socket == nullptr){
		P_V(get_net_port(), P_WARN);
		print((std::string)"cannot open socket (" + std::to_string(errno) + "):"+SDL_GetError(),
		      P_WARN);
		return;
	}else{
		print("opened socket", P_NOTE);
	}
	update_socket_set();
	running = true;
	NET_LOCK(thread_mutex);
	send_thread =
		std::move(
			std::thread(
				send_from_buffer,
				&send_buffer,
				&socket,
				&thread_running,
				this));
	if(get_net_ip_str() != ""){
		recv_thread =
			std::move(
				std::thread(
					recv_to_buffer,
					&recv_buffer,
					&socket,
					&thread_running,
					this));
	}
	NET_UNLOCK(thread_mutex);
}

void net_socket_t::update_socket_set(){
	NET_LOCK(thread_mutex);
	socket_set = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(socket_set, socket);
	NET_UNLOCK(thread_mutex);
}

void net_socket_t::disconnect(){
	thread_running = false;
	if(recv_thread.joinable()){
		recv_thread.join();
	}
	if(send_thread.joinable()){
		send_thread.join();
	}
	
	NET_LOCK(thread_mutex);
	SDLNet_TCP_Close(socket);
	socket = nullptr;
	SDLNet_FreeSocketSet(socket_set);
	socket_set = nullptr;
	NET_UNLOCK(thread_mutex);
}

void net_socket_t::reconnect(){
	disconnect();
	connect();
}

/*
  only used on accepting incoming connections
 */

void net_socket_t::set_tcp_socket(TCPsocket socket_){
	NET_LOCK(thread_mutex);
	socket = socket_;
	IPaddress tmp_ip;
	tmp_ip = *SDLNet_TCP_GetPeerAddress(socket);
	const char *ip_addr_tmp = SDLNet_ResolveIP(&tmp_ip);
	if(ip_addr_tmp == nullptr){
		print("cannot read IP", P_ERR);
		return;
	}
	set_net_ip(ip_addr_tmp,
		   NBO_16(tmp_ip.port));
	NET_UNLOCK(thread_mutex);
	update_socket_set();
}

TCPsocket net_socket_t::get_tcp_socket(){
	return socket;
}

bool net_socket_t::activity(){
	NET_LOCK(thread_mutex);
	try{
		if(socket == nullptr){
			print("socket is nullptr", P_UNABLE);
		}
		int activity_ = SDLNet_CheckSockets(socket_set, 0) > 0;
		if(activity_ == -1){
			print("SDLNet_CheckSockets failed:" + (std::string)SDL_GetError(), P_ERR);
		}
		return SDLNet_SocketReady(socket) != 0;
	}catch(...){
		print("exception caught in activity()", P_WARN);
	}
	NET_UNLOCK(thread_mutex);
	return false;
}

id_t_ net_socket_t::get_proxy_id(){
	return proxy_id;
}

void net_socket_t::register_inbound_data(
	uint32_t bytes,
	uint64_t start_time_micro_s,
	uint64_t end_time_micro_s){
	net::stats::add_throughput_datum(
		bytes,
		start_time_micro_s,
		end_time_micro_s,
		id.get_id(),
		proxy_id);
}
