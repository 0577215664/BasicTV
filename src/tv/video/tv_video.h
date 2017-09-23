#ifndef TV_VIDEO_H
#define TV_VIDEO_H

#include "../../id/id.h"
#include "../../util.h"
#include "../tv_prop.h"

#define TV_VIDEO_FORMAT_UNDEFINED 0
#define TV_VIDEO_FORMAT_VP9 1

class tv_video_prop_t : public tv_prop_t{
private:
	uint32_t x_res = 0;
	uint32_t y_res = 0;
	uint8_t bit_depth = 0;

	data_id_t *id = nullptr;
public:
	tv_video_prop_t();
	~tv_video_prop_t();
	void list_virtual_data(data_id_t *id_);
	bool operator==(const tv_video_prop_t& rhs){
		return x_res == rhs.x_res &&
		y_res == rhs.y_res &&
		bit_depth == rhs.bit_depth;
	}
	bool operator!=(const tv_video_prop_t &rhs){
		return !(*this == rhs);
	}
	GET_SET_V(x_res, uint32_t);
	GET_SET_V(y_res, uint32_t);
};

#endif
