#ifndef NET_HTTP_FILE_DRIVER_H
#define NET_HTTP_FILE_DRIVER_H
/*
  File drivers create downloadable content based on data we have in loadable
  tiers. This is also responsible for converting it to a downloadable/streamable
  format (coupling Opus & VP9/VP9 to WebM, etc), as well as any re-encoding that
  happens through HTTP GET requests
 */

#include "../../id/id.h"
#include "../../id/id_api.h"

#include "../../state.h"

// ID of the socket (given through add and push functions)
// ID of the item to service (if applicable)
// state_ptr of the pair (encoding/decoding states and what not)
typedef std::tuple<id_t_, id_t_, void*> net_http_file_driver_state_socket_service_bind_t;

struct net_http_file_driver_state_t : public state_t{
private:
	std::vector<uint8_t> outbound_data;
	std::vector<uint8_t> min_valid_url;

	std::vector<net_http_file_driver_state_socket_service_bind_t> socket_service;
public:
	data_id_t id;
	net_http_file_driver_state_t();
	~net_http_file_driver_state_t();

	GET(min_valid_url, std::vector<uint8_t>);
	ADD_DEL_VECTOR(socket_service, net_http_file_driver_state_socket_service_bind_t);
};



#define NET_HTTP_FILE_DRIVER_MEDIUM_AUDIO_STREAM 1
#define NET_HTTP_FILE_DRIVER_MEDIUM_VIDEO_STREAM 2
#define NET_HTTP_FILE_DRIVER_MEDIUM_AUDIO_VIDEO_STREAM 3
#define NET_HTTP_FILE_DRIVER_MEDIUM_ATOM 4

#define NET_HTTP_FILE_DRIVER_MEDIUM_INIT(medium_) net_http_file_driver_state_t *net_http_file_driver_##medium_##_init(uint8_t medium, id_t_ service_id)
#define NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(medium_) void net_http_file_driver_##medium_##_close(net_http_file_driver_state_t *file_driver_state_ptr)
#define NET_HTTP_FILE_DRIVER_MEDIUM_PULL(medium_) std::vector<uint8_t> net_http_file_driver_##medium_##_pull(net_http_file_driver_state_t *file_driver_state_ptr, id_t_ socket_id)

struct net_http_file_driver_medium_t{
	net_http_file_driver_state_t* (*init)(
		uint8_t medium,
		id_t_ service_id, // ID to send
		id_t_ socket_id); // socket to send it on
	void (*close)(
		net_http_file_driver_state_t* state_ptr);

	void (*add_binding)(
		std::vector<std::string> header);
	std::vector<uint8_t> (*pull)(
		net_http_file_driver_state_t* state_ptr,
		id_t_ socket_id);
};

extern id_t_ net_http_file_driver_state_from_url(std::vector<uint8_t> url);

#endif
