#include "../../id/id.h"
#include "../../util.h"
#ifndef TV_VIDEO_H
#define TV_VIDEO_H
// logic behind rendering video to the screen

#define TV_VIDEO_FORMAT_UNDEFINED 0
#define TV_VIDEO_FORMAT_VP9 1

struct tv_video_prop_t{
private:
	uint64_t bits_per_second = 0;
	uint32_t x_res = 0;
	uint32_t y_res = 0;
	uint8_t compression_scheme = 0;
	uint8_t flags = 0;
	uint8_t format = 0;
public:
	void list_virtual_data(data_id_t *id);
	GET_SET_S(bits_per_second, uint32_t);
	GET_SET_S(x_res, uint32_t);
	GET_SET_S(y_res, uint32_t);
	GET_SET_S(compression_scheme, uint8_t);
	GET_SET_S(flags, uint8_t);
};

#endif
