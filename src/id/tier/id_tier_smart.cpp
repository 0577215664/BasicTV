#include "id_tier.h"
#include "id_tier_smart.h"

// function assumes that only one datum is passed. Sets of multiple
// need to call id_api::transport
std::vector<id_t_> id_tier::operation::smart::add_data(std::vector<std::vector<uint8_t> > data){
	// Right now we just load to memory
	id_tier::operation::add_data_to_state(
		std::vector<id_t_>({id_tier::state_tier::only_state_of_tier(
					0, 0)}),
		data);
}
