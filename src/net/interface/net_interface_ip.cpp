#include "net_interface.h"
#include "net_interface_ip.h"
#include "net_interface_tcp.h"
#include "net_interface_hardware.h"
#include "net_interface_software.h"
#include "net_interface_intermediary.h"
#include "net_interface_medium.h"
#include "net_interface_ip_address.h"
#include "net_interface_api.h"
#include "net_interface_helper.h"
#include "net_interface_ip_thread.h"

// state, so allocated in ADD_ADDRESS

/*
  TODO: only works with TCP (but doesn't everything?)
*/

static void hardware_software_address_sanity_check(
	net_interface_hardware_dev_t *hardware_dev_ptr,
	net_interface_software_dev_t *software_dev_ptr,
	net_interface_ip_address_t *ip_address_ptr){
	if(software_dev_ptr == nullptr){
		ASSERT(ip_address_ptr->get_medium() == hardware_dev_ptr->get_medium(), P_ERR);
	}else{
		ASSERT(ip_address_ptr->get_required_intermediary() == 0 ||
		       ip_address_ptr->get_required_intermediary() == software_dev_ptr->get_intermediary(), P_ERR);
		ASSERT(hardware_dev_ptr->get_medium() == software_dev_ptr->get_medium() &&
		       hardware_dev_ptr->get_medium() == ip_address_ptr->get_medium(), P_ERR);
	}
}

// getters and setters for the local-global variables


/*
  The likelihoop that we need to formally call the drop function in IP is
  pretty low, so we just go through and drop the software devices with the
  oldest latest activity
 */

INTERFACE_CALCULATE_MOST_EFFICIENT_DROP(ip){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_ADDR_PTR(address_id);
	hardware_software_address_sanity_check(
		hardware_dev_ptr,
		nullptr,
		ip_address_ptr);
	uint64_t soft_dev_to_remove = 0;
	if(hardware_dev_ptr->get_max_soft_dev() ==
	   hardware_dev_ptr->get_size_soft_dev_list()){
		print("sitting at max_soft_dev limit", P_NOTE);
		soft_dev_to_remove = 1;
	}
	std::vector<id_t_> soft_dev_list =
		hardware_dev_ptr->get_soft_dev_list();
	std::vector<id_t_> retval;
	const uint64_t cur_time_micro_s =
		get_time_microseconds();
	while(retval.size() != soft_dev_to_remove){
		id_t_ preferable_id =
			ID_BLANK_ID;
		uint64_t preferable_diff_micro_s =
			0;
		for(uint64_t i = 0;i < soft_dev_list.size();i++){
			net_interface_software_dev_t *tmp_software_dev_ptr =
				PTR_DATA(soft_dev_list[i],
					 net_interface_software_dev_t);
			CONTINUE_IF_NULL(tmp_software_dev_ptr, P_WARN);
			const uint64_t tmp_diff_micro_s =
				cur_time_micro_s-tmp_software_dev_ptr->get_last_good_inbound_micro_s();
			if(tmp_diff_micro_s > preferable_diff_micro_s &&
			   std::find(retval.begin(), retval.end(), soft_dev_list[i]) == retval.end()){
				preferable_id =
					soft_dev_list[i];
				preferable_diff_micro_s =
					tmp_diff_micro_s;
			}
		}
		retval.push_back(preferable_id);
	}
	return retval;
}

INTERFACE_CALCULATE_MOST_EFFICIENT_TRANSFER(ip){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_ADDR_PTR(address_id);
	/*
	  Until I implement modulation/packet schemes, we are keeping with TCP,
	  and sockets can't be traded between interfaces easily without
	  disconnecting (same for UDP)
	 */
	return {};
}

#pragma message("no attempt at UDP support whatever right now")

/*
  Empty outbound_buffer first, then send the parameter information

  TODO: should probably move to a buffer and loop system to fit in
  with the rest of the program (TCP blocking send)
 */

