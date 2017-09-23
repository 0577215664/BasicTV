#ifndef NET_HTTP_H
#define NET_HTTP_H

#include "../../id/id.h"
#include "../../id/id_api.h"

#include "net_http_payload.h"

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


/*
  HTTP BREAKDOWN:
  Inbound connections are put into the non_bound_sockets
 
  They are ran through http::payload::serialize_from_socket until we
  have a complete header to work with (verified with /r/n/r/n)

  Once we have enough information, they are bound to a file_driver
  state and the state is listed in bound_file_driver_states

  bound_file_driver_states have a net_http_payload_t responding
  to the request (Connection: close is needed to simplify this), as
  well as the initial request passed to them through the init function.

  Any new data from that file driver is pushed to that response, and
  the HTTP code sends it down the TCP socket (flags it as sent). Data
  can only be added to the end of the vector.

  Any flags about the header information (defined length, MIME type,
  etc) are handled in the file driver state's HTTP response header

  NOTE: The HTTP header stored in net_http_chunk_header_t isn't
  strictly a HTTP request/response header. It is also used for
  borders for a multipart form, and other stuff as well. Also, there
  are certain headers that need to be comptued on the fly (date, maybe
  size, etc), and the first half is put in, the assemble() function 
  recognizes that and inserts it properly.

  NOTE: There's no restriction on what can be broken down into
  net_http_chunk_ts, if the header is blank, then the payload is
  just passed through to the socket, pretty useful in cases where
  there are latencies with generating the webpage.
*/

typedef std::pair<id_t_, net_http_payload_t> net_http_tuple_thing_t;

struct net_http_t{
private:
	id_t_ conn_socket_id = ID_BLANK_ID;
	// connection has been opened, but we don't have any
	// HTTP header information (or not enough to completely parse)
	std::vector<std::pair<id_t_, net_http_payload_t> > non_bound_sockets;
	std::vector<id_t_> bound_file_driver_states;

public:
	data_id_t id;
	net_http_t();
	~net_http_t();
	GET_SET_ID(conn_socket_id);
	GET_SET(non_bound_sockets, std::vector<net_http_tuple_thing_t>);
	ADD_DEL_VECTOR(non_bound_sockets, net_http_tuple_thing_t);
	GET_SET(bound_file_driver_states, std::vector<id_t_>);
	ADD_DEL_VECTOR(bound_file_driver_states, id_t_);
};

const static std::vector<uint8_t> http_header_divider = {'\r', '\n', '\r', '\n'};

extern void net_http_init();
extern void net_http_loop();
extern void net_http_close();

#endif
