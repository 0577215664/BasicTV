#ifndef ID_REQUEST_H
#define ID_REQUEST_H

/*
  I'm moving away from protocol sockets and towards creating a network tier
  integrating in the same statistical tests done for peers as is done for
  disks and other storage systems like that (IPFS, etc.)

  Each id_request_request_t creates a list of id_request_entry_t
 */

#include "../../state.h"
#include "../../math/math.h"
#include "../id.h"

// statistical linkage
#define ID_REQUEST_FORMAT_STAT 1
// match directly based on pubkey hash only
#define ID_REQUEST_FORMAT_HASH 2

struct id_request_tier_entry_t{
private:
	id_t_ tier_id = ID_BLANK_ID;
	uint8_t format = 0;
	math_stat_pval_t p_val = 0;
	uint64_t last_calc_time_micro_s = 0;
public:
	bool operator==(const id_request_tier_entry_t &rhs) const {
		return tier_id == rhs.tier_id &&
		format == rhs.format &&
		p_val == rhs.p_val &&
		last_calc_time_micro_s == rhs.last_calc_time_micro_s;
	}
	bool operator!=(const id_request_tier_entry_t &rhs) const {
		return !(*this == rhs);
	}
	GET_SET_ID_S(tier_id);
	GET_SET_S(p_val, uint16_t);

	void update();
};

struct id_request_entry_t{
private:
	id_t_ request_id = ID_BLANK_ID;
	std::vector<id_request_tier_entry_t> tier_vector;
public:
	id_request_entry_t(id_t_ id_){
		request_id = id_;
	}
	bool operator==(const id_request_entry_t &rhs) const {
		return request_id == rhs.request_id &&
		tier_vector == rhs.tier_vector;
	}
	bool operator!=(const id_request_entry_t &rhs) const {
		return !(*this == rhs);
	}
	GET_SET_ID_S(request_id);
	GET_SET_S(tier_vector, std::vector<id_request_tier_entry_t>);
};

// states are used to optimize statistics by keeping a cache stored locally
struct id_request_request_t{
private:
	std::vector<id_request_entry_t> ids;
public:
	data_id_t id;
	id_request_request_t();
	~id_request_request_t();
};

struct id_request_response_t{
private:
	id_t_ id_request_request_id = ID_BLANK_ID;
	std::vector<std::tuple<std::vector<uint8_t>, std::vector<id_t_> > > payload;
public:
	data_id_t id;
	id_request_response_t();
	~id_request_response_t();
};

#define ID_REQUEST_FORMAT_INIT(format) void id_request_##format##_init(uint8_t type)
#define ID_REQUEST_FORMAT_CLOSE(format) void id_request_##format##_close()
#define ID_REQUEST_FORMAT_REFRESH(format) void id_request_##format##_refresh()

extern void id_request_init();
extern void id_request_loop();
extern void id_request_close();

#endif
