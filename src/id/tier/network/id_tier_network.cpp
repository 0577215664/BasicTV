#include "id_tier_network.h"
#include "id_tier_network_meta.h"
#include "id_tier_network_cache.h"
#include "id_tier_network_request.h"
#include "../id_tier.h"
#include "../../id_transport.h"
#include "../../id.h"

#include "../../../net/interface/net_interface_address.h"
#include "../../../net/interface/net_interface_software.h"

#include "../../../net/proto/net_proto.h"

#include "../../../escape.h"
#include "../../../settings.h"

/*
  All peer to peer ID networking is defined in ID tiers, each instance
  of the network tier is one peer on the network. Peers with identical
  sets of information they have (and similar results in producing) are
  associated with each other. 

  ID tier states for networking can't be networked themselves (interferes
  with pointer system with the abstract interface). ID tier states are
  created from net_proto_peer_t information automatically, and flags in
  the payload data are sent out for any direct peer to peer queries
  (directory of data to populate the cache automatically).

  There is only one address bound to an ID tier, to help prevent
  discrepencies with certain addresses not responding with the same
  data.

  TODO: allow for exportind id_tier_state_t to disk, so we can have a 
  peer indexes survive reboots.
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
		ID_TIER_NETWORK_TYPE_ID);
	print("adding " + id_breakdown(id_api::raw::fetch_id(data)) + " to network queue", P_NOTE);
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
 */

ID_TIER_GET_HINT_ID(network){
	TIER_GET_STATE_PTR();
	TIER_GET_NETWORK_PTR();

	id_tier_network_ledger_entry_t outbound_entry;
	id_tier_network_simple_request_t *simple_request_ptr =
		new id_tier_network_simple_request_t;
	simple_request_ptr->set_ids(
		std::vector<id_t_>({id})); // TODO: scale up beyond one ID per request
	id_tier_network_add_data(
		state_id,
		simple_request_ptr->id.export_data(
			ID_EXTRA_ENCRYPT & ID_EXTRA_COMPRESS));
	outbound_entry.simple_request_id =
		simple_request_ptr->id.get_id();
	outbound_entry.request_time_micro_s =
		get_time_microseconds();
	network_state_ptr->outbound_ledger.push_back(
		outbound_entry);
}

static std::vector<uint8_t> id_tier_network_write_interface_packet(
	std::vector<std::vector<uint8_t> > data){
	id_tier_network_meta_t meta;
	meta.peer_id = net_proto::peer::get_self_as_peer();
	meta.ver_major = VERSION_MAJOR;
	meta.ver_minor = VERSION_MINOR;
	meta.ver_patch = VERSION_REVISION;
	meta.macros = 0;
	meta.unused = 0;
	std::vector<uint8_t> retval =
 		id_tier_network_meta_write(
			meta);
	for(uint64_t i = 0;i < data.size();i++){
		export_dynamic_size_payload(
			&retval,
			data[i]);
	}
	return retval;
}
/*
  The transmission order doesn't actually matter, because the network
  interface only ensures that data arrives in order inside of a packet
  returned (an element in the inbound_buffer vector).

  The actual transmission format is something like this:
  1. The first ID_TIER_NETWORK_META_SIZE bytes are the network metadata.
  The metadata itself uses a custom exporter so it can easily be
  interpreted by future versions, so the size is static (as all elements
  are as well).

  2. The rest of the packet is just an escaped list of ID data that we
  read in. 

  If this is a id_tier_request_t, then we catalog it inside
  the outbound ledger as a request, and let another part of the code
  deal with crafting a response and responding (and cataloging it as
  well).
*/

// returns what should be sent back to the peer
static std::vector<std::vector<uint8_t> > id_tier_network_read_interface_packet_macros(
	uint8_t macros){
	std::vector<std::vector<uint8_t> > retval;
	if(macros & ID_TIER_NETWORK_META_SEND_CACHE){
		retval.push_back(
			id_tier_network_cache_create_serialize());
	}
	return retval;
}

