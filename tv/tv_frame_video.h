#ifndef TV_FRAME_VIDEO_H
#define TV_FRAME_VIDEO_H
#include "tv_frame_standard.h"
#include "video/tv_video.h"

class tv_frame_video_t : public tv_frame_standard_t{
private:
	std::vector<std::vector<uint8_t> > packet_set;
	tv_video_prop_t video_prop;
public:
	data_id_t id;
	tv_frame_video_t();
	~tv_frame_video_t();
	GET_SET(video_prop, tv_video_prop_t);
	GET_SET(packet_set, std::vector<std::vector<uint8_t> >);
	
};

#endif
