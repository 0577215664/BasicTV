#include "../id/id.h"
#include "../math/math.h"
#include "net_const.h"
#include "net_ip.h"
#include "net_proxy.h"
#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <SDL2/SDL_net.h>
#include "thread"
#include "mutex"

/*
  net_socket_t: Manages network sockets. Socket is stored inside of this file. 
  Using torsocks should work with this program, as there are no plans for UDP
  packets whatsoever, and this program isn't doing anything too technical.

  TCP sockets should be able to have multiple chunks of information sent over
  them, but vandals and malicious nodes that mis-represent the data should be
  detected and punished somehow (perhaps have a sanity checker for all values
  built into the ID API, so invalid numbers can be detected?).
  
  TODO: implement multiple Tor circuits at one time. It would be great because
  the decentralized nature works better when multiple TCP streams are being used
  at the same time (to spread information, receiving information won't really
  matter after the ID index is used efficiently).
 */

/*
  IP addresses and port numbers shouldn't exist on their own, but instead in
  std::pair entries
 */

#include "../lock.h"

struct net_socket_t : public net_ip_t{
private:
	std::vector<uint8_t> recv_buffer;
	std::thread recv_thread;
	
	std::vector<uint8_t> send_buffer;
	std::thread send_thread;

	bool thread_running = true;
	
	// raw socket for SDL
	SDLNet_SocketSet socket_set = nullptr;
	TCPsocket socket = nullptr;

	void socket_check();
	id_t_ proxy_id = ID_BLANK_ID;
	
	/*
	  inbound is throughput
	  outbound isn't going to be created, instead we are going to name that
	  latency and use ping/pong system
	*/
	void register_inbound_data(
		uint32_t bytes,
		uint64_t start_time_micro_s,
		uint64_t end_time_micro_s);
	void update_socket_set();

	void create_threads();
	void destroy_threads();
public:
	data_id_t id;
	net_socket_t();
	~net_socket_t();

	std::mutex thread_mutex;
	
	// general socket stuff
	bool is_alive();
	void connect();
	void disconnect();
	void reconnect();
	
	// send and recv functions
	void send(std::vector<uint8_t> data);
	void send(std::string);
	std::vector<uint8_t> recv(uint64_t byte_count = 0, uint64_t flags = 0);
	std::vector<uint8_t> recv_all_buffer();
	bool activity();

	// TODO: should probably move code over to ADD_DEL_VECTOR
	// TODO: allow for mutex locking through ADD_DEL_VECTOR
	ADD_DEL_VECTOR(recv_buffer, uint8_t);
	GET_CONST_PTR(recv_buffer, std::vector<uint8_t>);

	GET_CONST_PTR(send_buffer, std::vector<uint8_t>);
	
	void set_proxy_id(id_t_ proxy_id_);
	id_t_ get_proxy_id();

	// hacky stuff that should be streamlined and abstracted
	void set_tcp_socket(TCPsocket);
	TCPsocket get_tcp_socket();
};
#endif
