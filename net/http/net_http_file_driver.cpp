#include "net_http.h"
#include "net_http_file_driver.h"

#include "net_http_file_driver_atom.h"

static const std::vector<net_http_file_driver_medium_t> file_driver_medium =
{
	(net_http_file_driver_medium_t){NET_HTTP_FILE_DRIVER_MEDIUM_ATOM,
					"atom",
					net_http_file_driver_atom_init,
					net_http_file_driver_atom_close,
					net_http_file_driver_atom_pull}
};

net_http_file_driver_state_t::net_http_file_driver_state_t() : id(this, TYPE_NET_HTTP_FILE_DRIVER_STATE_T){
}

net_http_file_driver_state_t::~net_http_file_driver_state_t(){
}

net_http_file_driver_medium_t net_http_file_driver_get_medium(
	uint8_t medium){
	for(uint64_t i = 0;i < file_driver_medium.size();i++){
		if(file_driver_medium[i].medium == medium){
			return file_driver_medium[i];
		}
	}
	print("can't find HTTP file driver medium", P_ERR);
}

net_http_file_driver_medium_t net_http_file_driver_get_medium_from_url(
	std::string url){
	for(uint64_t i = 0;i < file_driver_medium.size();i++){
		if(file_driver_medium[i].min_valid_url.size() > url.size()){
			continue;
		}
		if(file_driver_medium[i].min_valid_url.substr(
			   0, file_driver_medium[i].min_valid_url.size()) ==
		   url.substr(
			   0, file_driver_medium[i].min_valid_url.size())){
			print("found valid match between HTTP GET min URL and medium", P_DEBUG);
			return file_driver_medium[i];
		}
	}
}
