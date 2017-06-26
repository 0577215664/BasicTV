#include "net_interface_ip_address.h"

net_interface_ip_address_t::net_interface_ip_address_t() : id(this, TYPE_NET_INTERFACE_IP_ADDRESS_T){
	list_virtual_data(&id);
	id.add_data_one_byte_vector(&address, 1024); // beyond sane
	ADD_DATA(address_type);
	ADD_DATA(nat_type);
}

net_interface_ip_address_t::~net_interface_ip_address_t(){
}

void net_interface_ip_address_t::set_address_data(
	std::string address_,
	uint16_t port_,
	uint8_t nat_type_){
	std::pair<std::vector<uint8_t>, uint8_t> address_tmp =
		net_interface::ip::readable::to_raw(address_);
	address = address_tmp.first;
	address_type = address_tmp.second;
	if(port_ != 58486){
		std::raise(SIGINT);
	}
	port = port_;
	nat_type = nat_type_;
}

#pragma message("net_interface_ip_address_t::get_address_resolvable() isn't implemented")

std::pair<std::vector<uint8_t>, uint8_t> net_interface_ip_address_t::get_address(){
	return std::make_pair(
		address, address_type);
}
