#include "../../main.h"
#include "../../util.h"
#include "../../lock.h"
#include "../../settings.h"
#include "../net.h"
#include "../net_socket.h"
#include "../../id/id_api.h"

#include "../../id/tier/network/id_tier_network.h" 

#include "net_proto.h"
#include "net_proto_api.h"

/*
  NEW IDEA:
  We can mark all of our own net_proto_peer_ts as non-exportable
  but networkable. Other people can save ours, as they are not
  configured to be non-exportable (non-exporability and
  non-networkability only apply to local version, once exported
  they are reset).
 */

static id_t_ net_proto_init_new_peer(){
	const uint16_t tmp_port =
		settings::get_setting_unsigned_def(
			"net_interface_ip_tcp_port",
			58486);
	std::string ip_addr =
		settings::get_setting(
			"net_interface_ip_hostname");
	if(ip_addr == ""){
		ip_addr = net_get_ip();
	}else{
		print("assuming the hostname of " + ip_addr, P_NOTE);
	}
	P_V(tmp_port, P_NOTE);
	P_V_S(ip_addr, P_NOTE);
	net_proto_peer_t *proto_peer_ptr =
		new net_proto_peer_t;
	proto_peer_ptr->id.set_most_liberal_rules(
		data_id_transport_rules_t(
			all_tiers,
			all_intermediaries));
	net_interface_ip_address_t *ip_address_ptr =
		new net_interface_ip_address_t;
	ip_address_ptr->id.set_most_liberal_rules(
		data_id_transport_rules_t(
			all_tiers,
			all_intermediaries));
	ip_address_ptr->set_medium_modulation_encapsulation(
		NET_INTERFACE_MEDIUM_IP,
		NET_INTERFACE_MEDIUM_PACKET_MODULATION_TCP,
		NET_INTERFACE_MEDIUM_PACKET_ENCAPSULATION_TCP);
	ip_address_ptr->set_address_data(
		ip_addr,
		tmp_port,
		NET_INTERFACE_IP_ADDRESS_NAT_TYPE_NONE);
	proto_peer_ptr->set_address_id(
		ip_address_ptr->id.get_id());
	return proto_peer_ptr->id.get_id();
}

static void net_proto_init_self_peer(){
	if(net_proto::peer::get_self_as_peer() != ID_BLANK_ID){
		print("We already have local peer data (at init), where did it come from?", P_ERR);
	}
	if(settings::get_setting("net_interface_ip_tcp_enabled") == "true"){
		std::vector<id_t_> proto_peer_id;
		std::vector<id_t_> proto_peer_vector =
			ID_TIER_CACHE_GET(
				TYPE_NET_PROTO_PEER_T);
		for(uint64_t i = 0;i < proto_peer_vector.size();i++){
			if(PTR_ID(proto_peer_vector[i], net_proto_peer_t) == nullptr){
				// no use to us if we can't access it now
				ID_TIER_DESTROY(proto_peer_vector[i]);
				continue;
			}
			if(unlikely(get_id_hash(proto_peer_vector[i]) ==
				    get_id_hash(production_priv_key_id))){
				proto_peer_id.push_back(
					proto_peer_vector[i]);
			}
		}
		if(proto_peer_id.size() == 0){
			print("my pre-existing proto peer info isn't here, making a new set", P_NOTE);
			net_proto::peer::set_self_peer_id(
				net_proto_init_new_peer());
		}else if(proto_peer_id.size() == 1){
			print("reading in my only pre-existing proto peer info", P_NOTE);
			net_proto::peer::set_self_peer_id(
				proto_peer_id[0]);
		}else{
			print("we have more than one net_proto_peer_t, wiping all of them and starting over", P_WARN);
			for(uint64_t i = 0;i < proto_peer_id.size();i++){
				ID_TIER_DESTROY(proto_peer_id[i]);
			}
			net_proto::peer::set_self_peer_id(
				net_proto_init_new_peer());
		}
	}
	if(settings::get_setting("net_interface_ip_udp_enabled") == "true"){
		print("we have no UDP support, disable it in the settings", P_ERR);
	}
}

/*
  Creates a bootstrap node if it doesn't already exist

  Forces no encryption (generated locally)
 */

static std::vector<std::pair<std::string, uint16_t> > bootstrap_nodes;

