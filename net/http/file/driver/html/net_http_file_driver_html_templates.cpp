#include "net_http_file_driver_html_templates.h"
#include "../net_http_file_driver.h"

#include "net_http_file_driver_html_templates_tv_item.h"
#include "net_http_file_driver_html_templates_tv_channel.h"
#include "net_http_file_driver_html_templates_tv_sink.h"
#include "net_http_file_driver_html_templates_tv_window.h"

#define GET_LOGIC_VAR_RUN(x_) if(get_payload[i].first == #x_ "_submit"){return net_http_file_driver_frontpage_##x_(file_driver_state_ptr);}

std::string net_http_file_driver_html_template_generate(
	std::string temp){
	return temp;
}

std::string net_http_file_driver_frontpage_get_logic(
	net_http_file_driver_state_t *file_driver_state_ptr){
	const std::vector<std::pair<std::string, std::string> > get_payload =
		file_driver_state_ptr->request_payload.form_data.get_table();

	for(uint64_t i = 0;i < get_payload.size();i++){
		// GET_LOGIC_VAR_RUN(create_wallet_set);
		// GET_LOGIC_VAR_RUN(add_wallet_to_set);
		// GET_LOGIC_VAR_RUN(create_tv_channel);
		// GET_LOGIC_VAR_RUN(create_tv_item_plain);
		// // GET_LOGIC_VAR_RUN(create_tv_item_upload);
		// // hold off on uploads until we have good HTTP upload handling
		// GET_LOGIC_VAR_RUN(bind_tv_sink_item_window);
		// // GET_LOGIC_VAR_RUN(unbind_tv_sink_item);
	}
	return ""; // TODO: actually make a proper response
}

