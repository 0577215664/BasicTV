#ifndef NET_HTTP_FILE_DRIVER_H
#define NET_HTTP_FILE_DRIVER_H
#include "../../id/id.h"
#include "../../id/id_api.h"
#include "../../state.h"
 
#define NET_HTTP_FILE_DRIVER_PAYLOAD_PROGRESS (1 << 0)
#define NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE (1 << 1)

// one state per socket per servicing ID
struct net_http_file_driver_state_t : public state_t{
private:
	std::vector<uint8_t> outbound_data;
	std::vector<uint8_t> min_valid_url;
	
	id_t_ socket_id;
	id_t_ service_id;
public:
	data_id_t id;
	net_http_file_driver_state_t();
	~net_http_file_driver_state_t();

	GET_SET(min_valid_url, std::vector<uint8_t>);
	GET_SET_ID(socket_id);
	GET_SET_ID(service_id);
	
};

#define NET_HTTP_FILE_DRIVER_MEDIUM_AUDIO_STREAM 1
#define NET_HTTP_FILE_DRIVER_MEDIUM_VIDEO_STREAM 2
#define NET_HTTP_FILE_DRIVER_MEDIUM_AUDIO_VIDEO_STREAM 3
#define NET_HTTP_FILE_DRIVER_MEDIUM_ATOM 4

#define NET_HTTP_FILE_DRIVER_MEDIUM_INIT(medium_) net_http_file_driver_state_t *net_http_file_driver_##medium_##_init(uint8_t medium, id_t_ service_id, id_t_ socket_id)
#define NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(medium_) void net_http_file_driver_##medium_##_close(net_http_file_driver_state_t *file_driver_state_ptr)
#define NET_HTTP_FILE_DRIVER_MEDIUM_PULL(medium_) std::pair<std::vector<uint8_t>, uint8_t> net_http_file_driver_##medium_##_pull(net_http_file_driver_state_t *file_driver_state_ptr)

struct net_http_file_driver_medium_t{
	uint8_t medium = 0;
	
	net_http_file_driver_state_t* (*init)(
		uint8_t medium,
		id_t_ service_id, // ID to send
		id_t_ socket_id); // socket to send it on
	void (*close)(
		net_http_file_driver_state_t* state_ptr);

	std::pair<std::vector<uint8_t>, uint8_t> (*pull)(
		net_http_file_driver_state_t* state_ptr);
};

extern net_http_file_driver_medium_t net_http_file_driver_get_medium(
	uint8_t medium);

extern id_t_ net_http_file_driver_state_from_url(std::vector<uint8_t> url);

#endif
