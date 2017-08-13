#ifndef NET_HTTP_FILE_DRIVER_DOWNLOAD_H
#define NET_HTTP_FILE_DRIVER_DOWNLOAD_H

#include "net_http_file_driver.h"

#define NET_HTTP_FILE_DRIVER_DOWNLOAD_STATE_FLAG_COMPLEX (1 << 0)

struct net_http_file_driver_download_state_t{
public:
	uint8_t flags = 0;
	/*
	  TODO: make the transcode namespace allow for encoding-to-file
	  (would also help out with WAVE)
	 */
};

extern NET_HTTP_FILE_DRIVER_MEDIUM_INIT(download);
extern NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(download);
extern NET_HTTP_FILE_DRIVER_MEDIUM_PULL(download);

#endif
