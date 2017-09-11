#include "../../net_http.h"
#include "net_http_file_driver.h"

#include "net_http_file_driver_atom.h"
#include "net_http_file_driver_frontpage.h"
#include "net_http_file_driver_download.h"

// TODO: make sure all lookups to file_driver_medium check for the longest
// valid min_url so everything doesn't get sent to the frontpage ("")

/*
  FRONTPAGE is a user interface to generated webpages

  ATOM is an Atom feed that takes channel_id as an input (through the
  service_id)

  DOWNLOAD serves out tv_item_ts that we have locally in a file format
  specified in the HTTP GET
 */

static std::vector<net_http_file_driver_medium_t> file_driver_medium =
{
	net_http_file_driver_medium_t(NET_HTTP_FILE_DRIVER_MEDIUM_FRONTPAGE,
				      "",
				      net_http_file_driver_frontpage_init,
				      net_http_file_driver_frontpage_close,
				      net_http_file_driver_frontpage_loop),
	net_http_file_driver_medium_t(NET_HTTP_FILE_DRIVER_MEDIUM_ATOM,
				      "atom",
				      net_http_file_driver_atom_init,
				      net_http_file_driver_atom_close,
				      net_http_file_driver_atom_loop),
	net_http_file_driver_medium_t(NET_HTTP_FILE_DRIVER_MEDIUM_DOWNLOAD,
				      "download",
				      net_http_file_driver_download_init,
				      net_http_file_driver_download_close,
				      net_http_file_driver_download_loop)
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
	return net_http_file_driver_medium_t(0, "", nullptr, nullptr, nullptr);
}

net_http_file_driver_medium_t net_http_file_driver_get_medium_from_url(
	std::string url){
	uint64_t best_fit_iter = ~static_cast<uint64_t>(0);
	for(uint64_t i = 0;i < file_driver_medium.size();i++){
		if(file_driver_medium[i].min_valid_url.size() > url.size()){
			continue;
		}
		const bool valid_with_current =
			file_driver_medium[i].min_valid_url.substr(
				0, file_driver_medium[i].min_valid_url.size()) ==
			url.substr(
				0, file_driver_medium[i].min_valid_url.size());
		if(valid_with_current){
			const bool improvement =
				best_fit_iter == ~static_cast<uint64_t>(0) ||
				file_driver_medium[i].min_valid_url.size() >
				file_driver_medium[best_fit_iter].min_valid_url.size();
			if(improvement){
				best_fit_iter = i;
			}
		}
	}
	if(best_fit_iter == ~static_cast<uint64_t>(0)){
		print("can't find a valid file_driver_medium", P_ERR);
		std::raise(SIGINT);
	}
	return file_driver_medium[best_fit_iter];
}
