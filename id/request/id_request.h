#ifndef ID_REQUEST_H
#define ID_REQUEST_H

/*
  I'm moving away from protocol sockets and towards creating a network tier
  integrating in the same statistical tests done for peers as is done for
  disks and other storage systems like that (IPFS, etc.)

  Each id_request_request_t creates a list of id_request_entry_t

  PLEASE NOTE: id_request_request_t and id_request_response_t are meant for
  high level use. Actual requesting and responding (i.e. peers on a network)
  will depend on something lower level, since the majority of instances where
  id_request_request_t and id_request_response_t are used involve high level
  statistics and meta-management of sorts.
 */

#include "../../state.h"
#include "../../math/math.h"
#include "../id.h"

// statistical linkage
#define ID_REQUEST_FORMAT_STAT 1
// match directly based on pubkey hash only
#define ID_REQUEST_FORMAT_HASH 2

struct id_request_tier_entry_t : public state_t{
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

/*
  Statistics driver, responsible for generating the P-value and that's
  about it.
 */

struct id_request_tier_entry_medium_t{
public:
	id_request_tier_entry_t* (*init)() = nullptr;
	void (*close)(id_request_tier_entry_t *ther_entry_state_ptr) = nullptr;
	void (*refresh)(id_request_tier_entry_t *tier_entry_state_ptr) = nullptr;

	uint8_t medium = 0;
	id_request_tier_entry_medium_t(
		uint8_t medium_,
		id_request_tier_entry_t* (*init_)(),
		void (*close_)(id_request_tier_entry_t *tier_entry_state_ptr),
		void (*refresh_)(id_request_tier_entry_t *tier_entry_state_ptr)){
		medium = medium_;
		init = init_;
		close = close_;
		refresh = refresh_;
	}
};

// states are used to optimize statistics by keeping a cache stored locally
struct id_request_request_t{
private:
	id_t_ requested_id;
	std::vector<id_request_tier_entry_t> tier_vector;
public:
	data_id_t id;
	id_request_request_t();
	~id_request_request_t();

	void set_request_id(id_t_ requested_id);
};

struct id_request_response_t{
private:
	id_t_ id_request_request_id = ID_BLANK_ID;
	std::vector<std::vector<uint8_t> > response;
	// each subvector is an escaped list of IDs (multidimensional ID
	// vectors don't exist yet, but that's fine since we may be able to
	// compact the information down).
	std::vector<std::vector<uint8_t> > ids;
public:
	data_id_t id;

	GET_SET(id_request_request_id, id_t_);
	GET_SET(response, std::vector<std::vector<uint8_t> >);
	GET_SET(ids, std::vector<std::vector<uint8_t> >);
	
	id_request_response_t();
	~id_request_response_t();
};

#define ID_REQUEST_FORMAT_INIT(format) id_request_tier_entry_t* id_request_##format##_init()
#define ID_REQUEST_FORMAT_CLOSE(format) void id_request_##format##_close(id_request_tier_entry_t *tier_entry_state_ptr)
#define ID_REQUEST_FORMAT_REFRESH(format) void id_request_##format##_refresh(id_request_tier_entry_t *tier_entry_state_ptr)
#define ID_REQUEST_FORMAT_GET_P_VAL(format) math_stat_pval_t id_request_##format##_get_p_val(id_request_tier_entry_t *tier_entry_state_ptr)

extern void id_request_init();
extern void id_request_loop();
extern void id_request_close();

extern const std::vector<id_request_tier_entry_medium_t> id_request_tier_mediums;

#endif
