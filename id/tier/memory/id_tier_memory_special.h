#ifndef ID_TIER_MEMORY_SPECIAL_H
#define ID_TIER_MEMORY_SPECIAL_H
#include "../../id.h"
#include "../id_tier.h"

extern void id_tier_mem_regen_state_cache();
extern void id_tier_mem_update_state_cache(id_tier_state_t*);

extern void mem_add_id(data_id_t*);
extern void mem_del_id(data_id_t*);
#endif