static void net_proto_verify_bootstrap_nodes(){
	std::vector<id_t_> peer_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_PROTO_PEER_T);
	try{
		const std::string custom_bootstrap_ip =
			settings::get_setting(
				"net_proto_ip_tcp_bootstrap_ip");
		if(custom_bootstrap_ip != ""){
			print("adding custom bootstrap ip from settings", P_NOTE);
			bootstrap_nodes.push_back(
				std::make_pair(
					custom_bootstrap_ip,
					std::stoi(
						settings::get_setting(
							"net_proto_ip_tcp_bootstrap_port"))));
		}
	}catch(...){
		print("no custom bootstrap node specified", P_NOTE);
	}
	std::vector<std::pair<std::string, uint16_t> > nodes_to_connect;
	if(bootstrap_nodes.size() > 0){
		nodes_to_connect = 
			std::vector<std::pair<std::string, uint16_t> >(
				bootstrap_nodes.begin(),
				bootstrap_nodes.end());
	}
	print("attempting to read in " + std::to_string(nodes_to_connect.size()) + " bootstrap nodes", P_DEBUG);
	for(uint64_t i = 0;i < peer_vector.size();i++){
		net_proto_peer_t *tmp_proto_peer =
			PTR_DATA(peer_vector[i],
				 net_proto_peer_t);
		CONTINUE_IF_NULL(tmp_proto_peer, P_WARN);
		net_interface_ip_address_t *tmp_ip_address_ptr =
			PTR_DATA(tmp_proto_peer->get_address_id(),
				 net_interface_ip_address_t);
		CONTINUE_IF_NULL(tmp_ip_address_ptr, P_WARN);
		const std::vector<uint8_t> ip_addr =
			tmp_ip_address_ptr->get_address().first;
		const uint16_t port =
			tmp_ip_address_ptr->get_port();
		try{
			auto bootstrap_iter =
				std::find_if(
					nodes_to_connect.begin(),
					nodes_to_connect.end(),
					[&ip_addr, &port](std::pair<std::string, uint16_t> const& elem){
						return ip_addr == net_interface::ip::readable::to_raw(elem.first).first &&
						port == elem.second; // always not in NBO
					});
			if(bootstrap_iter != nodes_to_connect.end()){
				// remove duplicates, prefer encrypted version
				P_V_S(net_interface::ip::raw::to_readable(
					      std::make_pair(
						      ip_addr, NET_INTERFACE_IP_ADDRESS_TYPE_IPV4)), P_SPAM);
				P_V(port, P_SPAM);
				print("erasing obsolete bootstrap information", P_SPAM);
				nodes_to_connect.erase(
					bootstrap_iter);
				continue;
			}
		}catch(...){}
	}
	for(uint64_t i = 0;i < nodes_to_connect.size();i++){
		net_proto_peer_t *proto_peer_ptr =
			new net_proto_peer_t;
		net_interface_ip_address_t *ip_address_ptr =
			new net_interface_ip_address_t;
		ip_address_ptr->set_medium_modulation_encapsulation(
			NET_INTERFACE_MEDIUM_IP,
			NET_INTERFACE_MEDIUM_PACKET_MODULATION_TCP,
			NET_INTERFACE_MEDIUM_PACKET_ENCAPSULATION_TCP);
		ip_address_ptr->set_address_data(
			nodes_to_connect[i].first,
			nodes_to_connect[i].second,
			NET_INTERFACE_IP_ADDRESS_NAT_TYPE_NONE);
		proto_peer_ptr->set_address_id(
			ip_address_ptr->id.get_id());
		proto_peer_ptr->id.set_most_liberal_rules(
			data_id_transport_rules_t(
				all_mem_cache,
				{0})); // either empty or zero
		ip_address_ptr->id.set_most_liberal_rules(
			data_id_transport_rules_t(
				all_mem_cache,
				{0})); // either empty or zero
		// no harm in assuming port is open
		// WRONG_KEY forces no encryption
		print("created peer with IP " + nodes_to_connect[i].first +
		      " and port " + std::to_string(nodes_to_connect[i].second),
		      P_NOTE);
	}
	const uint64_t start_node_count =
		ID_TIER_CACHE_GET(
			"net_proto_peer_t").size()-1; // don't count ourselves
	if(start_node_count == 0){
		// only OK if i'm connecting with another peer first
		print("no peers exist whatsoever, I better know what i'm doing", P_WARN);
	}
	print("starting with " + std::to_string(start_node_count) + " unique nodes", P_NOTE);
}

