#include "net_interface.h"
#include "net_interface_medium.h"
#include "net_interface_packet.h"
#include "net_interface_address.h"

#pragma message("times for availability should be set with a conversion function from HH:MM:SS to offsets from midnight UTC")

void net_interface_address_t::set_medium_modulation_encapsulation(
	uint8_t medium_,
	uint8_t modulation_,
	uint8_t encapsulation_){
	sanity_check_medium_modulation_encapsulation(
		medium_,
		modulation_,
		encapsulation_);
	medium = medium_;
	packet_modulation = modulation_;
	packet_encapsulation = encapsulation_;
}

void net_interface_address_t::list_virtual_data(data_id_t *id_){
	id = id_;
	ADD_DATA_PTR(first_time_micro_s, mem_ruleset);
	ADD_DATA_PTR(end_to_start_micro_s, mem_ruleset);
	ADD_DATA_PTR(start_available_micro_s, mem_ruleset);
	ADD_DATA_PTR(end_available_micro_s, mem_ruleset);

	id->add_data_one_byte_vector(&latitude, ~0, mem_ruleset);
	id->add_data_one_byte_vector(&longitude, ~0, mem_ruleset);

	ADD_DATA_PTR(packet_modulation, mem_ruleset);
	ADD_DATA_PTR(packet_encapsulation, mem_ruleset);

	ADD_DATA_PTR(required_intermediary, mem_ruleset);
	ADD_DATA_PTR(last_attempted_connect_time, mem_ruleset);
}

