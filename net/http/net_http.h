#ifndef NET_HTTP_H
#define NET_HTTP_H

#include "../../id/id.h"
#include "../../id/id_api.h"

#include <functional>
#include <string>
#include <vector>

/*
  HTTP workflow:
  net_http_t conn_socket accepts requests and pulls the GET header
  GET header's URL is ran over all net_http_file_driver_medium_ts,
  gives to the first (or all?) min_valid_urls
  net_http_file_driver_state_t returns the vector and a 
  variable to let net_http_t know where the EOF is (PAYLOAD_PROGRESS
  or PAYLOAD_COMPLETE)
  net_http_t, once the transfer is complete, is responsible for deleting
  the TCP socket and the net_http_file_driver_state_t
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
public:
	data_id_t id;
	net_http_t();
	~net_http_t();
	GET_SET_ID(conn_socket_id);
	ADD_DEL_VECTOR(non_bound_sockets, id_t_);
};

extern void net_http_init();
extern void net_http_loop();
extern void net_http_close();

#endif
