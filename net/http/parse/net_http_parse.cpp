#include "net_http_parse.h"
#include "../net_http.h"

#include "../file/driver/net_http_file_driver.h" // macros

// // TODO: step through this and make sure it works, probably doesn't
// std::vector<std::pair<std::string, std::string> > http::header::get::var_list(
// 	std::string path){
	
// 	std::vector<std::pair<std::string, std::string> > retval;
// 	uint64_t var_start_pos =
// 		path.find_first_of('?');
// 	if(var_start_pos == std::string::npos){
// 		return retval;
// 	}else{
// 		var_start_pos++;
// 	}
//  	path.erase(
// 		0,
// 		var_start_pos);
// 	uint64_t equal_pos = 0;
// 	uint64_t div_pos = 0;
// 	bool finding_var = true;
// 	while(finding_var){
// 		try{
// 			equal_pos =
// 				path.find(
// 					'=',
// 					var_start_pos);
// 			div_pos =
// 				path.find(
// 					'&',
// 					var_start_pos);
// 			if(equal_pos == std::string::npos){
// 				finding_var = false;
// 				break;
// 			}
// 			if(div_pos == std::string::npos){
// 				div_pos = path.size();
// 			}
// 			retval.push_back(
// 				std::make_pair(
// 					path.substr(
// 						0,
// 						equal_pos),
// 					path.substr(
// 						equal_pos+1,
// 						div_pos-equal_pos)));
// 			path.erase(
// 				0, div_pos+1);
				
// 		}catch(...){
// 			print("random exception caught in url_var", P_WARN);
// 			finding_var = false;
// 			break;
// 		}
// 	}
// 	P_V(retval.size(), P_VAR);
// 	for(uint64_t i = 0;i < retval.size();i++){
// 		P_V_S(retval[i].first, P_VAR);
// 		P_V_S(retval[i].second, P_VAR);
// 	}
// 	return retval;
// }

// std::string http::header::get::value_from_var_list(
// 	std::vector<std::pair<std::string, std::string> > var_list,
// 	std::string var){
// 	for(uint64_t i = 0;i < var_list.size();i++){
// 		if(var_list[i].first == var){
// 			return var_list[i].second;
// 		}
// 	}
// 	print("value not found in var list", P_UNABLE);
// 	return "";
// }

// id_t_ http::header::get::pull_id(
// 	std::vector<std::pair<std::string, std::string> > var_list,
// 	std::string var){
// 	id_t_ retval = ID_BLANK_ID;
// 	try{
// 		retval =
// 			convert::array::id::from_hex(
// 				http::header::get::value_from_var_list(
// 					var_list,
// 					var));
// 	}catch(...){
// 		print("can't interpret ID, returning blank", P_WARN);
// 	}
// 	return retval;
// }

/*
  We use Connection: close for all HTTP requests, so we can safely assume
  that only one HTTP request/respone will go over a socket
 */

std::string http::raw::get_item_from_line(
	std::vector<std::vector<std::string> > payload,
	std::string search,
	uint8_t offset_from_first){
	for(uint64_t i = 0;i < payload.size();i++){
		if(payload.size() < 2){
			break;
		}
		if(payload[i].size() <= offset_from_first){
			continue;
		}
		if(payload[i][0] == search){
			return payload[i][offset_from_first];
 		}
 	}
 	// std::raise(SIGINT);
 	print("can't find " + search + " in header", P_UNABLE);
 	return "";
}

static std::vector<std::vector<std::string> > net_http_parse_pull_header_from_socket(
	net_socket_t *socket_ptr){
	PRINT_IF_NULL(socket_ptr, P_ERR);

	std::vector<std::vector<std::string> > retval;

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
	return retval;
}

void http::socket::payload::read(
	net_http_payload_t *payload,
	id_t_ socket_id){
	net_socket_t *socket_ptr =
		PTR_DATA(socket_id,
			 net_socket_t);
	PRINT_IF_NULL(socket_ptr, P_ERR);

	if(payload->get_size_chunks() == 0){
		const std::vector<std::vector<std::string> > tmp_header =
			net_http_parse_pull_header_from_socket(
				socket_ptr);
		if(tmp_header.size() == 0){
			return;
		}
		net_http_chunk_t http_chunk;
		http_chunk.header.set_payload(tmp_header);
		http_chunk.header.set_major_divider("\r\n"); // standard
		http_chunk.header.set_minor_divider(" ");
		payload->add_chunks(http_chunk);
	}

	// just keep adding to the payload until the socket closes, then
	// mark it as finished I guess
	
}

void http::socket::payload::write(
	net_http_payload_t *payload,
	id_t_ socket_id){
	net_socket_t *socket_ptr =
		PTR_DATA(socket_id,
			 net_socket_t);
	PRINT_IF_NULL(socket_ptr, P_ERR);

	socket_ptr->send(
		payload->pull());
}
