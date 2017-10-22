#include "net_interface.h"
#include "net_interface_ip.h"
#include "net_interface_tcp.h"
#include "net_interface_hardware.h"
#include "net_interface_software.h"
#include "net_interface_intermediary.h"
#include "net_interface_medium.h"
#include "net_interface_ip_address.h"

#include "../net.h"

net_interface_hardware_dev_t::net_interface_hardware_dev_t() : id(this, TYPE_NET_INTERFACE_HARDWARE_DEV_T){
	ADD_DATA(max_soft_dev, mem_ruleset);
	ADD_DATA(outbound_transport_type, mem_ruleset);
	ADD_DATA(inbound_transport_type, mem_ruleset);
	ADD_DATA(medium, mem_ruleset);

	id.add_data_one_byte_vector(&soft_dev_list, ~0, mem_ruleset);
	id.add_data_id(&inbound_throughput_number_set_id, 1, mem_ruleset);
	id.add_data_id(&outbound_throughput_number_set_id, 1, mem_ruleset);
}

net_interface_hardware_dev_t::~net_interface_hardware_dev_t(){
}


