#ifndef NET_HTTP_FILE_DRIVER_DOWNLOAD_H
#define NET_HTTP_FILE_DRIVER_DOWNLOAD_H

#include "net_http_file_driver.h"

#include <ogg/ogg.h>

// complex means that individual streams from the item are
// picked
#define NET_HTTP_FILE_DRIVER_DOWNLOAD_STATE_FLAG_COMPLEX (1 << 0)

struct net_http_file_driver_download_state_t{
public:
	uint8_t flags = 0;

	bool first_packet = false;
	uint64_t packet_num = 0;
	/*
	  TODO: make the transcode namespace allow for encoding-to-file
	  (would also help out with WAVE)
	 */

	// offset in tv_item_t, last ID we processed
	ogg_stream_state *ogg_state = nullptr;
	std::vector<std::pair<uint8_t, id_t_> > service_log;
};

extern NET_HTTP_FILE_DRIVER_MEDIUM_INIT(download);
extern NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(download);
extern NET_HTTP_FILE_DRIVER_MEDIUM_LOOP(download);

#endif
