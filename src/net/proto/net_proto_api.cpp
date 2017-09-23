#include "../../id/id_api.h"
#include "../../util.h"
#include "../../settings.h"
#include "net_proto.h"
#include "net_proto_api.h"
#include "net_proto_peer.h"

static id_t_ self_peer_id = ID_BLANK_ID;

void net_proto::peer::set_self_peer_id(id_t_ self_peer_id_){
	self_peer_id = self_peer_id_;
}

/*
  Should be done in net_proto_init, actually
 */

static uint8_t net_proto_generate_ip_address_nat_type(){
	print("assuming there is no NAT (not safe, but it'll work for now?)", P_WARN);
	return NET_INTERFACE_IP_ADDRESS_NAT_TYPE_NONE;
}

void net_proto::peer::set_self_as_peer(std::string ip, uint16_t port){
	net_proto_peer_t *proto_peer =
		PTR_DATA(self_peer_id,
			 net_proto_peer_t);
	PRINT_IF_NULL(proto_peer, P_ERR);
	// ASSERT(net_interface::medium::from_address(proto_peer->get_address_id()) == NET_INTERFACE_MEDIUM_IP, P_ERR);
	net_interface_ip_address_t *ip_address_ptr =
		PTR_DATA(proto_peer->get_address_id(),
			 net_interface_ip_address_t);
	PRINT_IF_NULL(ip_address_ptr, P_ERR);
	ip_address_ptr->set_address_data(
		ip,
		port,
		net_proto_generate_ip_address_nat_type());
}

id_t_ net_proto::peer::get_self_as_peer(){
	return self_peer_id;
}


id_t_ net_proto::peer::random_peer_id(){
	std::vector<id_t_> proto_peer_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_PROTO_PEER_T);
	std::random_shuffle(
		proto_peer_vector.begin(),
		proto_peer_vector.end());
	for(uint64_t i = 0;i < proto_peer_vector.size();i++){
		if(proto_peer_vector[i] !=
		   net_proto::peer::get_self_as_peer()){
			return proto_peer_vector[i];
		}
	}
	print("no other peers detected, cannot return a valid peer id", P_WARN);
	return ID_BLANK_ID;
}

static bool net_proto_self_reference(
	net_interface_ip_address_t *ip_address_ptr){
	const std::pair<std::vector<uint8_t>, uint8_t> ip_addr =
		ip_address_ptr->get_address();
	switch(ip_addr.second){
	case NET_INTERFACE_IP_ADDRESS_TYPE_IPV4:
		if(ip_addr.first.size() != 4){
			return false;
		}
		// NBO
		return (ip_addr.first[0] == 1 &&
			ip_addr.first[1] == 0 &&
			ip_addr.first[2] == 0 &&
			ip_addr.first[3] == 127) ||
			(ip_addr.first[0] == 0 &&
			 ip_addr.first[1] == 0 &&
			 ip_addr.first[2] == 0 &&
			 ip_addr.first[3] == 0);
	case NET_INTERFACE_IP_ADDRESS_TYPE_IPV6:
		if(ip_addr.first.size() != 16){
			return false;
		}
		return ip_addr.first[0] == 1 &&
			std::vector<uint8_t>(
				ip_addr.first.begin()+1,
				ip_addr.first.end()) ==
			std::vector<uint8_t>(
				15, 0);
	case NET_INTERFACE_IP_ADDRESS_TYPE_DOMAIN:
		return false; // best guess
	default:
		print("unrecognized address type", P_ERR);
	}
	return false;
}

#pragma message("optimal_peer_for_id only searches for matching hash, this isn't sustainable for large-scale deployment")

id_t_ net_proto::peer::optimal_peer_for_id(id_t_ id){
	const std::vector<id_t_> proto_peer_vector =
		ID_TIER_CACHE_GET(TYPE_NET_PROTO_PEER_T);
	const hash_t_ id_hash =
		get_id_hash(id);
	for(uint64_t i = 0;i < proto_peer_vector.size();i++){
		if(id_hash == get_id_hash(proto_peer_vector[i])){
			// wrong, but fail-safe, assumption that there can
			// be only one peer per hash
			return proto_peer_vector[i];
		}
	}
	print("no matching hash found on network, returning blank ID", P_WARN);
	return ID_BLANK_ID;
}

/*
  TODO: should provide a more generic interface to allow getting an IP/radio
  breakdown either in reference to an address, or in reference to a
  peer itself
 */

std::string net_proto::peer::get_breakdown(id_t_ id_){
	id_t_ peer_id = ID_BLANK_ID;
	id_t_ ip_address_id = ID_BLANK_ID;	
	std::string ip_addr;
	uint16_t port = 0;
	net_interface_ip_address_t *ip_address_ptr =
		nullptr;
	if(id_ != ID_BLANK_ID){
		switch(get_id_type(id_)){
		case TYPE_NET_PROTO_PEER_T:
			if(true){	
				peer_id = id_;
				net_proto_peer_t *proto_peer_ptr =
					PTR_DATA(peer_id,
						 net_proto_peer_t);
				if(proto_peer_ptr != nullptr){
					ip_address_id =
						proto_peer_ptr->get_address_id();
					ip_address_ptr =
						PTR_DATA(ip_address_id,
							 net_interface_ip_address_t);
				}
			}
			break;
		case TYPE_NET_INTERFACE_IP_ADDRESS_T:
			ip_address_id =
				id_;
			ip_address_ptr =
				PTR_DATA(ip_address_id,
					 net_interface_ip_address_t);
			break;
		default:
			print("invalid type for get_breakdown", P_WARN);
		}
		if(ip_address_ptr != nullptr){ // GCC optimized out (probably)
			ip_addr =
				net_interface::ip::raw::to_readable(
					ip_address_ptr->get_address());
			port =
				ip_address_ptr->get_port();
		}else{
			print("ip_address_ptr is a nullptr", P_WARN);
			ip_addr = "NOIP";
			port = 0;
		}
	}
	return "(" + id_breakdown(peer_id) + id_breakdown(ip_address_id) + " IP: " + ip_addr + ":" + std::to_string(port) + ")";
}
