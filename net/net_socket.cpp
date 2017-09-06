#include "../main.h"
#include "../util.h"
#include "net.h"
#include "net_socket.h"
#include "../id/id_api.h"
#include "../math/math.h"

static void recv_to_buffer(
	std::vector<uint8_t> *buffer,
	bool *recv_running,
	net_socket_t *ptr){

	print("recv buffer has started", P_NOTE);	
	char recv_buffer[65536];
	while(ptr != nullptr &&
	      *recv_running){
		while(ptr->activity()){
			try{
				const int32_t recv_retval = 
					SDLNet_TCP_Recv(
						ptr->get_tcp_socket(),
						&(recv_buffer[0]),
						65536);
				if(recv_retval > 0){
					ptr->thread_mutex.lock();
					try{
						buffer->insert(
							buffer->end(),
							&(recv_buffer[0]),
							&(recv_buffer[recv_retval]));
					}catch(...){
						ptr->thread_mutex.unlock();
						print("couldn't add recv_buffer", P_ERR);
					}
					ptr->thread_mutex.unlock();
				}else{ // not a standard connection closing
					if(recv_retval != 0){
						print("SDLNet_TCP_Recv failed with: " + (std::string)(SDL_GetError()), P_WARN);
					}
					*recv_running = false;
					break;
				}
			}catch(...){
				print("exception caught in recv_to_buffer", P_WARN);
				*recv_running = false;
				break;
			}
		}
		sleep_ms(1);
	}
}

static void send_from_buffer(
	std::vector<uint8_t> *buffer,
	bool *send_running,
	net_socket_t *ptr){

	print("send buffer has started", P_NOTE);
	
	std::vector<uint8_t> send_buffer;
	while(ptr != nullptr &&
	      *send_running){
		ptr->thread_mutex.lock();
		try{
			send_buffer =
				*buffer;
			if(send_buffer.size() > 0){
				const int64_t sent_bytes =
					SDLNet_TCP_Send(
						ptr->get_tcp_socket(),
						send_buffer.data(),
						send_buffer.size());
				if(sent_bytes < static_cast<int64_t>(send_buffer.size())){
					print("SDLNet_TCP_Send failed with " + (std::string)(SDL_GetError()), P_WARN);
					*send_running = false;
					break;
				}else{
					print("sent " + std::to_string(sent_bytes) + " bytes", P_DEBUG);
				}
				buffer->clear();
				send_buffer.clear();
			}
		}catch(...){
			print("exception caught in send thread", P_WARN);
		}
		ptr->thread_mutex.unlock();
		sleep_ms(1);
	}
}

net_socket_t::net_socket_t() : id(this, TYPE_NET_SOCKET_T){
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
	thread_mutex.lock();
	try{
		send_buffer.insert(
			send_buffer.end(),
			data.begin(),
			data.end());
	}catch(...){
		print("exception caught in send", P_ERR);
	}
	thread_mutex.unlock();
}

void net_socket_t::send(std::string data){
	send(std::vector<uint8_t>(
		     data.c_str(),
		     data.c_str()+data.size()));
}

std::vector<uint8_t> net_socket_t::recv(uint64_t byte_count, uint64_t flags){
	std::vector<uint8_t> retval;
	thread_mutex.lock();
	try{
		while(!(flags & NET_SOCKET_RECV_NO_HANG) &&
		      recv_buffer.size() < byte_count){
			thread_mutex.unlock();
			thread_mutex.lock();
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
	thread_mutex.unlock();
	return retval;
}

std::vector<uint8_t> net_socket_t::recv_all_buffer(){
	std::vector<uint8_t> retval;
	thread_mutex.lock();
	try{
		retval =
			recv_buffer;
		recv_buffer.clear();
	}catch(...){
		print("exception caught in recv_all_buffer", P_ERR);
	}
	thread_mutex.unlock();
	return retval;
}

void net_socket_t::create_threads(){
	thread_mutex.lock();
	send_thread =
		std::move(
			std::thread(
				send_from_buffer,
				&send_buffer,
				&thread_running,
				this));
	if(get_net_ip_str() != ""){
		recv_thread =
			std::move(
				std::thread(
					recv_to_buffer,
					&recv_buffer,
					&thread_running,
					this));
	}
	thread_mutex.unlock();
}

void net_socket_t::destroy_threads(){
	thread_running = false;
	if(recv_thread.joinable()){
		recv_thread.join();
	}
	if(send_thread.joinable()){
		send_thread.join();
	}
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
	create_threads();
}

void net_socket_t::update_socket_set(){
	thread_mutex.lock();
	if(socket_set != nullptr){
		SDLNet_TCP_DelSocket(socket_set, socket);
		SDLNet_FreeSocketSet(socket_set);
		socket_set = nullptr;
	}
	socket_set = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(socket_set, socket);
	thread_mutex.unlock();
}

void net_socket_t::disconnect(){
	destroy_threads();
	
	thread_mutex.lock();
	SDLNet_TCP_Close(socket);
	socket = nullptr;
	SDLNet_FreeSocketSet(socket_set);
	socket_set = nullptr;
	thread_mutex.unlock();
}

void net_socket_t::reconnect(){
	disconnect();
	connect();
}

/*
  only used on accepting incoming connections
 */

void net_socket_t::set_tcp_socket(TCPsocket socket_){
	thread_mutex.lock();
	socket = socket_;
	IPaddress tmp_ip;
	tmp_ip = *SDLNet_TCP_GetPeerAddress(socket);
	const char *ip_addr_tmp = SDLNet_ResolveIP(&tmp_ip);
	if(ip_addr_tmp == nullptr){
		print("cannot read IP", P_ERR);
		thread_mutex.unlock();
		return;
	}
	set_net_ip(ip_addr_tmp,
		   NBO_16(tmp_ip.port));
	thread_mutex.unlock();
	ASSERT(get_net_ip_str() != "", P_ERR);
	update_socket_set();
	create_threads();
}

TCPsocket net_socket_t::get_tcp_socket(){
	return socket;
}

bool net_socket_t::activity(){
	bool retval = false;
	thread_mutex.lock();
	try{
		if(socket == nullptr){
			print("socket is nullptr", P_UNABLE);
		}
		if(socket_set == nullptr){
			print("socket_set is a nullptr", P_UNABLE);
		}
		int activity_ = SDLNet_CheckSockets(socket_set, 0);
		if(activity_ == -1){
			print("SDLNet_CheckSockets failed:" + (std::string)SDL_GetError(), P_ERR);
		}
		retval = SDLNet_SocketReady(socket) != 0;
	}catch(...){
		print("exception caught in activity()", P_WARN);
	}
	thread_mutex.unlock();
	return retval;
}

id_t_ net_socket_t::get_proxy_id(){
	return proxy_id;
}
