#include "net_http_file_driver_frontpage_forms.h"
#include "net_http_file_driver_frontpage.h"

#include "html/net_http_file_driver_html_templates.h"

std::string net_http_file_driver_frontpage_form_create_wallet_set(){
	return H1("Create Wallet Set")
		FORM_START
		FORM_SUBMIT("Create", create_wallet_set_submit)
		FORM_END;
}

std::string net_http_file_driver_frontpage_form_add_wallet_to_set(){
	return H1("Add Wallet to Set")
		FORM_START
		FORM_TEXT("Wallet Set ID", add_wallet_to_set_id)
		FORM_TEXT("Address", add_wallet_to_set_address)
		FORM_TEXT("Trading Code (BTC, ETH, USD, etc)", add_wallet_to_set_code)
		FORM_SUBMIT("Add Wallet", add_wallet_to_set_submit)
		FORM_END;
}

std::string net_http_file_driver_frontpage_form_create_tv_channel(){
	return H1("Create a TV Channel")
		FORM_START
		FORM_TEXT("Title", create_tv_channel_title)
		FORM_TEXT("Description", create_tv_channel_desc)
		FORM_TEXT("Wallet Set ID", create_tv_channel_wallet_id)
		FORM_SUBMIT("Create TV Channel", create_tv_channel_submit)
		FORM_END;
}

std::string net_http_file_driver_frontpage_form_create_tv_item_plain(){
	return H1("Create TV Item (Plain)")
		FORM_START
		FORM_TEXT("Title", create_tv_item_plain_title)
		FORM_TEXT("Description", create_tv_item_plain_desc)
		FORM_TEXT("Wallet ID", create_tv_item_plain_wallet_id)
		FORM_SUBMIT("Create TV Item", create_tv_item_plain_submit)
		FORM_END;
}

std::string net_http_file_driver_frontpage_form_create_tv_item_upload(){
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

std::string net_http_file_driver_frontpage_form_bind_tv_sink_item_window(){
	return H1("Bind TV Sink, TV Item, and TV Window")
		FORM_START
		FORM_TEXT("TV Sink ID", bind_tv_sink_item_window_sink_id)
		FORM_TEXT("TV Item ID", bind_tv_sink_item_window_item_id)
		FORM_TEXT("TV Window ID", bind_tv_sink_item_window_window_id)
		FORM_SUBMIT("Bind TV Sink to Item", bind_tv_sink_item_submit)
		FORM_END;
}

std::string net_http_file_driver_frontpage_form_unbind_tv_sink_item_window(){
	return H1("Unbind TV Sink, TV Item, and TV Window")
		FORM_START
		FORM_TEXT("TV Sink ID", unbind_tv_sink_item_sink_id)
		FORM_TEXT("TV Item ID", unbind_tv_sink_item_item_id)
		FORM_TEXT("TV Window ID", unbind_tv_sink_item_window_id)
		FORM_SUBMIT("Unbind TV Sink to Item", unbind_tv_sink_item_submit)
		FORM_END;
}

std::string net_http_file_driver_frontpage_form_control_window(){
	return H1("TV Window Controls (TODO: Get better HTML input macros)");
}



