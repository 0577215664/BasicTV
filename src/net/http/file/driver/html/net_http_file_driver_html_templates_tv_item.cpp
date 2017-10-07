#include "net_http_file_driver_html_templates_tv_item.h"
#include "net_http_file_driver_html_templates.h"
#include "../../../parse/net_http_parse.h"
#include "../../../../../tv/tv_item.h"

std::string net_http_file_driver_frontpage_create_tv_item_plain_logic(
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

// TEMPLATES

HTML_TEMP(create_tv_item_plain){
	return H1("Create TV Item (Plain)")
		FORM_START
		FORM_TEXT("Title", create_tv_item_plain_title)
		FORM_TEXT("Description", create_tv_item_plain_desc)
		FORM_TEXT("Wallet ID", create_tv_item_plain_wallet_id)
		FORM_SUBMIT("Create TV Item", create_tv_item_plain_submit)
		FORM_END;
}

HTML_TEMP(create_tv_item_upload){
	return H1("Create TV Item (Upload)")
		FORM_START
		FORM_TEXT("Title", create_tv_item_upload_title)
		FORM_TEXT("Description", create_tv_item_upload_desc)
		FORM_TEXT("Wallet ID", create_tv_item_upload_wallet_id)
		// TODO: discriminate based on MIME type here (?)
		FORM_FILE("File", create_tv_item_upload_data)
		FORM_SUBMIT("Create TV Item", create_tv_item_upload_submit)
		FORM_END;
}

HTML_TEMP(list_tv_item){
	std::string retval = H1("List TV Items");
	std::vector<id_t_> tv_items =
		ID_TIER_CACHE_GET(
			TYPE_TV_ITEM_T);
	std::vector<std::vector<std::string> > tv_item_table = {{
			"ID",
			"Name",
			"Desc"
			// TODO: add a URL to another sink to handle HTTP downloads
		}};
	for(uint64_t i = 0;i < tv_items.size();i++){
		tv_item_t *tv_item_ptr =
			PTR_DATA(tv_items[i],
				 tv_item_t);
		CONTINUE_IF_NULL(tv_item_ptr, P_ERR);
		tv_item_table.push_back(
			std::vector<std::string>(
				{
					convert::array::id::to_hex(tv_items[i]),
						convert::string::from_bytes(tv_item_ptr->get_name()),
						convert::string::from_bytes(tv_item_ptr->get_desc())}));
	}
	return H1("List TV Items") + http::tags::html_table(tv_item_table);
}

