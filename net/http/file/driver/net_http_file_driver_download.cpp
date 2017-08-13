#include "net_http_file_driver_download.h"
#include "net_http_file_driver.h"

#include "../../net_http_parse.h"

#include "../../../../tv/tv_frame_audio.h"
#include "../../../../tv/tv_item.h"

/*
  Since the majority of the HTTP GETs at this stage are human generated, the
  default behaviour is to serve the (only) audio stream down the socket in the
  native format
 */

/*
  For now, the COMPLEX flag isn't allowed
 */

NET_HTTP_FILE_DRIVER_MEDIUM_INIT(download){
	STD_STATE_INIT(
		net_http_file_driver_state_t,
		file_driver_state_ptr,
		net_http_file_driver_download_state_t,
		download_state_ptr);
	file_driver_state_ptr->set_socket_id(
		socket_id);
	const std::vector<std::pair<std::string, std::string> > get_var =
		http::header::get::var_list(
			url);
	file_driver_state_ptr->set_var_list(
		get_var);
	file_driver_state_ptr->set_service_id(
	        http::header::get::pull_id(
			get_var,
			"item_id"));	
	file_driver_state_ptr->set_medium(
		NET_HTTP_FILE_DRIVER_MEDIUM_DOWNLOAD);

	// download specific
	ASSERT(ogg_stream_init(download_state_ptr->ogg_state,
			       0) == 0, P_ERR);
	ASSERT(download_state_ptr->ogg_state != nullptr, P_ERR);
	return file_driver_state_ptr;
}

NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(download){
	STD_STATE_CLOSE(
		file_driver_state_ptr,
		net_http_file_driver_download_state_t);
}

/*
  TODO: allow for chosing an encoding type and bitrate as an 
  HTTP GET (not hard at all honestly, just a matter of implementing
  it)
 */

/*
  Pulls Opus codec packets from the tv_frame_audio_t
*/

/*
  pulls as many IDs as we have locally
  first dimension is the packet offset in tv_item_t
  second dimension is just how many we have in that list
 */
static std::vector<std::vector<id_t_> >  net_http_file_driver_download_new_item_data(
	net_http_file_driver_state_t *file_driver_state_ptr,
	net_http_file_driver_download_state_t *download_state_ptr){
	std::vector<id_t_> pseudo_retval;
	
	tv_item_t *item_ptr =
		PTR_DATA(file_driver_state_ptr->get_service_id(),
			 tv_item_t);
	PRINT_IF_NULL(item_ptr, P_UNABLE);
	bool complex =
		false;
	try{
		complex =
			http::header::get::value_from_var_list(
				file_driver_state_ptr->get_var_list(),
				"complex") == "1";
	}catch(...){}
	ASSERT(complex == false, P_ERR);

	// currently can't dish out more than one, Murphy's Law protection
	ASSERT(download_state_ptr->service_log.size() <= 1, P_ERR);
	if(download_state_ptr->service_log.size() == 1){
		ASSERT(get_id_type(download_state_ptr->service_log[0].second) == TYPE_TV_FRAME_AUDIO_T, P_ERR);
		tv_frame_audio_t *frame_audio_ptr =
			PTR_DATA(download_state_ptr->service_log[0].second,
				 tv_frame_audio_t);
		PRINT_IF_NULL(frame_audio_ptr, P_ERR);
		while(frame_audio_ptr->id.get_linked_list().second.size() != 0){
			frame_audio_ptr =
				PTR_DATA(frame_audio_ptr->id.get_linked_list().second[0],
					 tv_frame_audio_t);
			if(frame_audio_ptr == nullptr){
				break;
			}
			// re-encoding is handled in OGG code (or just not here)
			pseudo_retval.push_back(
				frame_audio_ptr->id.get_id());
		}
	}
	return std::vector<std::vector<id_t_> > ({pseudo_retval});
}

/*
  THIS FUNCTION MAKES A LOT OF ASSUMPTIONS ABOUT THE SAMPLING FREQUENCY
  AND THE LIKE
 */

static std::vector<uint8_t> net_http_file_driver_download_ogg_packetize(
	net_http_file_driver_state_t *file_driver_state_ptr,
	net_http_file_driver_download_state_t *download_state_ptr,
	std::vector<std::vector<id_t_> > packet_vector){
	ASSERT(download_state_ptr->ogg_state != nullptr, P_ERR);
	ASSERT(packet_vector.size() <= 1, P_ERR);
	if(packet_vector.size() == 0){
		return std::vector<uint8_t>({});
	}
	for(uint64_t i = 0;i < packet_vector[0].size();i++){
		tv_frame_audio_t *frame_audio_ptr =
			PTR_DATA(packet_vector[0][i],
				 tv_frame_audio_t);
		CONTINUE_IF_NULL(frame_audio_ptr, P_WARN);
		std::vector<std::vector<uint8_t> > packet_set =
			frame_audio_ptr->get_packet_set();
		// TODO: MAKE SURE THESE FLAGS ARE ACTUALLY BEING USED
		const bool first_packet =
			frame_audio_ptr->get_flags() & TV_FRAME_STANDARD_START_PACKET;
		const bool last_packet =
			frame_audio_ptr->get_flags() & TV_FRAME_STANDARD_END_PACKET;
		for(uint64_t c = 0;c < packet_set.size();c++){
			ogg_packet packet;
			CLEAR(packet);
			packet.packet =
				packet_set[c].data();
			packet.bytes =
				packet_set[c].size();
			if(c == 0){
				packet.b_o_s =
					first_packet;
			}else{
				packet.b_o_s =
					false;
			}
			if(c == packet_set.size()-1){
				packet.e_o_s =
					last_packet;
			}else{
				packet.e_o_s =
					false;
			}
			packet.packetno =
				download_state_ptr->packet_num++;
			ASSERT(ogg_stream_packetin(
				       download_state_ptr->ogg_state,
				       &packet) == 0, P_ERR);
		}
	}
	std::vector<uint8_t> retval;
	bool has_pages = true;
	ogg_page *page_ptr = nullptr;
	while((has_pages =
	       ogg_stream_pageout(
		       download_state_ptr->ogg_state,
		       page_ptr) != 0)){
		retval.insert(
			retval.end(),
			page_ptr->header,
			page_ptr->header+page_ptr->header_len);
		retval.insert(
			retval.end(),
			page_ptr->body,
			page_ptr->body+page_ptr->body_len);
		page_ptr = nullptr;
	}
	return retval;
}

NET_HTTP_FILE_DRIVER_MEDIUM_PULL(download){
	STD_STATE_GET_PTR(
		file_driver_state_ptr,
		net_http_file_driver_download_state_t,
		download_state_ptr);
	file_driver_state_ptr->set_mime_type(
		"application/ogg"); // make more complicated later on
	return std::pair<std::vector<uint8_t>, uint8_t>(
		net_http_file_driver_download_ogg_packetize(
			file_driver_state_ptr,
			download_state_ptr,
			net_http_file_driver_download_new_item_data(
				file_driver_state_ptr,
				download_state_ptr)),
		file_driver_state_ptr->get_payload_status());
}
