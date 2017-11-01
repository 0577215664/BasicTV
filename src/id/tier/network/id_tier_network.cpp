#include "id_tier_network.h"
#include "id_tier_network_meta.h"
#include "id_tier_network_cache.h"
#include "../id_tier.h"
#include "../../id_transport.h"
#include "../../id.h"

#include "../../../net/interface/net_interface_address.h"
#include "../../../net/interface/net_interface_software.h"

#include "../../../net/proto/net_proto.h"

#include "../../../escape.h"
#include "../../../settings.h"

/*
  Net interface API allows defining arbitrary length datagrams, so
  that makes the interface pretty darn nice here
  First byte is an ID:
  REQUEST: compact ID set of all IDs to request
  DATA: response to a request, doesn't need to be tied
  META: metadata associated with other client
*/

#define TIER_GET_STATE_PTR()				\
	id_tier_state_t *tier_state_ptr =		\
		PTR_DATA(state_id,			\
			 id_tier_state_t);		\
	PRINT_IF_NULL(tier_state_ptr, P_UNABLE);	\

#define TIER_GET_NETWORK_PTR()				\
	id_tier_network_t *network_state_ptr =		\
		reinterpret_cast<id_tier_network_t*>(	\
			tier_state_ptr->get_payload());	\
	PRINT_IF_NULL(network_state_ptr, P_UNABLE);	\
	
#define TIER_GET_NETWORK_SOFTDEV_PTR()				\
	net_interface_software_dev_t *software_dev_ptr =	\
		PTR_DATA(network_state_ptr->software_dev_id,	\
			 net_interface_software_dev_t);		\
	PRINT_IF_NULL(software_dev_ptr, P_UNABLE);

#pragma warning("attempt to bind address, software dev, etc together")

ID_TIER_INIT_STATE(network){
	id_tier_state_t *tier_state_ptr =
		new id_tier_state_t;
	id_tier_network_t *network_state_ptr =
		new id_tier_network_t;
	tier_state_ptr->set_payload(
		reinterpret_cast<void*>(network_state_ptr));
	tier_state_ptr->set_medium(
		ID_TIER_MEDIUM_NETWORK);
	tier_state_ptr->set_tier_major(
		ID_TIER_MAJOR_NETWORK);
	tier_state_ptr->set_tier_minor(
		0);
	tier_state_ptr->storage.cache.update_freq.init(
		settings::get_setting_unsigned_def(
			"id_tier_network_cache_refresh_interval", 10*1000*1000),
		0);

	// offload all network config to caller
	return tier_state_ptr->id.get_id();
}

ID_TIER_DEL_STATE(network){
	id_tier_state_t *tier_state_ptr =
		PTR_DATA(state_id,
			 id_tier_state_t);
	if(tier_state_ptr == nullptr){
		print("tried to destroy an already destroyed id_tier_state_t, returning now", P_WARN);
		return;
	}
	id_tier_network_t *network_state_ptr =
		reinterpret_cast<id_tier_network_t*>(
			tier_state_ptr->get_payload());
	if(network_state_ptr == nullptr){
		print("tried to destroy an already destroyed id_tier_network_t, returning now", P_WARN);
		return;
	}
	delete network_state_ptr;
	network_state_ptr = nullptr;
	tier_state_ptr->set_payload(nullptr);
	ID_TIER_DESTROY(tier_state_ptr->id.get_id());
	tier_state_ptr = nullptr;
}

/*
  GET_ID is where all of the socket offloading, processing, requesting, and all
  that nonsense happens. 

  GET_ID is the ONLY function calling ADD_DATA

  ADD_DATA directly packetizes and sends anything passed through it. The net
  interface packetizer sends all the data at once, so make sure any data sent
  to it is complete enough to be interpreted. In sendind everything at once,
  the interface packetizer generates one packet, which makes it easier to
  decipher

  DEL_ID doesn't do anything
  
  Not all data going through here is ID data, so don't pull IDs
 */

ID_TIER_ADD_DATA(network){
	TIER_GET_STATE_PTR();
	TIER_GET_NETWORK_PTR();
	TIER_GET_NETWORK_SOFTDEV_PTR();
	data.insert(
		data.begin(),
		ID_TIER_NETWORK_TYPE_DATA);
	print("adding " + id_breakdown(id_api::raw::fetch_id(data)) + " to data network queue", P_NOTE);
	software_dev_ptr->add_outbound_data(
		data);
}

ID_TIER_DEL_ID(network){
	print("can't delete an ID from another peer", P_ERR);
}

/*
  GET_HINT_ID makes a request in the outbound ledger, sends the data out,
  and records the response ID when it comes back (response ID would refer to
  the request ID)

  TODO: care about endian later
 */

