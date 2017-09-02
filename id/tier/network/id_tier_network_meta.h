#include "../../id.h"
#ifndef ID_TIER_NETWORK_META_H
#define ID_TIER_NETWORK_META_H

typedef id_t_ id_tier_network_meta_id_t;
typedef uint32_t id_tier_network_meta_size_t;
typedef uint8_t id_tier_network_meta_ver_t;
typedef uint8_t id_tier_network_meta_macros_t;
typedef uint8_t id_tier_network_meta_unused_t;

struct id_tier_network_meta_t{
private:
public:
	id_tier_network_meta_id_t peer_id = ID_BLANK_ID;
	id_tier_network_meta_ver_t ver_major = 0;
	id_tier_network_meta_ver_t ver_minor = 0;
	id_tier_network_meta_ver_t ver_patch = 0;
	id_tier_network_meta_macros_t macros = 0;
	id_tier_network_meta_unused_t unused = 0;
};

#define ID_TIER_NETWORK_META_SIZE (sizeof(id_tier_network_meta_id_t)+(3*(sizeof(id_tier_network_meta_ver_t)))+sizeof(id_tier_network_meta_macros_t)+sizeof(id_tier_network_meta_unused_t))

extern std::vector<uint8_t> id_tier_network_meta_write(
        id_tier_network_meta_t data);

extern void id_tier_network_meta_read(
	std::vector<uint8_t> data,
	id_tier_network_meta_t *standard_data);
	
#endif
