#include "net_http_file_driver_download.h"
#include "net_http_file_driver.h"

NET_HTTP_FILE_DRIVER_MEDIUM_INIT(download){
	STD_STATE_INIT(
		net_http_file_driver_state_t,
		file_driver_state_ptr,
		net_http_file_driver_download_state_t,
		download_state_ptr);
	file_driver_state_ptr->set_socket_id(
		socket_id);
	file_driver_state_ptr->set_service_id(
		service_id);
	file_driver_state_ptr->set_medium(
		NET_HTTP_FILE_DRIVER_MEDIUM_DOWNLOAD);
	return file_driver_state_ptr;
}

NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(download){
	STD_STATE_CLOSE(
		file_driver_state_ptr,
		net_http_file_driver_download_state_t);
}

NET_HTTP_FILE_DRIVER_MEDIUM_PULL(download){
	print("actually implement me", P_WARN);
	file_driver_state_ptr->set_payload_status(
		NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE);
	file_driver_state_ptr->set_mime_type(
		"application/ogg"); // make more complicated later on
	return std::pair<std::vector<uint8_t>, uint8_t>(
		std::vector<uint8_t>({}),
		NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE);
}
