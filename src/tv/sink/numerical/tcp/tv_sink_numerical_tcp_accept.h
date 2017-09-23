#ifndef TV_SINK_NUMERICAL_TCP_ACCEPT_H
#define TV_SINK_NUMERICAL_TCP_ACCEPT_H

#include "../../tv_sink.h"

struct tv_sink_numerical_tcp_accept_state_t{
private:
	id_t_ conn_socket_id = ID_BLANK_ID;
	std::vector<id_t_> socket_vector;

public:
	std::vector<uint8_t> buffer; // socket buffer
	std::vector<std::tuple<uint64_t, std::vector<uint8_t>, uint64_t> > data_buffer;
	uint64_t current_frame_entry = 0;
	
	GET_SET_ID_S(conn_socket_id);
	GET_SET_S(socket_vector, std::vector<id_t_>);
	ADD_DEL_VECTOR_S(socket_vector, id_t_);
};

extern TV_SINK_MEDIUM_INIT(tcp_accept);
extern TV_SINK_MEDIUM_CLOSE(tcp_accept);
extern TV_SINK_MEDIUM_PULL(tcp_accept);
extern TV_SINK_MEDIUM_PUSH(tcp_accept);

#endif
