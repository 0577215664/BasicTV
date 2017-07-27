#include "tv_sink_numerical_tcp_accept.h"
#include "../../tv_sink.h"

#include "../../../../state.h"
#include "../../../tv.h"
#include "../../../tv_frame_numerical.h"
#include "../../../../net/net_socket.h"
#include "../../../../escape.h"

#include "../../../../math/math.h"

// We can assume that the caller to this function knows what they are
// creating, and can hard-set the tcp_accept_state_ptr themselves
// (specifically the socket id)

/*
  You should probably opt for two sinks, one input sink for the data,
  and one output sink for distributing to peers.
 */

TV_SINK_MEDIUM_INIT(tcp_accept){
	STD_STATE_INIT(
		tv_sink_state_t,
		state_ptr,
		tv_sink_numerical_tcp_accept_state_t,
		tcp_accept_state_ptr);
	state_ptr->set_medium(
		TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT);
	state_ptr->set_frame_type(
		TV_FRAME_TYPE_NUMERICAL);
	state_ptr->set_flow_direction(
		flow_direction);
	return state_ptr;
}

static void tv_sink_numerical_tcp_accept_socket_check(
	uint8_t direction,
	tv_sink_numerical_tcp_accept_state_t *tcp_accept_state_ptr,
	net_socket_t *socket_ptr){
	if(socket_ptr->is_alive() == false){
		print("connection accepting socket has died, restarting", P_WARN);
		socket_ptr->connect();
	}
	if(direction == TV_SINK_MEDIUM_FLOW_DIRECTION_IN){
		std::vector<uint8_t> tmp =
			socket_ptr->recv_all_buffer();
		tcp_accept_state_ptr->buffer.insert(
			tmp.begin(),
			tcp_accept_state_ptr->buffer.begin(),
			tcp_accept_state_ptr->buffer.end());
	}
}

TV_SINK_MEDIUM_CLOSE(tcp_accept){
	STD_STATE_CLOSE(
		state_ptr,
		tv_sink_numerical_tcp_accept_state_t);
}

/*
  TCP pull to frame logistics

  Each line of text coming in is formatted as such
  [UNIQUE NUMBER] [VALUE] [UNIT] [TIMESTAMP MICRO S]

  unique number is just any relatively short (32-bit or lower is good) number
  that identifies that stream of data, this is bound either internally to a 
  dimension on the number set (i.e. the frame of numerical data), or is
  internally mapped for oprimization or whatever

  value is just a plain-text version of the number. Floating point, huge, or
  small shouldn't matter in the long run. This can either simplify to a 
  double long IEEE number (if a decimal is detected), or a 64-bit signed if
  there is none detected.

  These restrictions are only in place because I use C-native to variable bit 
  length numbers, and don't actually read the plain-text directly.
 */

#define ASSERT_VALID_DIRECTION(direction) ASSERT(state_ptr->get_flow_direction() == direction, P_ERR);

static std::vector<uint8_t> pull_until_end_of_item(
	std::vector<uint8_t> *buffer){
	auto space_iter =
		std::find(
			buffer->begin(),
			buffer->end(),
			(uint8_t)' ');
	if(space_iter != buffer->end()){
		std::vector<uint8_t> retval(
			buffer->begin(),
			space_iter-1);
		buffer->erase(
			buffer->begin(),
			space_iter);
		return retval;
	}

	auto newline_iter =
		std::find(
			buffer->begin(),
			buffer->end(),
			(uint8_t)'\n');
	if(newline_iter != buffer->end()){
		std::vector<uint8_t> retval(
			buffer->begin(),
			newline_iter-1);
		buffer->erase(
			buffer->begin(),
			newline_iter);
		return retval;
	}
	return std::vector<uint8_t>({});
}

static std::vector<uint8_t> smart_number_creation(
	std::vector<uint8_t> data){
	if(std::find(data.begin(),
		     data.end(),
		     '.') != data.end()){
		print("detected decimal, interpreting as floating point", P_DEBUG);
		return math::number::create(
			std::stold(
				convert::string::from_bytes(
					data)),
			0); // don't bother with units for now
	}else{
		print("no decimal, interpreting as signed 64-bit", P_DEBUG);
		return math::number::create(
			(int64_t)std::stoll(
				convert::string::from_bytes(
					data)),
			0);
	}
}

