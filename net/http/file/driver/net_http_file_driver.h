#ifndef NET_HTTP_FILE_DRIVER_H
#define NET_HTTP_FILE_DRIVER_H
#include "../../../../id/id.h"
#include "../../../../id/id_api.h"
#include "../../../../state.h"

#include "../../net_http_payload.h"

#define NET_HTTP_FILE_DRIVER_PAYLOAD_PROGRESS 1
#define NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE 2

// one state per socket per servicing ID


typedef std::vector<std::pair<std::string, std::string> > net_http_pretty_darn_hacky_t;
struct net_http_file_driver_state_t : public state_t{
private:
	id_t_ socket_id = ID_BLANK_ID;

public:
	data_id_t id;
	net_http_file_driver_state_t();
	~net_http_file_driver_state_t();

	GET_SET_ID(socket_id);
	
	// easeier to do things this way
	net_http_payload_t request_payload;
	net_http_payload_t response_payload;
};


#define NET_HTTP_FILE_DRIVER_MEDIUM_ATOM 1
#define NET_HTTP_FILE_DRIVER_MEDIUM_FRONTPAGE 2
#define NET_HTTP_FILE_DRIVER_MEDIUM_DOWNLOAD 3

#define NET_HTTP_FILE_DRIVER_MEDIUM_INIT(medium_) net_http_file_driver_state_t *net_http_file_driver_##medium_##_init(net_http_payload_t request_payload, id_t_ socket_id)
#define NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(medium_) void net_http_file_driver_##medium_##_close(net_http_file_driver_state_t *file_driver_state_ptr)
#define NET_HTTP_FILE_DRIVER_MEDIUM_LOOP(medium_) void net_http_file_driver_##medium_##_loop(net_http_file_driver_state_t *file_driver_state_ptr)

struct net_http_file_driver_medium_t{
	uint8_t medium = 0;
	std::string min_valid_url;
	
	net_http_file_driver_state_t* (*init)(
		net_http_payload_t request_payload,
		id_t_ socket_id) = nullptr; // socket to send it on
	void (*close)(
		net_http_file_driver_state_t* state_ptr) = nullptr;

	void (*loop)(
		net_http_file_driver_state_t* state_ptr) = nullptr;

	net_http_file_driver_medium_t(
		uint8_t medium_,
		std::string min_valid_url_,
		net_http_file_driver_state_t* (*init_)(
			net_http_payload_t request_payload,
			id_t_ socket_id),
		void (*close_)(
			net_http_file_driver_state_t* state_ptr),
		void (*loop_)(
			net_http_file_driver_state_t* state_ptr)){
		medium = medium_;
		min_valid_url = min_valid_url_;
		init = init_;
		close = close_;
		loop = loop_;
	}
};

extern net_http_file_driver_medium_t net_http_file_driver_get_medium(
	uint8_t medium);

extern net_http_file_driver_medium_t net_http_file_driver_get_medium_from_url(
	std::string url);

extern id_t_ net_http_file_driver_state_from_url(std::vector<uint8_t> url);

#endif
