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
		net_interface_ip_accept)
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
	std::vector<id_t_> software_dev_list =
		ID_TIER_CACHE_GET(
			TYPE_NET_INTERFACE_SOFTWARE_DEV_T);
	for(uint64_t i = 0;i < software_dev_list.size();i++){
		net_interface_software_dev_t *software_dev_ptr =
			PTR_DATA(software_dev_list[i],
				 net_interface_software_dev_t);
		CONTINUE_IF_NULL(software_dev_ptr, P_WARN);
		net_interface_medium_t medium =
			interface_medium_lookup(
				software_dev_ptr->get_medium());
		// what exactly should we do with new sockets, let them be?
		medium.accept(
			software_dev_ptr->get_hardware_dev_id(),
			software_dev_ptr->id.get_id());
	}
}

void net_interface_close(){
}

