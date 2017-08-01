#include "net_http.h"
#include "net_http_file_driver.h"
#include "net_http_file_driver_atom.h"

#include "../../state.h"

#include "../../tv/tv_item.h"
#include "../../tv/tv_channel.h"

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
		"<title>" + convert::string::from_bytes(channel_ptr->get_name()) + "</title>"
		"<subtitle>" + convert::string::from_bytes(channel_ptr->get_description()) + "</subtitle>"
		"<link href=\"" "[INSERT URL HERE]" "\" rel=\"self\" />"
		"<id>urn:uuid:60a76c80-d399-11d9-b91C-0003939e0af6</id>"
		"<updated>2003-12-13T18:30:02Z</updated>";
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
	std::string retval =
		"<entry>"
		"<title>Atom-Powered Robots Run Amok</title>"
		"<link href=\"http://example.org/2003/12/13/atom03\" />"
		"<link rel=\"alternate\" type=\"text/html\" href=\"http://example.org/2003/12/13/atom03.html\"/>"
		"<link rel=\"edit\" href=\"http://example.org/2003/12/13/atom03/edit\"/>"
		"<id>urn:uuid:1225c695-cfb8-4ebb-aaaa-80da344efa6a</id>"
		"<updated>2003-12-13T18:30:02Z</updated>"
		"<summary>Some text.</summary>"
		"<content type=\"xhtml\">"
		"<div xmlns=\"http://www.w3.org/1999/xhtml\">"
		"<p>This is the entry content.</p>"
		"</div>"
		"</content>"
		"<author>"
		"<name>John Doe</name>"
		"<email>johndoe@example.com</email>"
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
	std::vector<uint8_t> retval;
	return retval;
}