static std::tuple<uint64_t, std::vector<uint8_t>, uint64_t> formatted_to_computable(
	std::vector<uint8_t> *inbound_buffer){
	std::tuple<uint64_t, std::vector<uint8_t>, uint64_t> tmp;
	auto newline_iter =
		std::find(inbound_buffer->begin(),
			  inbound_buffer->end(),
			  static_cast<uint8_t>('\n'));
	if(newline_iter == inbound_buffer->end()){
		return tmp; // not enough data to make a new frame
	}
	std::vector<uint8_t> tmp_vector =
		std::vector<uint8_t>(
			inbound_buffer->begin(),
			newline_iter);
	// unique number, value (VBL), and timestamp in microseconds
	// no units, but they'd be tied in with the number
	std::vector<uint8_t> old_buffer =
		*inbound_buffer;
	try{
		std::get<0>(tmp) =
			std::stoi(
				convert::string::from_bytes(
					pull_until_end_of_item(
						&tmp_vector)));
		std::get<1>(tmp) =
			smart_number_creation(
				std::vector<uint8_t>(
					pull_until_end_of_item(
						&tmp_vector)));
		pull_until_end_of_item(&tmp_vector); // unit isn't used for now
		std::get<2>(tmp) =
			std::stoull(
				convert::string::from_bytes(
					pull_until_end_of_item(
						&tmp_vector)));
	}catch(...){
		print("couldn't interpret the inbound TCP data", P_ERR);
		*inbound_buffer =
			old_buffer;
	}
	return tmp;
}

static std::vector<uint8_t> frames_to_formatted(
	std::vector<id_t_> frames){
	std::vector<uint8_t> retval;
	for(uint64_t i = 0;i < frames.size();i++){
		tv_frame_numerical_t *frame_numerical_ptr =
			PTR_DATA(frames[i],
				 tv_frame_numerical_t);
		CONTINUE_IF_NULL(frame_numerical_ptr, P_WARN);
		std::vector<std::vector<uint8_t> > data =
			unescape_all_vectors(
				frame_numerical_ptr->get_escaped_one_dimension_data(),
				TV_FRAME_NUMERICAL_ESCAPE).first;
		const uint64_t start_time_micro_s =
			frame_numerical_ptr->get_start_time_micro_s();
		const uint64_t ttl_micro_s =
			frame_numerical_ptr->get_ttl_micro_s();
		for(uint64_t c = 0;c < data.size();c++){
			std::vector<uint8_t> uuid =
				convert::string::to_bytes(
					std::to_string(c));
			std::vector<uint8_t> unit = {};
			std::vector<uint8_t> timestamp_micro_s =
				convert::string::to_bytes(
					std::to_string(
						start_time_micro_s+((c/(data.size()-1))*ttl_micro_s)));
			retval.insert(
				retval.end(),
				uuid.begin(),
				uuid.end());
			retval.push_back('\t');
			retval.insert(
				retval.end(),
				data[c].begin(),
				data[c].end());
			retval.push_back('\t');
			retval.insert(
				retval.end(),
				unit.begin(),
				unit.end());
			retval.push_back('\t');
			retval.insert(
				retval.end(),
				timestamp_micro_s.begin(),
				timestamp_micro_s.end());
			retval.push_back('\n');
		}
	}
	return retval;
}

static void get_new_connections(
	net_socket_t *socket_ptr,
	tv_sink_numerical_tcp_accept_state_t *tcp_accept_state_ptr){
	TCPsocket conn_socket = socket_ptr->get_tcp_socket();
	TCPsocket tmp_socket = nullptr;
	while((tmp_socket = SDLNet_TCP_Accept(conn_socket)) != nullptr){
		print("accepted and listing a new socket to sink", P_NOTE);
		net_socket_t *new_socket_ptr =
			new net_socket_t;
		new_socket_ptr->set_tcp_socket(
			tmp_socket);
		tcp_accept_state_ptr->add_socket_vector(
			new_socket_ptr->id.get_id());
	}
}

