#include "net_http.h"
#include "net_http_file_driver.h"

#include "net_http_file_driver_atom.h"

static const std::vector<net_http_file_driver_medium_t> file_driver_medium =
{
	(net_http_file_driver_medium_t){NET_HTTP_FILE_DRIVER_MEDIUM_ATOM,
					net_http_file_driver_atom_init,
					net_http_file_driver_atom_close,
					net_http_file_driver_atom_pull}
};

net_http_file_driver_state_t::net_http_file_driver_state_t() : id(this, TYPE_NET_HTTP_FILE_DRIVER_STATE_T){
}

net_http_file_driver_state_t::~net_http_file_driver_state_t(){
}

id_t_ net_http_file_driver_state_from_url(std::vector<uint8_t> url){
	std::vector<id_t_> file_driver_state_vector =
		ID_TIER_CACHE_GET(
			TYPE_NET_HTTP_FILE_DRIVER_STATE_T);
	for(uint64_t i = 0;i < file_driver_state_vector.size();i++){
		net_http_file_driver_state_t *file_state_ptr =
			PTR_DATA(file_driver_state_vector[i],
				 net_http_file_driver_state_t);
		CONTINUE_IF_NULL(file_state_ptr, P_WARN);
		std::vector<uint8_t> min_url =
			file_state_ptr->get_min_valid_url();
		if(url.size() < min_url.size()){
			continue;
		}
		if(std::memcmp(url.data(), min_url.data(), min_url.size()) == 0){
			return file_state_ptr->id.get_id();
		}
	}
	return ID_BLANK_ID;
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
