#ifndef NET_HTTP_H
#define NET_HTTP_H

#include "../../id/id.h"
#include "../../id/id_api.h"

#include <functional>
#include <string>
#include <vector>

/*
  The HTTP sink is meant for long term playback to software, normally not
  on this computer. All requests for data initiate from outside of this
  program, there are no latencies to deal with. However, it is possible
  to stream live through HTTP by requesting a currently viewed stream.
 */

// HTTP interface, bound to a socket
// Some files can't be bound to some sockets
// Any HTTP interface has access to all HTTP files (easy, but maybe create
// multiple for different interfaces?)
struct net_http_t{
private:
	id_t_ conn_socket_id = ID_BLANK_ID;
	// connection has been opened, but we don't have any
	// HTTP header information (or not enough to completely parse)
	std::vector<id_t_> non_bound_sockets;
	std::vector<id_t_> bound_file_driver_states;
public:
	data_id_t id;
	net_http_t();
	~net_http_t();
	GET_SET_ID(conn_socket_id);
	GET_SET(non_bound_sockets, std::vector<id_t_>);
	ADD_DEL_VECTOR(non_bound_sockets, id_t_);
	GET_SET(bound_file_driver_states, std::vector<id_t_>);
	ADD_DEL_VECTOR(bound_file_driver_states, id_t_);
};

extern void net_http_init();
extern void net_http_loop();
extern void net_http_close();

#endif
