#ifndef ID_TIER_NETWORK_H
#define ID_TIER_NETWORK_H

#include "../id_tier.h"
#include "id_tier_network_meta.h"
#include "id_tier_network_request.h"

struct id_tier_network_ledger_entry_t{
public:
	id_t_ simple_request_id = ID_BLANK_ID;
	id_t_ simple_response_id = ID_BLANK_ID;

	// timing information is only accurate with a high iteration
	// rate, otherwise it's just sitting in a buffer
	uint64_t request_time_micro_s = 0;
	uint64_t response_time_micro_s = 0;
	
	id_tier_network_meta_t meta;
	bool complete = false;
	uint8_t flags = 0;
};

/*
  There is a 1:1 mapping between id_tier_network_ts and
  net_proto_peer_t data, so the id_tier_network_t isn't destroyed
  when all connections are lost. 

  This is nice for long term statistics gathering, since we can (sometime in
  the future) export the index data reported by them and our own records
  (confirming that they have ID X, Y, Z, etc.) so we don't have to redownload
  that information.
 */

struct id_tier_network_t{
public:
	id_t_ address_id = ID_BLANK_ID;
	id_t_ software_dev_id = ID_BLANK_ID;
	
	// request ID and response ID
	/*
	  OUTBOUND, request originates from myself going outward:
	  simple_request_id is my hash
	  simple_response_id isn't my hash

	  INBOUND, request originates from somebody else, coming inward:
	  simple_request_id isn't my hash
	  simple_response_id is my hash
	*/
	std::vector<id_tier_network_ledger_entry_t> outbound_ledger;
	std::vector<id_tier_network_ledger_entry_t> inbound_ledger;

};


extern ID_TIER_INIT_STATE(network);
extern ID_TIER_DEL_STATE(network);
extern ID_TIER_LOOP(network);

/* extern ID_TIER_ADD_DATA(network); */
/* extern ID_TIER_DEL_ID(network); */
/* extern ID_TIER_GET_ID(network); */
/* extern ID_TIER_UPDATE_CACHE(network); */

#endif