TV_SINK_MEDIUM_PULL(tcp_accept){
	STD_STATE_GET_PTR(
		state_ptr,
		tv_sink_numerical_tcp_accept_state_t,
		tcp_accept_state_ptr);
	ASSERT_VALID_DIRECTION(TV_SINK_MEDIUM_FLOW_DIRECTION_IN);
	net_socket_t *socket_ptr =
		PTR_DATA(tcp_accept_state_ptr->get_conn_socket_id(),
			 net_socket_t);
	tv_sink_numerical_tcp_accept_socket_check(
		state_ptr->get_flow_direction(),
		tcp_accept_state_ptr,
		socket_ptr);
	try{
		while(true){
			std::tuple<uint64_t, std::vector<uint8_t>, uint64_t> number =
				formatted_to_computable(
					&tcp_accept_state_ptr->buffer);
			tcp_accept_state_ptr->data_buffer.push_back(
				number);
				
		}
	}catch(...){}
	uint64_t start_time_micro_s =
		std::get<2>(tcp_accept_state_ptr->data_buffer[0]);
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < tcp_accept_state_ptr->data_buffer.size();i++){
		const std::tuple<uint64_t, std::vector<uint8_t>, uint64_t> elem =
			tcp_accept_state_ptr->data_buffer[i];
		if(std::get<2>(elem)-start_time_micro_s > 1*1000*1000){
			tv_frame_numerical_t *frame_numerical_ptr =
				new tv_frame_numerical_t;
			frame_numerical_ptr->set_start_time_micro_s(
				start_time_micro_s);
			frame_numerical_ptr->set_ttl_micro_s(
				std::get<2>(elem)-start_time_micro_s);
			frame_numerical_ptr->set_frame_entry(
				tcp_accept_state_ptr->current_frame_entry++);
			std::vector<uint8_t> escaped_vector;
			for(uint64_t c = 0;c <= i;c++){
				std::vector<uint8_t> tmp =
					escape_vector(
						std::get<1>(tcp_accept_state_ptr->data_buffer[c]),
						TV_FRAME_NUMERICAL_ESCAPE);
				escaped_vector.insert(
					escaped_vector.end(),
					tmp.begin(),
					tmp.end());
			}
			frame_numerical_ptr->set_escaped_one_dimension_data(
				escaped_vector);
			retval.push_back(
				frame_numerical_ptr->id.get_id());
			start_time_micro_s = std::get<2>(elem);
			tcp_accept_state_ptr->data_buffer.erase(
				tcp_accept_state_ptr->data_buffer.begin(),
				tcp_accept_state_ptr->data_buffer.begin()+i);
			i = 0;
		}
	}
	return retval;
}

TV_SINK_MEDIUM_PUSH(tcp_accept){
	STD_STATE_GET_PTR(
		state_ptr,
		tv_sink_numerical_tcp_accept_state_t,
		tcp_accept_state_ptr);
	ASSERT_VALID_DIRECTION(TV_SINK_MEDIUM_FLOW_DIRECTION_OUT);
	net_socket_t *socket_ptr =
		PTR_DATA(tcp_accept_state_ptr->get_conn_socket_id(),
			 net_socket_t);
	PRINT_IF_NULL(socket_ptr, P_ERR);
	tv_sink_numerical_tcp_accept_socket_check(
		state_ptr->get_flow_direction(),
		tcp_accept_state_ptr,
		socket_ptr);
	get_new_connections(
		socket_ptr,
		tcp_accept_state_ptr);

	std::vector<id_t_> socket_vector =
		tcp_accept_state_ptr->get_socket_vector();
	const std::vector<uint8_t> formatted_numerical_data =
		frames_to_formatted(
			frames);
	for(uint64_t i = 0;i < socket_vector.size();i++){
		try{
			net_socket_t *socket_ptr_ =
				PTR_DATA(socket_vector[i],
					 net_socket_t);
			CONTINUE_IF_NULL(socket_ptr, P_WARN);
			socket_ptr_->send(
				formatted_numerical_data);
		}catch(...){}
	}
}
