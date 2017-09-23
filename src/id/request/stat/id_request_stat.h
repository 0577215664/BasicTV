#ifndef ID_REQUEST_STAT_H
#define ID_REQUEST_STAT_H

#include "../id_request.h"
#include "../../id.h"

// pretty simple right now,

struct id_request_tier_entry_stat_t{
public:
	std::vector<std::pair<hash_t_, uint64_t> > hash_dist;

	uint64_t last_update_micro_s = 0;
	uint64_t update_sleep_micro_s = 0;
};

extern ID_REQUEST_FORMAT_INIT(stat);
extern ID_REQUEST_FORMAT_CLOSE(stat);
extern ID_REQUEST_FORMAT_REFRESH(stat);

#endif
