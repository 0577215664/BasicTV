#include "net_http_parse.h"
#include "net_http.h"

#include "file/driver/net_http_file_driver.h" // macros

const static std::vector<uint8_t> http_header_divider = {'\r', '\n', '\r', '\n'};

std::string http::header::make_header(
	uint8_t medium,
	std::string mime_type,
	uint8_t payload_status, // full or incomplete,
	uint64_t payload_size){ // size of current payload (only used if payload_status == PAYLOAD_COMPLETE)
	std::string retval =
		"HTTP/1.1 200 OK\r\n"
		"Date: " + convert::time::to_http_time(get_time_microseconds()) + "\r\n"
		"Content-Type: " + mime_type + "\r\n"
		"Server: BasicTV\r\n";
		"Connection: close\r\n";
	switch(payload_status){
	case NET_HTTP_FILE_DRIVER_PAYLOAD_COMPLETE:
		retval += "Content-Length: " + std::to_string(payload_size) + "\r\n";
		break;
	case NET_HTTP_FILE_DRIVER_PAYLOAD_PROGRESS:
	default:
		break;
	}
	retval += "\r\n"; // end of header
	return retval;
}
	

std::vector<std::vector<std::string> > http::header::pull_from_socket(
	net_socket_t *socket_ptr){
	PRINT_IF_NULL(socket_ptr, P_ERR);

	std::vector<std::vector<std::string> > retval;

	// we can get the const and not have to set it again, since
	// we are only searching initially and, since we can't write,
	// we don't need to update the mod_inc
	std::vector<uint8_t> const* recv_buffer =
		socket_ptr->get_const_ptr_recv_buffer();
	
	const uint64_t header_start =
		std::distance(
			recv_buffer->begin(),
			std::search(
				recv_buffer->begin(),
				recv_buffer->end(),
				http_header_divider.begin(),
				http_header_divider.end()));
	
	if(header_start == recv_buffer->size()){
		return retval;
	}
	const std::string header_only =
		convert::string::from_bytes(
			socket_ptr->pull_erase_until_pos_recv_buffer(
				header_start+4));
	std::vector<std::string> tmp =
		convert::vector::vectorize_string_with_divider(
			header_only,
			"\n");
	for(uint64_t i = 0;i < tmp.size();i++){
		retval.push_back(
			convert::vector::vectorize_string_with_divider(
				tmp[i],
				" "));
	}
	// std::raise(SIGINT); // check against \r\n leaking into retval
	return retval;
}

std::string http::header::pull_value(
	std::vector<std::vector<std::string> > header,
	std::string data,
	uint8_t offset_from_first){
	for(uint64_t i = 0;i < header.size();i++){
		if(header.size() < 2){
			break;
		}
		if(header[i].size() <= offset_from_first){
			continue;
		}
		if(header[i][0] == data){
			return header[i][offset_from_first];
		}
	}
	// std::raise(SIGINT);
	print("can't find " + data + " in header", P_UNABLE);
}

// TODO: step through this and make sure it works, probably doesn't
std::vector<std::pair<std::string, std::string> > http::header::get::var_list(
	std::string path){
	std::vector<std::pair<std::string, std::string> > retval;
	const uint64_t var_start_pos =
		path.find_first_of('?');
	if(var_start_pos == std::string::npos){
		return retval;
	}
	path.erase(
		0,
		var_start_pos);
	uint64_t equal_pos = 0;
	uint64_t div_pos = 0;
	bool finding_var = true;
	while(finding_var){
		try{
			equal_pos =
				path.find(
					'=',
					var_start_pos);
			div_pos =
				path.find(
					'&',
					var_start_pos);
			if(equal_pos == std::string::npos ||
			   div_pos == std::string::npos){
				finding_var = false;
				break;
			}
			retval.push_back(
				std::make_pair(
					path.substr(
						0,
						equal_pos),
					path.substr(
						equal_pos+1,
						div_pos-equal_pos)));
			path.erase(
				0, div_pos-equal_pos);
				
		}catch(...){
			print("random exception caught in url_var", P_WARN);
			finding_var = false;
			break;
		}
	}
	return retval;
}

std::string http::header::get::value_from_var_list(
	std::vector<std::pair<std::string, std::string> > var_list,
	std::string var){
	for(uint64_t i = 0;i < var_list.size();i++){
		if(var_list[i].first == var){
			return var_list[i].second;
		}
	}
	return "";
}
