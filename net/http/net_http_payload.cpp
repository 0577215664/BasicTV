#include "net_http_payload.h"
#include "net_http_parse.h"
#include "net_http.h"

static net_http_chunk_header_t gen_standard_http_header(){
	net_http_chunk_header_t retval;
	// typedef std::pair<std::vector<std::pair<std::vector<std::string>, uint8_t> >, uint8_t>
	
	return retval;
}

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
			for(uint64_t b = 0;b < chunks.size();b++){
				// can't sent out of order
				ASSERT(chunks[b].get_sent() == false, P_ERR);
			}
			retval.insert(
				retval.end(),
				assembled_packet.begin(),
				assembled_packet.end());
		}
	}
	return retval;
}
