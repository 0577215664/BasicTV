#include "../../net_http.h"
#include "net_http_file_driver.h"
#include "net_http_file_driver_atom.h"
#include "../../parse/net_http_parse.h"

#include "../../../../state.h"

#include "../../../../tv/tv_item.h"
#include "../../../../tv/tv_channel.h"

#include "../../../proto/net_proto_api.h"

/*
  Atom shouldn't worry about populating the channel_vector right now, but it
  should sometime in the future.

  The BasicTV Atom feed allows for downloading different bitrate audio/video
  files from the node, which don't inherently have to exist at those bitrates,
  but can be generated on the fly
*/

static std::string atom_tv_channel_to_prefix(
	id_t_ channel_id){
	tv_channel_t *channel_ptr =
		PTR_DATA(channel_id,
			 tv_channel_t);
	std::string title;
	std::string subtitle;
	if(channel_ptr != nullptr){
		title = (channel_ptr->search_for_param(VORBIS_COMMENT_PARAM_TITLE).at(0));
		subtitle = (channel_ptr->search_for_param(VORBIS_COMMENT_PARAM_DESCRIPTION).at(0));
	}else{
		title = "NOTITLE";
		subtitle = "NOSUBTITLE";
	}
	std::string retval =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<feed xmlns=\"http://www.w3.org/2005/Atom\">"
		"<title>" + title + "</title>"
		"<subtitle>" + subtitle + "</subtitle>"
		"<link href=\"" "[INSERT URL HERE]" "\" rel=\"self\" />"
		"<id>urn:uuid:" + convert::array::id::to_hex(channel_id) + "</id>"
		"<updated>" + convert::time::to_iso8601(0) +"</updated>"; // channels don't have a bound time
	return retval;
}

static std::string atom_tv_channel_to_suffix(){
	std::string retval =
		"</feed>";
	return retval;
}

static std::string atom_tv_item_to_entry(
	id_t_ item_id){
	tv_item_t *item_ptr =
		PTR_DATA(item_id,
			 tv_item_t);
	PRINT_IF_NULL(item_ptr, P_UNABLE);
	tv_channel_t *channel_ptr =
		PTR_DATA(item_ptr->get_tv_channel_id(),
			 tv_channel_t);
	std::string atom_author = "UNKNOWN";
	std::string atom_author_email = "UNKNOWN";
	if(channel_ptr != nullptr){
		// TODO: allow for multiple authors and emails
		try{
			atom_author =
				channel_ptr->search_for_param(
					VORBIS_COMMENT_PARAM_TITLE).at(0);
		}catch(...){}
		try{
			atom_author_email =
				channel_ptr->search_for_param(
					VORBIS_COMMENT_PARAM_CONTACT).at(0);
		}catch(...){}
	}
	// TODO: make direct mappings between the (soon to be implemeneted)
	// Vorbis comments and Atom/XML tags (crypto integration through
	// atom feeds!)
	std::string retval =
		"<entry>"
		"<title>" + item_ptr->search_for_param(VORBIS_COMMENT_PARAM_TITLE).at(0) + "</title>"
		"<link href=\"" "[INSERT VALID URL HERE]" "\" />"
		"<link rel=\"enclosure\" href=\"" + "[INSERT VALID AUDIO FILE DRIVER URL HERE]" + "\"/>"
		"<id>urn:uuid:" + convert::array::id::to_hex(item_id) + "</id>"
		"<updated>" + convert::time::to_iso8601(item_ptr->get_start_time_micro_s()) + "</updated>"
		"<summary>" + (item_ptr->search_for_param(VORBIS_COMMENT_PARAM_DESCRIPTION).at(0)) +"</summary>"
		"<author>"
		"<name>" + atom_author + "</name>"
		"<email>" + atom_author_email + "</email>"
		"</author>"
		"</entry>";
	return retval;
}

// a blank service ID just returns all items we have locally
NET_HTTP_FILE_DRIVER_MEDIUM_INIT(atom){	
	STD_STATE_INIT(
		net_http_file_driver_state_t,
		file_driver_state_ptr,
		net_http_file_driver_atom_state_t,
		atom_state_ptr);
	file_driver_state_ptr->set_socket_id(
		socket_id);
	file_driver_state_ptr->set_medium(
		NET_HTTP_FILE_DRIVER_MEDIUM_ATOM);

	file_driver_state_ptr->response_payload.set_direction(
		NET_HTTP_PAYLOAD_OUT);
	return file_driver_state_ptr;
}

NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(atom){
	STD_STATE_CLOSE(
		file_driver_state_ptr,
		net_http_file_driver_atom_state_t);
}

NET_HTTP_FILE_DRIVER_MEDIUM_LOOP(atom){
	STD_STATE_GET_PTR(
		file_driver_state_ptr,
		net_http_file_driver_atom_state_t,
		atom_state_ptr);
	std::string retval_str;
	try{
		const id_t_ service_id =
			file_driver_state_ptr->request_payload.form_data.get_id("service_id");
		if(service_id != ID_BLANK_ID){
			// request data
		}
		std::vector<id_t_> item_vector =
			ID_TIER_CACHE_GET(
				TYPE_TV_ITEM_T);
		retval_str +=
			atom_tv_channel_to_prefix(
				service_id);
		for(uint64_t i = 0;i < item_vector.size();i++){
			if(service_id != ID_BLANK_ID){
				CONTINUE_IF_DIFF_OWNER(
					service_id,
					item_vector[i]);
			}
			retval_str +=
				atom_tv_item_to_entry(
					item_vector[i]);
		}
		retval_str +=
			atom_tv_channel_to_suffix();
	}catch(...){
		print("unexpected exception caught", P_ERR);
	}
	// TODO: if hanging was a lot less common, we can wait a second or
	// two to update our list (probably not a good idea)

	net_http_chunk_t payload_chunk;
	payload_chunk.set_payload(
		convert::string::to_bytes(
			retval_str));
	file_driver_state_ptr->response_payload.set_finished(
		true);
	file_driver_state_ptr->response_payload.add_chunks(
		payload_chunk);
		
}
