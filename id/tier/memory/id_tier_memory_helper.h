#ifndef ID_TIER_MEMORY_HELPER_H
#define ID_TIER_MEMORY_HELPER_H

#include "../../id.h"

/*
  All id_tier_state_t have to stay in RAM to work properly, and we need to have
  special functions that can be used inside of Tier 0 (RAM) functions to prevent
  deadlocks and the like. These functions are any functions that Tier 0 would
  need to use because of the nature of it being directly in memory
 */

namespace mem_helper{
	std::vector<data_id_t*> get_data_id_vector();
	namespace lookup{
		data_id_t *id(id_t_ id_);
		void *ptr(id_t_ id_);
		std::vector<id_tier_state_t*> tier_state(
			std::vector<std::pair<uint8_t, uint8_t> > tier_vector);
	};
};


#endif
