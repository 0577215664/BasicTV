#ifndef ID_TIER_CACHE_H
#define ID_TIER_CACHE_H
extern ID_TIER_INIT_STATE(cache);
extern ID_TIER_DEL_STATE(cache);
extern ID_TIER_LOOP(cache);

/* extern ID_TIER_ADD_DATA(cache); */
/* extern ID_TIER_DEL_ID(cache); */
/* extern ID_TIER_GET_ID(cache); */
/* extern ID_TIER_GET_HINT_ID(cache); */
/* extern ID_TIER_UPDATE_CACHE(cache); */

struct id_tier_cache_state_t{
public:
	// ID, mod_inc, etc. are pulled when needed directly
	// from here, and checked for sanity on addition
	std::vector<std::vector<uint8_t> > cache_data;
};
#endif