INTERFACE_SEND(ip){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_SW_PTR(software_dev_id);
	ASSERT(payload->size() != 0, P_ERR);

	// sanity checks for medium, modulation, and encapsulation are done
	// on assignment to address, so they should be fine herex

	int64_t sent_bytes = 0;

	net_interface_medium_ip_ptr_t *working_state = 
		static_cast<net_interface_medium_ip_ptr_t*>(software_dev_ptr->get_state_ptr());
		
	net_interface_medium_packet_t medium_packet =
		medium_packet_lookup(
			software_dev_ptr->get_medium(),
			software_dev_ptr->get_packet_modulation(),
			software_dev_ptr->get_packet_encapsulation());

	software_dev_ptr->add_outbound_data(
		*payload);
	payload->clear();

	std::vector<std::vector<uint8_t> > outbound_data =
		software_dev_ptr->get_outbound_data();
	
	std::vector<std::vector<uint8_t> > packetized;
	for(uint64_t i = 0;i < outbound_data.size();i++){
		const std::vector<std::vector<uint8_t> > packets =
			medium_packet.packetize(
				hardware_dev_id,
				software_dev_id,
				&(outbound_data[i]));
		if(outbound_data[i].size() > 0){
			// shouldn't trip
			print("outbound_data[i].size() > 0, can't pakcetize all data", P_WARN);
		}
		packetized.insert(
			packetized.end(),
			packets.begin(),
			packets.end());
	}
	software_dev_ptr->set_outbound_data(
        	std::vector<std::vector<uint8_t> >({}));
	working_state->send_mutex.lock();
	try{
		working_state->send_buffer.insert(
			working_state->send_buffer.end(),
			packetized.begin(),
			packetized.end());
	}catch(...){
		print("can't add to send buffer in main thread", P_ERR);
	}
	working_state->send_mutex.unlock();
}

INTERFACE_RECV_ALL(ip){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_SW_PTR(software_dev_id);
	std::vector<uint8_t> retval;

	net_interface_medium_ip_ptr_t *working_state =
		reinterpret_cast<net_interface_medium_ip_ptr_t*>(
			software_dev_ptr->get_state_ptr());
	PRINT_IF_NULL(working_state, P_ERR);

	if(working_state->recv_buffer.size() != 0){
		working_state->recv_mutex.lock();
		try{
			software_dev_ptr->add_raw_inbound_data(
				working_state->recv_buffer);
			working_state->recv_buffer.clear();
		}catch(...){
			print("caught exception in transferring recv buffer", P_WARN);
		}
		working_state->recv_mutex.unlock();
		print("inbound buffer is now at " + std::to_string(software_dev_ptr->get_size_inbound_data()), P_WARN);
	}

	net_interface_medium_packet_t medium_packet =
		medium_packet_lookup(
			software_dev_ptr->get_medium(),
			software_dev_ptr->get_packet_modulation(),
			software_dev_ptr->get_packet_encapsulation());
	std::vector<std::vector<uint8_t> > inbound_data_ =
		software_dev_ptr->get_raw_inbound_data();
	std::vector<uint8_t> unpacketized =
		medium_packet.depacketize(
			hardware_dev_id,
			software_dev_id,
			&inbound_data_);
	software_dev_ptr->set_raw_inbound_data(
		inbound_data_);
	if(unpacketized.size() > 0){
		software_dev_ptr->add_inbound_data(
			unpacketized);
	}
}

INTERFACE_ADD_ADDRESS_COST(ip){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_ADDR_PTR(address_id);
	if(hardware_dev_ptr->get_max_soft_dev() > hardware_dev_ptr->get_size_soft_dev_list()){
		return NET_INTERFACE_HARDWARE_ADD_ADDRESS_FREE;
	}else{
		return NET_INTERFACE_HARDWARE_ADD_ADDRESS_DROP;
	}
}

