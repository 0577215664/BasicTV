#include "net_http.h"

net_http_file_conn_instance_t::net_http_file_conn_instance_t(){
}

net_http_file_conn_instance_t::~net_http_file_conn_instance_t(){
}

net_http_file_t::net_http_file_t() : id(this, TYPE_NET_HTTP_FILE_T){
}

net_http_file_t::~net_http_file_t(){
}