static std::vector<std::vector<uint8_t> > id_tier_network_read_interface_packet(
	std::vector<uint8_t> packet){
	std::vector<std::vector<uint8_t> > retval;
	id_tier_network_meta_t meta;
	id_tier_network_meta_read(
		std::vector<uint8_t>(
			packet.begin(),
			packet.begin()+ID_TIER_NETWORK_META_SIZE),
		&meta);
	packet.erase(
		packet.begin(),
		packet.begin()+ID_TIER_NETWORK_META_SIZE);
	while(packet.size() > 0){
		const std::vector<uint8_t> tmp =
			import_dynamic_size_payload(
				&packet);
		if(tmp.size() != 0){
			retval.push_back(
				tmp);
		}
	}
	return retval;
}

ID_TIER_GET_ID(network){
	print("requesting " + id_breakdown(id) + " from network tier", P_NOTE);
	id_tier_state_t *tier_state_ptr =
		PTR_DATA(state_id,
			 id_tier_state_t);
	PRINT_IF_NULL(tier_state_ptr, P_ERR);
	id_tier_network_t *network_state_ptr =
		reinterpret_cast<id_tier_network_t*>(
			tier_state_ptr->get_payload());
	PRINT_IF_NULL(network_state_ptr, P_ERR);

	net_proto_peer_t *proto_peer_ptr =
		PTR_DATA(network_state_ptr->get_proto_peer_id(),
			 net_proto_peer_t);
	PRINT_IF_NULL(proto_peer_ptr, P_ERR);
	
	net_interface_address_t *address_ptr =
		PTR_DATA(proto_peer_ptr->get_address_id(),
			 net_interface_address_t);
	PRINT_IF_NULL(address_ptr, P_ERR);
	net_interface_software_dev_t *software_dev_ptr =
		PTR_DATA(network_state_ptr->software_dev_id,
			 net_interface_software_dev_t);
	PRINT_IF_NULL(software_dev_ptr, P_ERR);
	
	const std::vector<std::vector<uint8_t> > *inbound_buffer =
		software_dev_ptr->get_const_ptr_inbound_data();
	for(uint64_t i = 0;i < inbound_buffer->size();i++){
		id_tier_network_read_interface_packet(
			(*inbound_buffer)[i]);
	}
	for(uint64_t i = 0;i < network_state_ptr->outbound_ledger.size();i++){
		id_tier_network_simple_response_t *simple_response_ptr =
			PTR_DATA(network_state_ptr->outbound_ledger[i].simple_response_id,
				 id_tier_network_simple_response_t);
		CONTINUE_IF_NULL(simple_response_ptr, P_NOTE);
		const std::vector<std::vector<uint8_t> > *payload_ptr =
			simple_response_ptr->get_const_ptr_payload();
		for(uint64_t c = 0;c < payload_ptr->size();c++){
			if(id_api::raw::fetch_id((*payload_ptr)[c]) == id){
				return (*payload_ptr)[c];
			}
		}
	}
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
		meta.macros |= ID_TIER_NETWORK_META_SEND_CACHE;
		const net_interface_medium_t medium =
			interface_medium_lookup(
				net_interface::medium::from_address(
					software_dev_ptr->get_address_id()));
		std::vector<uint8_t> out_data =
			id_tier_network_meta_write(
				meta);
		medium.send(
			software_dev_ptr->get_hardware_dev_id(),
			software_dev_ptr->id.get_id(),
			&out_data);
	}catch(...){}
}

ID_TIER_LOOP(network){
	id_tier_state_t *tier_state_ptr =
		PTR_DATA(state_id,
			 id_tier_state_t);
	PRINT_IF_NULL(tier_state_ptr, P_ERR);
	ID_TIER_LOOP_STANDARD(
		id_tier_network_add_data,
		id_tier_network_get_id);
}
