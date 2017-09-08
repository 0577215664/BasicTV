#include "net_http_payload.h"
#include "net_http_parse.h"
#include "net_http.h"

net_http_chunk_t::net_http_chunk_t(){
}

net_http_chunk_t::~net_http_chunk_t(){
}

std::vector<uint8_t> net_http_chunk_t::assemble(){
	std::vector<uint8_t> retval;
	for(uint64_t a = 0;a < header.first.size();a++){
		for(uint64_t b = 0;b < header.first[a].first.size();b++){
			retval.insert(
				retval.end(),
				header.first[a].first[b].data(),
				header.first[a].first[b].data()+
				header.first[a].first[b].size());
			retval.push_back(header.first[a].second);
		}
		retval.push_back(header.first[a].second);
	}
	return retval;
}

net_http_payload_t::net_http_payload_t(){
}

net_http_payload_t::~net_http_payload_t(){
}

std::vector<uint8_t> net_http_payload_t::pull(){
	std::vector<uint8_t> retval;
	for(uint64_t i = 0;i < chunks.size();i++){
		if(chunks[i].get_sent() == false){
			const std::vector<uint8_t> assembled_packet =
				chunks[i].assemble();
			retval.insert(
				retval.end(),
				assembled_packet.begin(),
				assembled_packet.end());
		}
	}
	return retval;
}
