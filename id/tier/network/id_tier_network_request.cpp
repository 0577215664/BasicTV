#include "id_tier_network_request.h"

id_tier_network_simple_request_t::id_tier_network_simple_request_t() : id(this, TYPE_ID_TIER_NETWORK_SIMPLE_REQUEST_T){
	id.add_data_one_byte_vector(&ids, ~0, public_ruleset);
}

id_tier_network_simple_request_t::~id_tier_network_simple_request_t(){
}

id_tier_network_simple_response_t::id_tier_network_simple_response_t() : id(this, TYPE_ID_TIER_NETWORK_SIMPLE_RESPONSE_T){
	id.add_data_one_byte_vector_vector(&payload, ~0, ~0, public_ruleset);
}

id_tier_network_simple_response_t::~id_tier_network_simple_response_t(){
}
