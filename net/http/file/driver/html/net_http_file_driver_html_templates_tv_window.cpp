#include "net_http_file_driver_html_templates_tv_window.h"
#include "net_http_file_driver_html_templates.h"

#include "../../../../../tv/tv_window.h"

HTML_TEMP(list_tv_window){
	std::vector<std::vector<std::string> > retval;
	std::vector<id_t_> window_vector =
		ID_TIER_CACHE_GET(
			TYPE_TV_WINDOW_T);
	retval.push_back(
		std::vector<std::string>({
				"ID"}));
	for(uint64_t i = 0;i < window_vector.size();i++){
		tv_window_t *window_ptr =
			PTR_DATA(window_vector[i],
				 tv_window_t);
		CONTINUE_IF_NULL(window_ptr, P_WARN);
		retval.push_back(
			std::vector<std::string>(
				{convert::array::id::to_hex(window_vector[i])}));
	}
	return http::tags::html_table(retval);
}
