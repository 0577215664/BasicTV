#include "test.h"

#include "../id/id_api.h"
#include "../util.h"
#include "../net/net_socket.h"
#include "../net/proto/net_proto_socket.h"
#include "../net/proto/net_proto_api.h"

// wallet is used for exporting stuff
#include "../cryptocurrency.h"

void test::net::proto_socket::send_recv(){
	net_socket_t *intermediate_socket = new net_socket_t;
	uint16_t port = 59999;
	/*
	  One pretty cool side-effect of this code is it keeps looping until
	  one port opens and uses it, since ports fit nicely (with the exception
	  of 0) into a 16-bit variable
	 */
	while(intermediate_socket->get_tcp_socket() == nullptr){
		port++; // 60000 is the typical
		intermediate_socket->set_net_ip("", port); // blank means accepts incoming
		intermediate_socket->connect();
	}
	std::vector<std::pair<net_proto_socket_t *, net_socket_t *> > socket_vector =
		{std::make_pair(new net_proto_socket_t,
				new net_socket_t),
		 std::make_pair(new net_proto_socket_t,
				new net_socket_t)};
	std::vector<id_t_> created_ids =
		{socket_vector[0].first->id.get_id(),
		 socket_vector[1].second->id.get_id(),
		 socket_vector[0].first->id.get_id(),
		 socket_vector[1].second->id.get_id(),
		 intermediate_socket->id.get_id()};
		 
	socket_vector[0].first->id.set_lowest_global_flag_level(
		ID_DATA_RULE_UNDEF,
		ID_DATA_EXPORT_RULE_NEVER,
		ID_DATA_RULE_UNDEF);
	socket_vector[1].first->id.set_lowest_global_flag_level(
		ID_DATA_RULE_UNDEF,
		ID_DATA_EXPORT_RULE_NEVER,
		ID_DATA_RULE_UNDEF);
	// as of right now, there shouldn't be any problems with recycling my
	// peer, so long as we are just testing this. This wouldn't normally
	// fly in software since it (should) never try and send something
	// to itself
	socket_vector[0].first->set_peer_id(
		net_proto::peer::get_self_as_peer());
	socket_vector[1].first->set_peer_id(
		net_proto::peer::get_self_as_peer());

	socket_vector[0].first->set_socket_id(
		socket_vector[0].second->id.get_id());
	socket_vector[1].first->set_socket_id(
		socket_vector[1].second->id.get_id());
	// 0 attempts a connect, intermediate_socket accepts, shifts ownership
	// to 1, and allows for transmission of data over the proto_socket
	socket_vector[0].second->set_net_ip("127.0.0.1", port);
	socket_vector[0].second->connect();
	// accept incoming
	TCPsocket new_socket = nullptr;
	while((new_socket = SDLNet_TCP_Accept(intermediate_socket->get_tcp_socket())) == nullptr){
		sleep_ms(1);
	}
	socket_vector[1].second->set_tcp_socket(new_socket);
	// load, export, delete, send, reload
	for(uint64_t i = 0;i < 1000;i++){
		id_t_ wallet_set_id = test_create_generic_id();
		socket_vector[0].first->send_id(wallet_set_id);
		// checks for having it are done on read, not send, we're fine
		ID_TIER_DESTROY(wallet_set_id);
		usleep(1000*1000);
		// sleep_ms(10000);
		socket_vector[1].first->update();
		if(PTR_ID(wallet_set_id, ) == nullptr){
			print("net_proto_socket transcoding failed", P_ERR);
		}else{
			ID_TIER_DESTROY(wallet_set_id);
		}
	}
	for(uint64_t i = 0;i < created_ids.size();i++){
		try{
			ID_TIER_DESTROY(created_ids[i]);
		}catch(...){}
	}
	socket_vector[0].first = nullptr;
	socket_vector[0].second = nullptr;
	socket_vector[1].first = nullptr;
	socket_vector[1].second = nullptr;
	intermediate_socket = nullptr;
}

void test::net::socket::send_recv(){
	net_socket_t *sockets[3] = {
		new net_socket_t,
		new net_socket_t,
		new net_socket_t
	};
	// Zero initiates a connection to One, One creates
	// socket Two to contain the information
	sockets[1]->set_net_ip(
		"", 64000);
	sockets[1]->connect();
	sockets[0]->set_net_ip(
		"127.0.0.1", 64000);
	sockets[0]->connect();
	TCPsocket inbound_socket =
		nullptr;
	while((inbound_socket = SDLNet_TCP_Accept(sockets[1]->get_tcp_socket())) == nullptr){
		sleep_ms(1);
	}
	sockets[2]->set_tcp_socket(
		inbound_socket);
	// create and send one chunk (payload)
	const std::vector<uint8_t> sent_payload =
		true_rand_byte_vector(
			true_rand(
				0, 65536));
	sockets[0]->send(
		sent_payload);
	std::vector<uint8_t> inbound_payload;
	while(inbound_payload.size() < sent_payload.size()){
		std::vector<uint8_t> inbound_tmp =
			sockets[2]->recv_all_buffer();
		inbound_payload.insert(
			inbound_payload.end(),
			inbound_tmp.begin(),
			inbound_tmp.end());
	}
	ASSERT(inbound_payload.size() == sent_payload.size(), P_ERR); // only trips if we receive too much
	ASSERT(inbound_payload == sent_payload, P_ERR);
	inbound_payload = std::vector<uint8_t>();
	sockets[2]->send(
		sent_payload);
	while(inbound_payload.size() < sent_payload.size()){
		std::vector<uint8_t> inbound_tmp =
			sockets[0]->recv_all_buffer();
		inbound_payload.insert(
			inbound_payload.end(),
			inbound_tmp.begin(),
			inbound_tmp.end());
	}
	ASSERT(inbound_payload.size() == sent_payload.size(), P_ERR); // only trips if we receive too much
	ASSERT(inbound_payload == sent_payload, P_ERR);
	delete sockets[0];
	sockets[0] = nullptr;
	delete sockets[1];
	sockets[1] = nullptr;
	delete sockets[2];
	sockets[2] = nullptr;
}
