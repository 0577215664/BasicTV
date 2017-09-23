#ifndef NET_HTTP_FILE_DRIVER_FRONTPAGE_H
#define NET_HTTP_FILE_DRIVER_FRONTPAGE_H

#include "net_http_file_driver.h"

struct net_http_file_driver_frontpage_state_t{
};

extern NET_HTTP_FILE_DRIVER_MEDIUM_INIT(frontpage);
extern NET_HTTP_FILE_DRIVER_MEDIUM_CLOSE(frontpage);
extern NET_HTTP_FILE_DRIVER_MEDIUM_LOOP(frontpage);

#endif
