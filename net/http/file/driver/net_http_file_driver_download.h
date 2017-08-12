#ifndef NET_HTTP_FILE_DRIVER_DOWNLOAD_H
#define NET_HTTP_FILE_DRIVER_DOWNLOAD_H

#include "net_http_file_driver.h"

struct net_http_file_driver_download_state_t{
};

extern NET_HTTP_FILE_DRIVER_MEDIUM_INIT(download);
extern NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(download);
extern NET_HTTP_FILE_DRIVER_MEDIUM_PULL(download);

#endif
