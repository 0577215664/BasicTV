#include "net_interface.h"
#include "net_interface_ip.h"
#include "net_interface_tcp.h"

#include "net_interface_hardware.h"
#include "net_interface_software.h"
#include "net_interface_address.h"

net_interface_medium_t medium_array[NET_INTERFACE_MEDIUM_COUNT] =
{
	net_interface_medium_t(
	        net_interface_ip_add_address_cost,
		net_interface_ip_add_address,
		net_interface_ip_calculate_most_efficient_drop,
		net_interface_ip_calculate_most_efficient_transfer,
		net_interface_ip_send,
		net_interface_ip_recv_all,
		net_interface_ip_accept,
		net_interface_ip_drop)
};

net_interface_medium_t interface_medium_lookup(uint8_t medium){
	ASSERT(medium != NET_INTERFACE_MEDIUM_UNDEFINED, P_ERR);
	return medium_array[medium-1];
}

static void net_interface_create_hardware_dev(){
	net_interface_hardware_dev_t *hardware_dev_ptr =
		new net_interface_hardware_dev_t;
	hardware_dev_ptr->set_max_soft_dev(512);
	hardware_dev_ptr->set_outbound_transport_type(
		NET_INTERFACE_TRANSPORT_ENABLED);
	hardware_dev_ptr->set_inbound_transport_type(
		NET_INTERFACE_TRANSPORT_ENABLED);
	hardware_dev_ptr->set_medium(
		NET_INTERFACE_MEDIUM_IP);
}

void net_interface_init(){
	net_interface_create_hardware_dev();
}

void net_interface_loop(){
	// accepting is done in net_proto for simplicity
	
}

void net_interface_close(){
}