static void create_client_socket(
	net_interface_hardware_dev_t *hardware_dev_ptr,
	net_interface_ip_address_t *ip_address_ptr,
	net_interface_software_dev_t *software_dev_ptr,
	net_interface_medium_ip_ptr_t *working_state){
	switch(software_dev_ptr->get_packet_modulation()){
	case NET_INTERFACE_MEDIUM_PACKET_MODULATION_TCP:
		if(true){
			IPaddress ipaddr;
			const std::pair<std::vector<uint8_t>, uint8_t> address =
				ip_address_ptr->get_address();
			const uint16_t port =
				ip_address_ptr->get_port();
			
			switch(address.second){
			case NET_INTERFACE_IP_ADDRESS_TYPE_IPV4:
				print("connecting to " + net_interface::ip::raw::to_readable(address), P_NOTE);
				ASSERT(address.first.size() == 4, P_ERR);
				ipaddr.host = NBO_32(*reinterpret_cast<const uint32_t*>(address.first.data()));
				ipaddr.port = NBO_16(port);
				break;
			case NET_INTERFACE_IP_ADDRESS_TYPE_DOMAIN:
				SDLNet_ResolveHost(&ipaddr, convert::string::from_bytes(address.first).data(), port);
				break;
			case NET_INTERFACE_IP_ADDRESS_TYPE_IPV6:
				print("IPv6 is not supported yet", P_ERR);
				break;
			default:
				print("undefined IP address type", P_ERR);
			}
			working_state->tcp_socket =
				SDLNet_TCP_Open(&ipaddr);
			if(working_state->tcp_socket == nullptr){
				print("cannot connect to peer: " + SDL_GetError(), P_ERR);
			}
			std::thread recv_thread(net_interface_ip_recv_thread, working_state);
			working_state->recv_thread =
				std::move(recv_thread);
			std::thread send_thread(net_interface_ip_send_thread, working_state);
			working_state->send_thread =
				std::move(send_thread);
		}
		break;
	default:
		print("undefined or unsupported modulation scheme", P_ERR);
	}
}

static void create_server_socket(
	net_interface_hardware_dev_t *hardware_dev_ptr,
	net_interface_ip_address_t *ip_address_ptr,
	net_interface_software_dev_t *software_dev_ptr,
	net_interface_medium_ip_ptr_t *working_state){
	IPaddress ipaddr;
	if(SDLNet_ResolveHost(&ipaddr, nullptr, ip_address_ptr->get_port()) == -1){
		print("couldn't resolve host: " + SDL_GetError(), P_ERR);
	}
	if((working_state->tcp_socket = SDLNet_TCP_Open(&ipaddr)) == nullptr){
		print("couldn't open server socket: " + SDL_GetError(), P_ERR);
	}
}

INTERFACE_ADD_ADDRESS(ip){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_ADDR_PTR(address_id);
	
	net_interface_software_dev_t *software_dev_ptr =
		new net_interface_software_dev_t;
	software_dev_ptr->set_address_id(
		address_id);
	software_dev_ptr->set_packet_modulation(
		NET_INTERFACE_MEDIUM_PACKET_MODULATION_TCP);
	software_dev_ptr->set_packet_encapsulation(
		NET_INTERFACE_MEDIUM_PACKET_ENCAPSULATION_TCP);
	software_dev_ptr->set_inbound_transport_type(
		inbound_transport_rules);
	software_dev_ptr->set_outbound_transport_type(
		outbound_transport_rules);
	
	net_interface::bind::software_to_hardware(
		software_dev_ptr->id.get_id(),
		hardware_dev_id);

	net_interface_medium_ip_ptr_t *working_state =
		new net_interface_medium_ip_ptr_t;
	const bool inbound_enabled =
		(software_dev_ptr->get_inbound_transport_type() & 0x01) == NET_INTERFACE_TRANSPORT_ENABLED;
	const bool outbound_enabled =
		(software_dev_ptr->get_outbound_transport_type() & 0x01) == NET_INTERFACE_TRANSPORT_ENABLED;
	if(inbound_enabled == true && outbound_enabled == true){
		create_client_socket(
			hardware_dev_ptr,
			ip_address_ptr,
			software_dev_ptr,
			working_state);
	}else if(inbound_enabled == true && outbound_enabled == false){
		create_server_socket(
			hardware_dev_ptr,
			ip_address_ptr,
			software_dev_ptr,
			working_state);
	}else{
		// only using vanilla IP, so this is pretty weird
		print("this configuration for IP transports makes no sense", P_ERR);
	}
	ASSERT(working_state->tcp_socket != nullptr, P_ERR);
	software_dev_ptr->set_state_ptr(
		working_state);
	return software_dev_ptr->id.get_id();
}

