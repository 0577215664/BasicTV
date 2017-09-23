#include "net_http_file_driver_html_templates_tv_sink.h"
#include "net_http_file_driver_html_templates.h"

#include "../../../../../tv/sink/tv_sink.h"
#include "../../../../../tv/tv.h"

#define RETURN_STR(x__) if(macro == x__){return #x__;}

static std::string stringify_medium(uint8_t macro){
	RETURN_STR(TV_SINK_MEDIUM_AUDIO_HARDWARE);
	RETURN_STR(TV_SINK_MEDIUM_NUMERICAL_TCP_ACCEPT);
	return "UNDEFINED";
}

static std::string stringify_frame_type(uint8_t macro){
	RETURN_STR(TV_FRAME_TYPE_AUDIO);
	RETURN_STR(TV_FRAME_TYPE_VIDEO);
	RETURN_STR(TV_FRAME_TYPE_TEXT);
	RETURN_STR(TV_FRAME_TYPE_NUMERICAL);
	return "UNDEFINED";
}

HTML_TEMP(list_tv_sink){
	std::vector<std::vector<std::string> > sink_data;
	std::vector<id_t_> sink_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_SINK_STATE_T);
	sink_data.push_back(
		std::vector<std::string>({
				"ID",
					"Medium",
					"Frame Type"}));
	for(uint64_t i = 0;i < sink_vector.size();i++){
		tv_sink_state_t *sink_ptr =
			PTR_DATA(sink_vector[i],
				 tv_sink_state_t);
		CONTINUE_IF_NULL(sink_ptr, P_WARN);

		sink_data.push_back(
			std::vector<std::string>({
					convert::array::id::to_hex(sink_vector[i]),
						stringify_medium(sink_ptr->get_medium()),
						stringify_frame_type(sink_ptr->get_frame_type())}));
	}
	return http::tags::html_table(sink_data);
}