ID_TIER_GET_HINT_ID(network){
	TIER_GET_STATE_PTR();
	TIER_GET_NETWORK_PTR();
	TIER_GET_NETWORK_SOFTDEV_PTR();
	std::vector<uint8_t> data;
	data.insert(
		data.begin(),
		ID_TIER_NETWORK_TYPE_REQUEST);
	data.insert(
		data.begin(),
		reinterpret_cast<uint8_t*>(&(id[0])),
		reinterpret_cast<uint8_t*>(&(id[0]))+sizeof(id_t_));
	print("adding " + id_breakdown(id) + " to request network queue", P_NOTE);
	software_dev_ptr->add_outbound_data(
		data);
}

ID_TIER_GET_ID(network){
	TIER_GET_STATE_PTR();
	TIER_GET_NETWORK_PTR();
	TIER_GET_NETWORK_SOFTDEV_PTR();
	for(uint64_t i = 0;i < network_state_ptr->inbound_buffer.size();i++){
		if(id_api::raw::fetch_id(network_state_ptr->inbound_buffer[i]) == id){
			return network_state_ptr->inbound_buffer[i];
		}
	}
	id_tier_network_get_hint_id(
		state_id, id);
	return std::vector<uint8_t>({});
}

// meta doesn't need any payload associated with it

ID_TIER_UPDATE_CACHE(network){
	TIER_GET_STATE_PTR();
	TIER_GET_NETWORK_PTR();
	TIER_GET_NETWORK_SOFTDEV_PTR();
	try{
		id_tier_network_meta_t meta =
			id_tier_network_meta_gen_standard();
		// TODO: fix this
		meta.macros |= ID_TIER_NETWORK_META_SEND_CACHE_FULL;
		const net_interface_medium_t medium =
			interface_medium_lookup(
				software_dev_ptr->get_medium());
		std::vector<uint8_t> out_data =
			id_tier_network_meta_write(
				meta);
		medium.send(
			software_dev_ptr->get_hardware_dev_id(),
			software_dev_ptr->id.get_id(),
			&out_data);
	}catch(...){}
}

static void id_tier_network_fill_request(
	id_t_ tier_id,
	std::vector<uint8_t> id_set){
	std::vector<id_t_> id_vector =
		expand_id_set(
			id_set);
	std::vector<id_t_> tier_list =
		id_tier::state_tier::optimal_state_vector_of_tier_vector(
			all_tiers);
	for(uint64_t i = 0;i < tier_list.size() && id_vector.size() > 0;i++){
		id_tier::operation::shift_data_to_state(
			tier_list[i],
			tier_id,
			&id_vector);
	}
	// TODO: craft a response for when we don't have an ID
}

static void id_tier_network_network_to_tier(
	id_tier_state_t *tier_state_ptr,
	id_tier_network_state_t *network_state_ptr,
	net_interface_software_dev_t *software_dev_ptr){

	std::vector<std::vector<uint8_t> > net_inbound_buffer =
		software_dev_ptr->get_inbound_data();
	for(uint64_t i = 0;i < net_inbound_buffer.size();i++){
		const uint8_t type =
			net_inbound_buffer[i][0];
		net_inbound_buffer[i].erase(
			net_inbound_buffer[i].begin());
		switch(net_inbound_buffer[i][0]){
		case ID_TIER_NETWORK_TYPE_META:
			// update our version of the metadata
			// TODO: possible DoS vector, punish redundancy
			id_tier_network_meta_read(
				net_inbound_buffer[i],
				&(network_state_ptr->meta));
			break;
		case ID_TIER_NETWORK_TYPE_DATA:
			network_state_ptr->inbound_buffer.push_back(
				net_inbound_buffer[i]);
			break;
		case ID_TIER_NETWORK_TYPE_REQUEST:
			id_tier_network_fill_request(
				tier_state_ptr->id.get_id(),
				net_inbound_buffer[i]);
			break;
		default:
			print("invalid type for protocol datagram", P_WARN);
		}
	}
	ASSERT(!((network_state_ptr->meta.macros & ID_TIER_NETWORK_META_SEND_CACHE_DIFF) &&
		 (network_state_ptr->meta.macros & ID_TIER_NETWORK_META_SEND_CACHE_FULL)), P_WARN);
	
	if(network_state_ptr->meta.macros & ID_TIER_NETWORK_META_SEND_CACHE_DIFF){
		
	}
	if(network_state_ptr->meta.macros & ID_TIER_NETWORK_META_SEND_CACHE_FULL){
		
	}
	software_dev_ptr->set_inbound_data(std::vector<std::vector<uint8_t> >({}));
}

ID_TIER_LOOP(network){
	TIER_GET_STATE_PTR();
	TIER_GET_NETWORK_PTR();
	TIER_GET_NETWORK_SOFTDEV_PTR();
	id_tier_network_network_to_tier(
		tier_state_ptr,
		network_state_ptr,
		software_dev_ptr);
	ID_TIER_LOOP_STANDARD(
		id_tier_network_add_data,
		id_tier_network_get_id);
}