static void net_proto_init_proxy(){
	if(settings::get_setting("socks_enable") == "true"){
		try{
			std::string socks_proxy_ip = settings::get_setting("socks_proxy_ip");
			uint16_t socks_proxy_port =
				std::stoi(settings::get_setting("socks_proxy_port"));
			if(socks_proxy_ip == ""){
				throw std::runtime_error("");
			}
			print("need to implement SOCKS with sockets", P_ERR);
			net_proxy_t *proxy_ptr =
				new net_proxy_t;
			proxy_ptr->set_net_ip(
				socks_proxy_ip,
				socks_proxy_port);
			print("added a proxy of " + socks_proxy_ip + ":" + std::to_string(socks_proxy_port), P_NOTE);
		}catch(std::exception e){
			const bool strict =
				settings::get_setting("socks_strict") == "true";
	 		print("unable to configure SOCKS",
			      (strict) ? P_ERR : P_NOTE);
	 	}
	}else{
	 	print("SOCKS has been disabled", P_NOTE);
	}
}

void net_proto_init(){
	net_proto_init_self_peer();	
	net_proto_init_proxy();
	net_proto_verify_bootstrap_nodes();
}

void net_proto_close(){
	// doesn't do anything, GC takes care of all data types
	// All data types should destroy any internal data
}

/*
  Right now it just binds all new net_proto_peer_t's to an ID tier instance,
  since ID tier states aren't networkable because of instance-specific state
  pointers
 */

static std::vector<id_t_> net_proto_loop_peers_from_tiers(){
	std::vector<id_t_> retval;
	std::vector<id_t_> tier_vector =
		ID_TIER_CACHE_GET(
			TYPE_ID_TIER_STATE_T);
	// Whenever we do lookups for a certain ID state type, always check the
	// medium, and don't rely on the tier major/minor matching with the
	// medium we want (could leave out a lot)
	for(uint64_t i = 0;i < tier_vector.size();i++){
		id_tier_state_t *tier_state_ptr =
			PTR_DATA(tier_vector[i],
				 id_tier_state_t);
		CONTINUE_IF_NULL(tier_state_ptr, P_WARN);
		if(tier_state_ptr->get_medium() != ID_TIER_MEDIUM_NETWORK){
			continue;
		}
		id_tier_network_state_t *network_state_ptr =
			reinterpret_cast<id_tier_network_state_t*>(
				tier_state_ptr->get_payload());
		CONTINUE_IF_NULL(network_state_ptr, P_WARN);
		retval.push_back(
			network_state_ptr->get_address_id());
	}
	return retval;
}

/*
  TODO: define the network protocol used as a tier state (this is gonna get
  really interesting)
 */

// bind all network peers to a ID network tier state
// request the 

static void net_proto_loop_bind_peers(){
	std::vector<id_t_> old_peer_ids =
		net_proto_loop_peers_from_tiers();
	std::vector<id_t_> all_peer_ids =
		ID_TIER_CACHE_GET(
			TYPE_NET_PROTO_PEER_T);
	for(uint64_t a = 0;a < all_peer_ids.size();a++){
		for(uint64_t b = 0;b < old_peer_ids.size();b++){
			if(all_peer_ids[b] == old_peer_ids[a] ||
			   all_peer_ids[a] == net_proto::peer::get_self_as_peer()){
				all_peer_ids.erase(
					all_peer_ids.begin()+a);
				a--;
				continue;
			}
		}
	}
	id_tier_medium_t network_medium =
		id_tier::get_medium(
			ID_TIER_MEDIUM_NETWORK);
	for(uint64_t i = 0;i < all_peer_ids.size();i++){
		id_tier_state_t *tier_state_ptr =
			PTR_DATA(network_medium.init_state(),
				 id_tier_state_t);
		tier_state_ptr->set_medium(
			ID_TIER_MEDIUM_NETWORK);
		tier_state_ptr->set_tier_major(
			ID_TIER_MAJOR_NETWORK);
		tier_state_ptr->set_tier_minor(
			0);
		id_tier_network_state_t *network_state_ptr =
			reinterpret_cast<id_tier_network_state_t*>(
				tier_state_ptr->get_payload());
		PRINT_IF_NULL(network_state_ptr, P_ERR);
		network_state_ptr->set_address_id(
			all_peer_ids[i]);
		// TODO: perhaps create optimized paths by creating whitelists of
		// net_proto_peer_ts (they don't have the computing power to
		// handle decryption, or don't bother since it is trusted and
		// isn't able to relay any information)
		tier_state_ptr->storage.set_extras(
			{ID_EXTRA_ENCRYPT & ID_EXTRA_COMPRESS});
	}
}

void net_proto_loop(){
	//net_proto_loop_bind_peers();
	//net_proto_loop_routine_requests();
}
