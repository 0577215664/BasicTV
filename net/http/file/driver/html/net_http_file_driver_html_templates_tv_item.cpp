#include "net_http_file_driver_html_templates_tv_item.h"
#include "net_http_file_driver_html_templates.h"

#include "../../../../../tv/tv_item.h"

std::string net_http_file_driver_frontpage_create_tv_item_plain(
	net_http_file_driver_state_t *file_driver_state_ptr){
	tv_item_t *tv_item_ptr =
		new tv_item_t;
	std::string error_str;
	const GET_STR(create_tv_item_plain_title);
	const GET_STR(create_tv_item_plain_desc);
	const GET_STR(create_tv_item_plain_wallet_id);
	tv_item_ptr->add_param(
		VORBIS_COMMENT_PARAM_TITLE,
		create_tv_item_plain_title,
		true);
	tv_item_ptr->add_param(
		VORBIS_COMMENT_PARAM_DESCRIPTION,
		create_tv_item_plain_desc,
		true);
	if(create_tv_item_plain_wallet_id != ""){
		try{
			tv_item_ptr->set_wallet_set_id(
				convert::array::id::from_hex(
					create_tv_item_plain_wallet_id));
		}catch(...){
			error_str = "Couldn't bind Wallet ID";
		}
	}
	return error_str + "<br />Created TV Item " + convert::array::id::to_hex(tv_item_ptr->id.get_id());
}
