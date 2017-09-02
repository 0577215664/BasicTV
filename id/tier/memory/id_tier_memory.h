#ifndef ID_TIER_MEMORY_H
#define ID_TIER_MEMORY_H

#include "../id_tier.h"

#include <algorithm>

extern ID_TIER_INIT_STATE(mem);
extern ID_TIER_DEL_STATE(mem);
extern ID_TIER_ADD_DATA(mem);
extern ID_TIER_DEL_ID(mem);
extern ID_TIER_GET_ID(mem);
extern ID_TIER_GET_HINT_ID(mem);
extern ID_TIER_UPDATE_CACHE(mem);

// init/bootstrap only (other uses internally, only use externally)
struct id_tier_memory_state_t{
};
#endif
