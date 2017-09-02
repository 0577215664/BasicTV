#ifndef ID_TIER_DISK_H
#define ID_TIER_DISK_H

#include "../id_tier.h"

struct id_tier_disk_state_t{
public:
	std::vector<uint8_t> path;
};

extern ID_TIER_INIT_STATE(disk);
extern ID_TIER_DEL_STATE(disk);
extern ID_TIER_ADD_DATA(disk);
extern ID_TIER_DEL_ID(disk);
extern ID_TIER_GET_ID(disk);
extern ID_TIER_GET_HINT_ID(disk);
extern ID_TIER_UPDATE_CACHE(disk);
	
#endif

