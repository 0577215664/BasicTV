#include "net_http.h"
#include "net_http_file_driver.h"
#include "net_http_file_driver_atom.h"

#include "../../state.h"

#include "../../tv/tv_item.h"
#include "../../tv/tv_channel.h"

#include "../../net/proto/net_proto_api.h"

/*
  Atom shouldn't worry about populating the channel_vector right now, but it
  should sometime in the future.

  The BasicTV Atom feed allows for downloading different bitrate audio/video
  files from the node, which don't inherently have to exist at those bitrates,
  but can be generated on the fly
*/

static std::string atom_get_prefix(){
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
}

static std::string atom_wrap_to_feed(std::string data){
	return "<feed xmlns=\"http://www.w3.org/2005/Atom\">" + data + "</feed>";
}

static std::string atom_wrap_title(std::string data){
	return "<title>" + data + "</title>";
}

static std::string atom_tv_channel_to_prefix(
	id_t_ channel_id){
	tv_channel_t *channel_ptr =
		PTR_DATA(channel_id,
			 tv_channel_t);
	std::string retval =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<feed xmlns=\"http://www.w3.org/2005/Atom\">"
		"<title>" + (channel_ptr->search_for_param(VORBIS_COMMENT_PARAM_TITLE).at(0)) + "</title>"
		"<subtitle>" + (channel_ptr->search_for_param(VORBIS_COMMENT_PARAM_DESCRIPTION).at(0)) + "</subtitle>"
		"<link href=\"" "[INSERT URL HERE]" "\" rel=\"self\" />"
		"<id>urn:uuid:60a76c80-d399-11d9-b91C-0003939e0af6</id>"
		"<updated>" + convert::time::to_iso8601(0) +"</updated>"; // channels don't have a bound time
	return retval;
}

static std::string atom_tv_channel_to_suffix(
	id_t_ channel_id){
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
		// try{
			// atom_author_email =
			// 	channel_ptr->search_for_param(
			// 		VORBIS_COMMENT_PARAM_CONTACT).at(0);
		// }catch(...){}
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

NET_HTTP_FILE_DRIVER_MEDIUM_INIT(atom){	
	STD_STATE_INIT(
		net_http_file_driver_state_t,
		file_driver_state_ptr,
		net_http_file_driver_atom_state_t,
		atom_state_ptr);
	return file_driver_state_ptr;
}

NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(atom){
	STD_STATE_CLOSE(
		file_driver_state_ptr,
		net_http_file_driver_atom_state_t);
}

NET_HTTP_FILE_DRIVER_MEDIUM_PULL(atom){
	STD_STATE_GET_PTR(
		file_driver_state_ptr,
		net_http_file_driver_atom_state_t,
		atom_state_ptr);
	std::string retval_str;
	
	uint64_t socket_pos =
		file_driver_state_ptr->find_iter_socket_service(
			[&socket_id](const std::tuple<id_t_, id_t_, void*> &elem){
				return std::get<1>(elem) == socket_id;
			});
	std::vector<std::tuple<std::string, std::string, std::string> > item_metadata;
	if(socket_pos < file_driver_state_ptr->get_size_socket_service()){
		try{
			const std::tuple<id_t_, id_t_, void*> socket_tuple =
				file_driver_state_ptr->get_elem_socket_service(
					socket_pos);
			tv_channel_t *channel_ptr =
				PTR_DATA(std::get<0>(socket_tuple),
					 tv_channel_t);
			net_proto::request::add_type_hash_whitelist(
				{TYPE_TV_ITEM_T},
				get_id_hash(std::get<0>(socket_tuple)));
			std::vector<id_t_> item_vector =
				ID_TIER_CACHE_GET(
					TYPE_TV_ITEM_T);
			retval_str +=
				atom_tv_channel_to_prefix(
					std::get<1>(socket_tuple));
			for(uint64_t i = 0;i < item_vector.size();i++){
				CONTINUE_IF_DIFF_OWNER(
				        std::get<0>(socket_tuple),
					item_vector[i]);
				retval_str +=
					atom_tv_item_to_entry(
						item_vector[i]);
			}
			retval_str +=
				atom_tv_channel_to_suffix(
					std::get<1>(socket_tuple)); // channel_id isn't used currently
		}catch(...){
			print("unexpected exception caught", P_ERR);
		}
	}else{
		print("socket is not bound to a servicable item", P_WARN);
	}
	
	
	return convert::string::to_bytes(
		retval_str);;
}
