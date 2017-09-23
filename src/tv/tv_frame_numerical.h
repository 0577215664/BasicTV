#ifndef TV_FRAME_NUMERICAL_H
#define TV_FRAME_NUMERICAL_H

#define TV_FRAME_NUMERICAL_ESCAPE 0x20

#include "../id/id.h"
#include "../id/id_api.h"
#include "tv_frame_standard.h"

struct tv_frame_numerical_t : public tv_frame_standard_t{
private:
	// math_number_set_t is a mess right now, and
	// this should satisfy most needs (and possibly expand
	// to another tv_frame_numerical_t type if we have to)
	std::vector<uint8_t> escaped_one_dimension_data;
public:
	data_id_t id;
	tv_frame_numerical_t();
	~tv_frame_numerical_t();
	GET_SET(escaped_one_dimension_data, std::vector<uint8_t>);
};

#endif

