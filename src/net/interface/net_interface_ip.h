#ifndef NET_INTERFACE_IP_H
#define NET_INTERFACE_IP_H

#include "net_interface_medium.h"
#include "net_interface.h"

#include "SDL2/SDL_net.h"

#include "thread"
#include "mutex"


struct net_interface_medium_ip_ptr_t{
public:
	TCPsocket tcp_socket = nullptr;
	SDLNet_SocketSet socket_set = nullptr;

	std::thread recv_thread;
	std::mutex recv_mutex;
	std::vector<uint8_t> recv_buffer;
	bool recv_running = true;

	std::thread send_thread;
	std::mutex send_mutex;
	std::vector<std::vector<uint8_t> > send_buffer;
	bool send_running = true;
};

extern INTERFACE_ADD_ADDRESS_COST(ip);
extern INTERFACE_ADD_ADDRESS(ip);
extern INTERFACE_CALCULATE_MOST_EFFICIENT_DROP(ip);
extern INTERFACE_CALCULATE_MOST_EFFICIENT_TRANSFER(ip);
extern INTERFACE_SEND(ip);
extern INTERFACE_RECV_ALL(ip);
extern INTERFACE_ACCEPT(ip);
extern INTERFACE_DROP(ip);

#endif
