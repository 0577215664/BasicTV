#include "../../id/id.h"
#include "../../id/id_api.h"
#include "../../settings.h"
#include "net_proto.h"
#include "net_proto_peer.h"
#include "net_proto_con_req.h"

#include "../interface/net_interface_intermediary.h"

net_proto_con_req_t::net_proto_con_req_t() : id(this, TYPE_NET_PROTO_CON_REQ_T){
	const data_id_transport_rules_t con_req_rules(
		all_mem_cache,
		all_intermediary);
	ADD_DATA(first_peer_id, con_req_rules);
	ADD_DATA(second_peer_id, con_req_rules);
	ADD_DATA(third_peer_id, con_req_rules);
	ADD_DATA(timestamp, con_req_rules);
}

net_proto_con_req_t::~net_proto_con_req_t(){
}

void net_proto_con_req_t::get_peer_ids(id_t_ *first_peer_id_,
				       id_t_ *second_peer_id_,
				       id_t_ *third_peer_id_){
	if(first_peer_id_ != nullptr){
		*first_peer_id_ = first_peer_id;
	}
	if(second_peer_id_ != nullptr){
		*second_peer_id_ = second_peer_id;
	}
	if(third_peer_id_ != nullptr){
		*third_peer_id_ = third_peer_id;
	}
}

uint64_t net_proto_con_req_t::get_timestamp(){
	return timestamp;
}

void net_proto_con_req_t::set(id_t_ first_peer_id_,
			      id_t_ second_peer_id_,
			      id_t_ third_peer_id_,
			      uint64_t timestamp_){
	first_peer_id = first_peer_id_;
	second_peer_id = second_peer_id_;
	third_peer_id = third_peer_id_;
	timestamp = timestamp_;
}
