#ifndef NET_HTTP_H
#define NET_HTTP_H

#include "../../id/id.h"
#include "../../id/id_api.h"


#include <functional>
#include <string>
#include <vector>

/*
  HTTP interface to the rest of the program

  Any part of the program can add or delete a handler function from the
  master vector freely. Any handler functions have the HTTP POST
  information passed directly to them, and must return with the file to
  download. This means that the entire file needs to be generated for
  downloading at the moment, which works fine for Atom feeds and the like.

  If a file were to be downloaded, expecially when they need to be re-encoded,
  we could really hit a performance wall, and multithreading would be the
  next major step.

  TODO: can probably abstract this out a lot
 */


typedef std::pair<std::vector<uint8_t>, std::vector<uint8_t> > net_http_file_conn_request_line_t;

/*
  The HTTP code (should) handle all the input code in one of two ways:
  
  If NET_HTTP_FILE_CONN_INSTANCE_PAYLOD_DEFINITE is true, then set the
  size of the payload vector in the 'Content-Length' HTTP header, and offload
  the entire payload to the socket.

  If NET_HTTP_FILE_CONN_INSTANCE_PAYLOAD_DEFINITE is false, then don't set
  'Content-Length' in the HTTP header (indefinite size), and offload the payload
  to the socket on every iteration. The file is assumed to be completely sent
  when we close the socket. The HTTP code knows to close the socket when
  NET_HTTP_FILE_CONN_INSTANCE_PAYLOAD_COMPLETE is set.
 */

#define NET_HTTP_FILE_CONN_INSTANCE_PAYLOAD_DEFINITE (1 << 0)
#define NET_HTTP_FILE_CONN_INSTANCE_PAYLOAD_COMPLETE (1 << 1)

struct net_http_file_conn_instance_t{
private:
	// HTTP connection metadata
	id_t_ socket_id = ID_BLANK_ID;
	std::vector<uint8_t> http_buffer;
	std::vector<std::vector<uint8_t> > header; // seperated by newlines (no CRLF)
	std::vector<uint8_t> full_path; // derived from HTTP header

	// Data jargon
	std::vector<uint8_t> payload; // payla
	uint8_t payload_flags = 0;
	std::vector<net_http_file_conn_request_line_t> request_vector;
public:
	bool operator!=(const net_http_file_conn_instance_t &rhs){
		return !(*this == rhs);
	}

	bool operator==(const net_http_file_conn_instance_t &rhs){
		return socket_id == rhs.socket_id &&
		full_path == rhs.full_path;
	}
	
	net_http_file_conn_instance_t();
	~net_http_file_conn_instance_t();
	GET_SET_ID_S(socket_id);
	GET_SET_S(http_buffer, std::vector<uint8_t>);
	GET_SET_S(full_path, std::vector<uint8_t>);
	GET_SET_S(header, std::vector<std::vector<uint8_t> >);
	ADD_DEL_VECTOR_S(request_vector, net_http_file_conn_request_line_t);
	GET_SET_S(request_vector, std::vector<net_http_file_conn_request_line_t>);
};

/*
  For now, we just scan every file and check in on any conn_vectors that they
  reference directly
 */

struct net_http_file_t{
private:
	std::vector<uint8_t> min_path_needed;
	std::vector<net_http_file_conn_instance_t> conn_vector;
public:
	data_id_t id;
	net_http_file_t();
	~net_http_file_t();
	GET_SET(min_path_needed, std::vector<uint8_t>);
	ADD_DEL_VECTOR(conn_vector, net_http_file_conn_instance_t);
	GET_SET(conn_vector, std::vector<net_http_file_conn_instance_t>);
};

// HTTP interface, bound to a socket
// Some files can't be bound to some sockets
struct net_http_t{
private:
	id_t_ conn_socket_id = ID_BLANK_ID;
	std::vector<id_t_> file_vector;
public:
	data_id_t id;
	net_http_t();
	~net_http_t();
	GET_SET_ID(conn_socket_id);
	ADD_DEL_VECTOR(file_vector, id_t_);
	
};

extern void net_http_init();
extern void net_http_loop();
extern void net_http_close();

#endif
