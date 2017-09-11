#ifndef ID_TIER_CONTROL_H
#define ID_TIER_CONTROL_H

#define ID_TIER_CONTROL_FLAG_UPDATE_CACHE (1 << 0)

struct id_tier_state_control_t{
public:
	uint64_t flags = 0;
};

#endif
