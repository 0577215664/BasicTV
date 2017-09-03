#ifndef ID_TIER_NETWORK_REQUEST_H
#define ID_TIER_NETWORK_REQUEST_H

#include "../../id.h"

struct id_tier_network_simple_request_t{
private:
	std::vector<uint8_t> ids;
public:
	data_id_t id;
	id_tier_network_simple_request_t();
	~id_tier_network_simple_request_t();

	/* GET_SET(ids, std::vector<id_t_>); */
	std::vector<id_t_> get_ids(){return expand_id_set(ids);}
	void set_ids(std::vector<id_t_> ids_){ids = compact_id_set(ids_, false);}
};

struct id_tier_network_simple_response_t{
private:
	std::vector<std::vector<uint8_t> > payload;
public:
	data_id_t id;
	id_tier_network_simple_response_t();
	~id_tier_network_simple_response_t();

	GET_SET(payload, std::vector<std::vector<uint8_t> >);
	FULL_VECTOR_CONTROL(payload, std::vector<uint8_t>);
};

#endif
