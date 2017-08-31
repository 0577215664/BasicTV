#include "../../id/id.h"
#include "../net_const.h"
#include "../net_socket.h"
#include "../../util.h"
#ifndef NET_PROTO_SOCKET_H
#define NET_PROTO_SOCKET_H
#include <SDL2/SDL_net.h>

/*
  net_proto_socket_t: handles protocol specific transcoding

  net_proto_socket_t is getting re-implemented as an ID tier so the
  statistics information can be fetched, computed, and IDs can be
  managed more efficiently on disks and individual peers.
 */

#define NET_PROTO_SOCKET_NO_ENCRYPT (1 << 1)

#define NET_PROTO_SOCKET_STATE_PEER_NOT_SENT 0
#define NET_PROTO_SOCKET_STATE_PEER_SENT 1

struct net_proto_socket_t{
private:
	id_t_ socket_id = ID_BLANK_ID;
	id_t_ peer_id = ID_BLANK_ID;
	uint8_t flags = 0;
	uint8_t state = 0;
	uint64_t last_recv_micro_s = 0;
	std::vector<uint8_t> std_data;
	std::vector<uint8_t> working_buffer;
	std::vector<std::pair<std::vector<uint8_t>, std::vector<uint8_t> > > block_buffer;
	void update_working_buffer();
	void update_block_buffer();
	void load_blocks();
	void check_state();
	
public:
	data_id_t id;
	net_proto_socket_t();
	~net_proto_socket_t();

	// getters and setters
	void set_socket_id(id_t_ socket_id_){socket_id = socket_id_;}
	id_t_ get_socket_id(){return socket_id;}
	void set_peer_id(id_t_ peer_id_){peer_id = peer_id_;}
	id_t_ get_peer_id(){return peer_id;}
	void set_flags(uint8_t flags_){flags = flags_;}
	uint8_t get_flags(){return flags;}

	// send stuff
	void send_id(id_t_ id_);
	void send_id_vector(std::vector<id_t_> id_vector);
	void update();
	uint64_t get_last_recv_micro_s(){return last_recv_micro_s;}
	// TODO: should probably do something else...
	bool is_alive(){return get_time_microseconds()-last_recv_micro_s <= 30*1000*1000;}
};

#endif
