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
#include "../../../../tv/tv_frame_numerical.h"
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

static std::string net_http_file_driver_frontpage_create_tv_channel(
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

static std::string net_http_file_driver_frontpage_create_tv_item_plain(
	std::vector<std::pair<std::string, std::string> > get_vector){
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

static std::string net_http_file_driver_frontpage_bind_tv_sink_item_window(
	std::vector<std::pair<std::string, std::string> > get_vector){
	std::string retval;
	const GET_STR(bind_tv_sink_item_window_sink_id);
	const GET_STR(bind_tv_sink_item_window_item_id);
	const GET_STR(bind_tv_sink_item_window_window_id);
	tv_item_t *item_ptr =
		PTR_DATA(convert::array::id::from_hex(
				 bind_tv_sink_item_window_item_id),
			 tv_item_t);
	if(item_ptr == nullptr){
		return "TV Item is NULL";
	}
	tv_sink_state_t *sink_state_ptr =
		PTR_DATA(convert::array::id::from_hex(
				 bind_tv_sink_item_window_sink_id),
			tv_sink_state_t);
	if(sink_state_ptr == nullptr){
		return "TV Sink is NULL";
	}
	tv_window_t *window_ptr =
		PTR_DATA(convert::array::id::from_hex(
				 bind_tv_sink_item_window_window_id),
			 tv_window_t);
	if(window_ptr == nullptr){
		return "TV Window is NULL";
	}

	id_t_ frame_id = ID_BLANK_ID;
	switch(sink_state_ptr->get_frame_type()){
	case TV_FRAME_TYPE_AUDIO:
		frame_id = (new tv_frame_audio_t)->id.get_id();
		break;
	case TV_FRAME_TYPE_NUMERICAL:
		frame_id = (new tv_frame_numerical_t)->id.get_id();
		break;
	default:
		return "Unsupported TV Sink Frame Type";
	}
	window_ptr->set_active_streams(
		std::vector<std::tuple<id_t_, id_t_, std::vector<uint8_t> > >(
			{std::make_tuple(
					frame_id,
					convert::array::id::from_hex(
						bind_tv_sink_item_window_sink_id),
					std::vector<uint8_t>({}))}));
	// Vorbis bindings/etc aren't used right now
			
	return retval;
}

std::string net_http_file_driver_frontpage_get_logic(
	net_http_file_driver_state_t *file_driver_state_ptr){
	std::vector<std::pair<std::string, std::string> > get_payload =
		file_driver_state_ptr->get_var_list();

	for(uint64_t i = 0;i < get_payload.size();i++){
		GET_LOGIC_VAR_RUN(create_wallet_set);
		GET_LOGIC_VAR_RUN(add_wallet_to_set);
		GET_LOGIC_VAR_RUN(create_tv_channel);
		GET_LOGIC_VAR_RUN(create_tv_item_plain);
		// GET_LOGIC_VAR_RUN(create_tv_item_upload);
		// hold off on uploads until we have good HTTP upload handling
		GET_LOGIC_VAR_RUN(bind_tv_sink_item_window);
		// GET_LOGIC_VAR_RUN(unbind_tv_sink_item);
	}
	return ""; // TODO: actually make a proper response
}
