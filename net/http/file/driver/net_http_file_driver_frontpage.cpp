#include "net_http_file_driver_frontpage.h"

NET_HTTP_FILE_DRIVER_MEDIUM_INIT(frontpage){
	STD_STATE_INIT(
		net_http_file_driver_state_t,
		file_driver_state_ptr,
		net_http_file_driver_frontpage_state_t,
		frontpage_state_ptr);
	file_driver_state_ptr->set_socket_id(
		socket_id);
	file_driver_state_ptr->set_service_id(
		service_id);
	file_driver_state_ptr->set_medium(
		NET_HTTP_FILE_DRIVER_MEDIUM_FRONTPAGE);
	return file_driver_state_ptr;
}

NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(frontpage){
	STD_STATE_CLOSE(
		file_driver_state_ptr,
		net_http_file_driver_frontpage_state_t);
}

const std::string frontpage_test =
	"<html><head><title>BasicTV</title></head><body>Yo</body></html>";

NET_HTTP_FILE_DRIVER_MEDIUM_PULL(frontpage){
	file_driver_state_ptr->set_payload_status(
		NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE);
	file_driver_state_ptr->set_mime_type(
		"text/html");
	return std::make_pair(
		convert::string::to_bytes(frontpage_test),
		NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE);
}
