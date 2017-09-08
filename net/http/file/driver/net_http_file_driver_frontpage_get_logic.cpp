#include "net_http_file_driver_frontpage_get_logic.h"
#include "net_http_file_driver_frontpage.h"
#include "../../net_http.h"
#include "../../net_http_parse.h"

#include "../../../../cryptocurrency.h"
#include "../../../../id/id.h"

#include "../../../../tv/tv_window.h"
#include "../../../../tv/tv_channel.h"
#include "../../../../tv/tv_item.h"
#include "../../../../tv/tv_frame_audio.h"
#include "../../../../tv/tv_frame_video.h"
#include "../../../../tv/tv_meta.h"


#define GET_LOGIC_VAR_RUN(x_) if(get_payload[i].first == #x_ "_submit"){return net_http_file_driver_frontpage_##x_(get_payload);}

#define GET_STR(name) std::string name = http::header::get::value_from_var_list(get_vector, #name);

static std::string net_http_file_driver_frontpage_create_wallet_set(
	std::vector<std::pair<std::string, std::string> > get_vector){

	std::string retval = "Created Wallet Set ";
	wallet_set_t *wallet_set_ptr =
		new wallet_set_t;
	retval += convert::array::id::to_hex(wallet_set_ptr->id.get_id());
	return retval ;
}

static std::string net_http_file_driver_frontpage_add_wallet_to_set(
	std::vector<std::pair<std::string, std::string> > get_vector){
	const id_t_ wallet_set_id =
		convert::array::id::from_hex(
			http::header::get::value_from_var_list(
				get_vector,
				"add_wallet_to_set_id"));
	const std::string wallet_set_address =
		http::header::get::value_from_var_list(
			get_vector,
			"add_wallet_to_set_address");
	const std::string wallet_set_code =
		http::header::get::value_from_var_list(
			get_vector,
			"add_wallet_to_set_code");

	wallet_set_t *wallet_set_ptr =
		PTR_DATA(wallet_set_id,
			 wallet_set_t);

	if(wallet_set_ptr == nullptr){
		return "ID does not exist";
	}
	wallet_set_ptr->add_wallet(
		convert::string::to_bytes(wallet_set_code),
		convert::string::to_bytes(wallet_set_address));
	return "Added wallet " + wallet_set_address + " of type " + wallet_set_code + " to " + convert::array::id::to_hex(wallet_set_id);
}

std::string net_http_file_driver_frontpage_create_tv_channel(
	std::vector<std::pair<std::string, std::string> > get_vector){

	tv_channel_t *channel_ptr =
		new tv_channel_t;
	
	const GET_STR(create_tv_channel_title);
	const GET_STR(create_tv_channel_desc);

	channel_ptr->add_param(
		VORBIS_COMMENT_PARAM_TITLE,
		create_tv_channel_title,
		true);
	channel_ptr->add_param(
		VORBIS_COMMENT_PARAM_DESCRIPTION,
		create_tv_channel_desc,
		true);
	return "Created TV Channel " + convert::array::id::to_hex(channel_ptr->id.get_id());
}

std::string net_http_file_driver_frontpage_create_tv_item_plain(
	std::vector<std::pair<std::string, std::string> > get_vector){
	tv_item_t *tv_item_ptr =
		new tv_item_t;
	return "Created TV Item " + convert::array::id::to_hex(tv_item_ptr->id.get_id());
}

std::string net_http_file_driver_frontpage_get_logic(
	net_http_file_driver_state_t *file_driver_state_ptr){
	std::vector<std::pair<std::string, std::string> > get_payload =
		file_driver_state_ptr->get_var_list();

	for(uint64_t i = 0;i < get_payload.size();i++){
		GET_LOGIC_VAR_RUN(create_wallet_set);
		GET_LOGIC_VAR_RUN(add_wallet_to_set);
		GET_LOGIC_VAR_RUN(create_tv_channel);
	}
	return ""; // TODO: actually make a proper response
}
