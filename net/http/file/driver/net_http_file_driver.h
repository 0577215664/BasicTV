#ifndef NET_HTTP_FILE_DRIVER_H
#define NET_HTTP_FILE_DRIVER_H
#include "../../../../id/id.h"
#include "../../../../id/id_api.h"
#include "../../../../state.h"
 
#define NET_HTTP_FILE_DRIVER_PAYLOAD_PROGRESS 1
#define NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE 2

// one state per socket per servicing ID


typedef std::vector<std::pair<std::string, std::string> > net_http_pretty_darn_hacky_t;
struct net_http_file_driver_state_t : public state_t{
private:
	std::vector<uint8_t> outbound_data;
	std::vector<std::pair<std::string, std::string> > var_list;
	std::string mime_type;
	id_t_ socket_id;
	id_t_ service_id;
	uint8_t payload_status = 0;
public:
	data_id_t id;
	net_http_file_driver_state_t();
	~net_http_file_driver_state_t();

	GET_SET_ID(socket_id);
	GET_SET_ID(service_id);
	GET_SET(payload_status, uint8_t);
	GET_SET(var_list, net_http_pretty_darn_hacky_t);
	GET_SET(mime_type, std::string);
};


#define NET_HTTP_FILE_DRIVER_MEDIUM_ATOM 1
#define NET_HTTP_FILE_DRIVER_MEDIUM_FRONTPAGE 2
#define NET_HTTP_FILE_DRIVER_MEDIUM_DOWNLOAD 3

#define NET_HTTP_FILE_DRIVER_MEDIUM_INIT(medium_) net_http_file_driver_state_t *net_http_file_driver_##medium_##_init(std::string url, id_t_ socket_id)
#define NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(medium_) void net_http_file_driver_##medium_##_close(net_http_file_driver_state_t *file_driver_state_ptr)
#define NET_HTTP_FILE_DRIVER_MEDIUM_PULL(medium_) std::pair<std::vector<uint8_t>, uint8_t> net_http_file_driver_##medium_##_pull(net_http_file_driver_state_t *file_driver_state_ptr)

struct net_http_file_driver_medium_t{
	uint8_t medium = 0;
	std::string min_valid_url;
	
	net_http_file_driver_state_t* (*init)(
		std::string url,
		id_t_ socket_id) = nullptr; // socket to send it on
	void (*close)(
		net_http_file_driver_state_t* state_ptr) = nullptr;

	std::pair<std::vector<uint8_t>, uint8_t> (*pull)(
		net_http_file_driver_state_t* state_ptr) = nullptr;
};

extern net_http_file_driver_medium_t net_http_file_driver_get_medium(
	uint8_t medium);

extern net_http_file_driver_medium_t net_http_file_driver_get_medium_from_url(
	std::string url);

extern id_t_ net_http_file_driver_state_from_url(std::vector<uint8_t> url);

#endif
