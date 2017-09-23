#include "net_http_payload.h"
#include "parse/net_http_parse.h"
#include "net_http.h"

std::string net_http_chunk_header_t::assemble(){
	std::vector<std::vector<std::string> > tmp;
	for(uint64_t a = 0;a < payload.size();a++){
		tmp.push_back(
			std::vector<std::string>({}));
		for(uint64_t b = 0;b < payload[a].size();b++){
			tmp[a].push_back(payload[a][b]);
			if(payload[a][b].find("Date") != std::string::npos){
				print("auto-inserting date", P_SPAM);
				tmp[a].push_back(
					convert::time::to_http_time(get_time_microseconds()));
			}
			if(payload[a][b].find("Content-Type") != std::string::npos){
				print("auto-inserting mime type", P_SPAM);
				tmp[a].push_back(
					mime_type);
			}
		}
	}

	std::string retval;
	for(uint64_t a = 0;a < tmp.size();a++){
		for(uint64_t b = 0;b < tmp[a].size();b++){
			retval += tmp[a][b];
			retval += minor_divider;
		}
		retval += major_divider;
	}

	P_V_S(retval, P_VAR);
	return retval;
}

std::vector<std::string> net_http_chunk_header_t::fetch_line_from_start(
	std::string start){
	for(uint64_t i = 0;i < payload.size();i++){
		if(payload[i].size() == 0){
			print("payload line is blank, shouldn't happen", P_WARN);
			continue;
		}
		if(payload[i][0] == start){
			return payload[i];
		}
	}
	return std::vector<std::string>({});
}

void net_http_chunk_header_t::set_payload(std::vector<std::vector<std::string> > payload_){
	try{
		std::string mime_type_ =
			http::raw::get_item_from_line(
				payload_,
				"Content-Type:",
				1);
		if(mime_type_[mime_type_.size()-1] == ';'){
			mime_type_.erase(
				mime_type_.end());
		}
		mime_type = mime_type_;
	}catch(...){}
	payload = payload_;
}

net_http_chunk_t::net_http_chunk_t(){
}

net_http_chunk_t::~net_http_chunk_t(){
}

std::vector<uint8_t> net_http_chunk_t::assemble(){
	std::vector<uint8_t> retval;
	const std::string assembled_header =
		header.assemble();
	retval.insert(
		retval.end(),
		assembled_header.data(),
		assembled_header.data()+
		assembled_header.size());
	retval.insert(
		retval.end(),
		http_header_divider.begin(),
		http_header_divider.end());
	retval.insert(
		retval.end(),
		payload.begin(),
		payload.end());
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
			for(uint64_t b = i+1;b < chunks.size();b++){
				// can't sent out of order
				ASSERT(chunks[b].get_sent() == false, P_ERR);
			}
			retval.insert(
				retval.end(),
				assembled_packet.begin(),
				assembled_packet.end());
			retval.insert(
				retval.end(),
				http_header_divider.begin(),
				http_header_divider.end());
			chunks[i].set_sent(true);
			print("adding chunk " + std::to_string(i), P_WARN);
		}
	}
	return retval;
}

std::string net_http_form_data_t::get_str(std::string key){
	for(uint64_t i = 0;i < table.size();i++){
		if(table[i].first == key){
			return table[i].second;
		}
	}
	print("can't find key in table", P_UNABLE);
}

id_t_ net_http_form_data_t::get_id(std::string key){
	return convert::array::id::from_hex(
		get_str(key));
}

int64_t net_http_form_data_t::get_int(std::string key){
	return std::stoi(get_str(key));
}