INTERFACE_ACCEPT(ip){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_SW_PTR(software_dev_id);

	if((software_dev_ptr->get_outbound_transport_type() & 0x01) == NET_INTERFACE_TRANSPORT_ENABLED){
		print("not configured as a server socket, cannot accept connections", P_ERR);
	}
	
	net_interface_medium_ip_ptr_t *working_state =
		reinterpret_cast<net_interface_medium_ip_ptr_t*>(
			software_dev_ptr->get_state_ptr());
	PRINT_IF_NULL(working_state, P_ERR);
	TCPsocket new_socket = SDLNet_TCP_Accept(working_state->tcp_socket);
	if(new_socket != nullptr){
		net_interface_software_dev_t *new_software_dev_ptr =
			new net_interface_software_dev_t;
		net_interface_medium_ip_ptr_t *working_state_new =
			new net_interface_medium_ip_ptr_t;
		working_state_new->tcp_socket = new_socket;
		working_state_new->socket_set = SDLNet_AllocSocketSet(1);

		std::thread recv_thread(net_interface_ip_recv_thread, working_state_new);
		working_state_new->recv_thread =
			std::move(recv_thread);
		std::thread send_thread(net_interface_ip_send_thread, working_state_new);
		working_state_new->send_thread =
			std::move(send_thread);

		// soft_dev_list (hardware), hardware_dev_id, medium (software)
		net_interface::bind::software_to_hardware(
			new_software_dev_ptr->id.get_id(),
			hardware_dev_ptr->id.get_id());
 		
		new_software_dev_ptr->set_inbound_transport_type(
			NET_INTERFACE_TRANSPORT_ENABLED | NET_INTERFACE_TRANSPORT_FLAG_LOSSLESS);
		new_software_dev_ptr->set_outbound_transport_type(
			NET_INTERFACE_TRANSPORT_ENABLED | NET_INTERFACE_TRANSPORT_FLAG_LOSSLESS);
		new_software_dev_ptr->set_packet_modulation(
			NET_INTERFACE_MEDIUM_PACKET_MODULATION_TCP);
		new_software_dev_ptr->set_packet_encapsulation(
			NET_INTERFACE_MEDIUM_PACKET_ENCAPSULATION_TCP);
		
		hardware_dev_ptr->add_soft_dev_list(
			new_software_dev_ptr->id.get_id());
		
		new_software_dev_ptr->set_state_ptr(working_state_new);
		//std::raise(SIGINT);
		return new_software_dev_ptr->id.get_id();
	}
	return ID_BLANK_ID;
}

INTERFACE_DROP(ip){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_SW_PTR(software_dev_id);

	net_interface_medium_ip_ptr_t *working_state =
		reinterpret_cast<net_interface_medium_ip_ptr_t*>(
			software_dev_ptr->get_state_ptr());

	PRINT_IF_NULL(working_state, P_ERR);
	working_state->recv_running = false;
	working_state->recv_thread.join();
	software_dev_ptr->add_inbound_data(
		working_state->recv_buffer);
	working_state->recv_buffer.clear();
	delete working_state;
	working_state = nullptr;

	// dropping doesn't mean we delete the software_dev, just delete the
	// state it is bound to, probably to make sure all queues are flused
	// to pull more data
}
