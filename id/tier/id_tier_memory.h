#ifndef ID_TIER_MEMORY_H
#define ID_TIER_MEMORY_H

#include "id_tier.h"

#include <algorithm>

extern ID_TIER_INIT_STATE(mem);
extern ID_TIER_DEL_STATE(mem);
extern ID_TIER_ADD_DATA(mem);
extern ID_TIER_DEL_ID(mem);
extern ID_TIER_GET_ID(mem);
extern ID_TIER_GET_ID_MOD_INC(mem);

// init/bootstrap only (other uses internally, only use externally)
extern id_tier_state_t* mem_tier_state_lookup(uint8_t, uint8_t);
extern void id_tier_mem_regen_state_cache();
extern void id_tier_mem_update_state_cache(id_tier_state_t*);


extern void mem_add_id(data_id_t*);
extern void mem_del_id(data_id_t*);

// only used in init, plan to keep it that way
extern std::vector<data_id_t*> mem_get_data_id_vector();

struct id_tier_memory_state_t{
};
#endif
