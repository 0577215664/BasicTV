#include "net_http_file_driver_frontpage_get_logic.h"
#include "net_http_file_driver_frontpage.h"
#include "html/net_http_file_driver_html_templates.h"
#include "../../net_http.h"
#include "../../parse/net_http_parse.h"

#include "../../../../cryptocurrency.h"
#include "../../../../id/id.h"

#include "../../../../tv/tv_window.h"
#include "../../../../tv/tv_channel.h"
#include "../../../../tv/tv_item.h"
#include "../../../../tv/tv_frame_audio.h"
#include "../../../../tv/tv_frame_numerical.h"
#include "../../../../tv/tv_frame_video.h"
#include "../../../../tv/tv_meta.h"

static std::string net_http_file_driver_frontpage_create_wallet_set(
	net_http_file_driver_state_t *file_driver_state_ptr){

	std::string retval = "Created Wallet Set ";
	wallet_set_t *wallet_set_ptr =
		new wallet_set_t;
	retval += convert::array::id::to_hex(wallet_set_ptr->id.get_id());
	return retval;
}

static std::string net_http_file_driver_frontpage_add_wallet_to_set(
	net_http_file_driver_state_t *file_driver_state_ptr){
	const id_t_ wallet_set_id =
		file_driver_state_ptr->request_payload.form_data.get_id(
			"add_wallet_to_set_id");
	const std::string wallet_set_address =
		file_driver_state_ptr->request_payload.form_data.get_str(
			"add_wallet_to_set_address");
	const std::string wallet_set_code =
		file_driver_state_ptr->request_payload.form_data.get_str(
			"add_wallet_to_set_address_code");

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
	net_http_file_driver_state_t *file_driver_state_ptr){

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

// tv_item_t isn't bound to the window, this function just creates a
// seed frame type from the tv_sink_state_t's frame_type, and adds
// a binding between the sink state and the seed frame to the tv_window_t,
// and adds the seed frame id to the tv_item_t.

// TODO: should expand upon this and expose individual streams being
// bound, instead of assuming one stream per frame

static std::string net_http_file_driver_frontpage_bind_tv_sink_item_window(
	net_http_file_driver_state_t *file_driver_state_ptr){
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
	item_ptr->add_frame_id(
		std::vector<id_t_>({frame_id}));
	return "Bound TV Item, TV Sink, and TV Window";
}

static std::string net_http_file_driver_frontpage_unbind_tv_sink_item_window(
	net_http_file_driver_state_t *file_driver_state_ptr){
	const GET_STR(unbind_tv_sink_item_window_item_id);
	const GET_STR(unbind_tv_sink_item_window_sink_id);
	const GET_STR(unbind_tv_sink_item_window_window_id);
	tv_window_t *window_ptr =
		PTR_DATA(convert::array::id::from_hex(
				 unbind_tv_sink_item_window_item_id),
			 tv_window_t);
	if(window_ptr == nullptr){
		return "TV Window is NULL";
	}
	std::vector<
		std::tuple<
			id_t_,
			id_t_,
			std::vector<uint8_t> > > active_streams =
		window_ptr->get_active_streams();
	return "TODO: make the unbinding code go to the end of the linked list"
		" and check to see for any overlap";
	// const id_t frame_id =
	// 	convert::array::id::from_hex(
	// 		unbind_tv_sink_item_window_item_id);
	// for(uint64_t i = 0;i < active_streams.size();i++){
	// 	if(std::get<0>(active_streams[i]) == 
	// }
}
