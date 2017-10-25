/*
  networking functions include downloading and uploading, along with caching 
  files. The cache shouldn't be overridden outside of this program, and a file
  becomes stale after 30 seconds. If a newer version is needed, then stale_time
  can be changed at will
 */

#include "net.h"
#include "net_cache.h"
#include "../main.h"
#include "../util.h"

#include "proto/net_proto.h"
#include "http/net_http.h"

static std::vector<net_cache_t> net_cache; // lock this

static size_t curl_write(void *ptr, size_t size, size_t nmemb, void *userdata){
	if(size*nmemb > CURL_MAX_WRITE_SIZE){
		print("size*nmemb > CURL_MAX_WRITE_SIZE", P_ERR);
	}
	uint64_t timestamp = time(NULL);
	std::string *url = (std::string*)userdata; // better way to do this?
	std::string data = std::string((char*)ptr, size*nmemb);
	int net_cache_entry = -1;
	for(unsigned int i = 0;i < net_cache.size();i++){
		if(net_cache.at(i).get_url() == *url){
			net_cache_entry = i;
			break;
		}
	}
	if(net_cache_entry == -1){
		// create new entry at end
		net_cache_entry = net_cache.size();
		net_cache.push_back(net_cache_t(*url, data, timestamp));
	}else{
		// append new data
		const std::string cache_ = net_cache.at(net_cache_entry).get_data();
		net_cache.at(net_cache_entry).set_data(cache_+data);
	}
	if(size*nmemb < CURL_MAX_WRITE_SIZE){
		net_cache.at(net_cache_entry).set_complete(true);
		*url = "";
		delete url;
		url = nullptr;
		userdata = nullptr;
	}
	return size*nmemb; // defined behavior
}

std::string net::get_cache(std::string url, int stale_time){
	const uint64_t timestamp = time(NULL);
	for(unsigned int i = 0;i < net_cache.size();i++){
		const std::string cache_url = net_cache.at(i).get_url();
		if(cache_url != url){
			continue;
		}
		const bool complete = net_cache.at(i).get_complete();
		if(complete == false){
			print("found incomplete, but valid cache", P_DEBUG);
			continue;
		}
		const uint64_t cached_time = net_cache.at(i).get_timestamp();
		const int elapsed_time = timestamp-cached_time;
		if(elapsed_time >= stale_time){
			print("found outdated cache, deleting", P_DEBUG);
			net_cache.erase(net_cache.begin()+i);
			continue;
		}
		const std::string cache_data = net_cache.at(i).get_data();
		return cache_data;
	}
	return "";
}

void net::force_url(std::string url){
	CURL *curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_write);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, new std::string(url));
	curl_easy_perform(curl_handle);
	curl_easy_cleanup(curl_handle);
	curl_handle = nullptr;
}

std::string net::get_url(std::string url, int stale_time){
	std::string retval = net::get_cache(url, stale_time);
	if(retval == ""){
		print("not found in cache, downloading", P_DEBUG);
		net::force_url(url);
		while((retval = net::get_cache(url, stale_time)) == ""){
			print("waiting for libcurl to finish", P_DEBUG);
			sleep_ms(50);
		}
	}
	return retval;
}

std::string net::get(std::string url, int stale_time){
	return net::get_url(url, stale_time);
}

/*
  TODO: get some backups for this, seriously
  canihazip.com/s has the plaintext stuff, which is nice
 */

std::string net_get_ip(){
	// std::string ip_addr =
	// 	net::get_url("checkip.dyndns.com");
	// ip_addr = ip_addr.substr(
	// 	ip_addr.find_first_of(":")+2,
	// 	ip_addr.find_last_of("<//body>")-strlen("<//body>"));
	return net::get_url("canihazip.com/s");
}

/*
  newer stuff, code above is pretty old and decrepit
 */

void net_init(){
	net_interface_init();
	net_proto_init();
	net_http_init();
}

void net_loop(){
	net_interface_loop();
	net_proto_loop();
	net_http_loop();
}

void net_close(){
	net_interface_close();
	net_proto_close();
	net_http_close();
}
