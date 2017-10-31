#ifndef ID_TIER_NETWORK_H
#define ID_TIER_NETWORK_H

#include "../id_tier.h"
#include "id_tier_network_meta.h"
#include "id_tier_network_cache.h"

/*
  META is just the metadata as a constant length (defined as a macro)
  DATA is the payload in response to a REQUEST (dynamic size)
  REQUEST is a list of IDs (size modulo sizeof(id_t_) == 0)
 */

#define ID_TIER_NETWORK_TYPE_UNDEFINED 0
#define ID_TIER_NETWORK_TYPE_META 1
#define ID_TIER_NETWORK_TYPE_DATA 2
#define ID_TIER_NETWORK_TYPE_REQUEST 3
#define ID_TIER_NETWORK_TYPE_CACHE 4

#define ID_TIER_NETWORK_REQUEST_STATE_SENT 1

struct id_tier_network_request_log_t{
public:
	id_t_ id;
	uint64_t sent_time_micro_s = 0;
	uint8_t state = 0;
};

struct id_tier_network_t{
public:
	id_t_ proto_peer_id = ID_BLANK_ID;
	id_t_ software_dev_id = ID_BLANK_ID;

	id_tier_network_meta_t meta;
	
	std::vector<id_tier_network_request_log_t> request_log;
	std::vector<std::vector<uint8_t> > inbound_buffer;

	id_tier_network_cache_t cache;
	
	GET_SET_ID_S(proto_peer_id);
	GET_SET_ID_S(software_dev_id);
};

typedef id_tier_network_t id_tier_network_state_t; // just for clarity

extern ID_TIER_INIT_STATE(network);
extern ID_TIER_DEL_STATE(network);
extern ID_TIER_LOOP(network);
extern ID_TIER_UPDATE_CACHE(network);

/* extern ID_TIER_ADD_DATA(network); */
/* extern ID_TIER_DEL_ID(network); */
/* extern ID_TIER_GET_ID(network); */

#endif
