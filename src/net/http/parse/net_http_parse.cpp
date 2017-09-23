#include "net_http_parse.h"
#include "../net_http.h"

#include "../file/driver/net_http_file_driver.h" // macros

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

static std::vector<std::pair<std::string, std::string> > net_http_parse_urlencoded(
	std::string str){
	// truncation is handled in the caller
	std::vector<std::pair<std::string, std::string> > retval;
	uint64_t old_size = 0;
	while(str.size() != old_size){
		old_size = str.size();
		const std::string key_half =
			str.substr(
				0,
				str.find_first_of('='));
		str.erase(
			0,
			str.find_first_of('=')+1);
		uint64_t end =
			str.find_first_of('&');
		if(end == std::string::npos){
			end = str.size();
		}
		const std::string value_half =
			str.substr(
				0,
				end);
		P_V_S(key_half, P_VAR);
		P_V_S(value_half, P_VAR);
		retval.push_back(
			std::make_pair(
				key_half,
				value_half));
		str.erase(
			0,
			end+1);
	}
	return retval;
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
	for(uint64_t a = 0;a < retval.size();a++){
		P_V(a, P_VAR);
		for(uint64_t b = 0;b < retval[a].size();b++){
			P_V_S(retval[a][b], P_VAR);
		}

	}
	return retval;
}

// x-www-form-urlencoded either in POST or GET
// POST data is seperate from the HTTP header by one blank line
// TODO: I think I put an assertion in to stop that from working...

static net_http_form_data_t net_http_form_data_from_header(
	std::vector<std::vector<std::string> > header){
	std::vector<std::pair<std::string, std::string> > form_table_raw;
	try{
		std::string url =
			http::raw::get_item_from_line(
				header,
				"GET",
				1);
		const uint64_t get_start =
			url.find_first_of('?');
		if(get_start != std::string::npos){ // correct, right?
			const std::string url_substr =
				url.substr(
					get_start+1,
					url.size());
			P_V_S(url_substr, P_VAR);
			const std::vector<std::pair<std::string, std::string> > tmp_form_data =
				net_http_parse_urlencoded(
					url_substr);
			form_table_raw.insert(
				form_table_raw.end(),
				tmp_form_data.begin(),
				tmp_form_data.end());
		}
	}catch(...){}
	try{
		const bool post_exists =
			std::find_if(
				header.begin(),
				header.end(),
				[&](const std::vector<std::string> &elem){
					return elem.size() == 0; // only doubles are removed
				}) != header.end();
		if(post_exists){
			const std::vector<std::pair<std::string, std::string> > tmp_form_data =
				net_http_parse_urlencoded(
					header[header.size()-1].at(0));
			form_table_raw.insert(
				form_table_raw.end(),
				tmp_form_data.begin(),
				tmp_form_data.end());
		}
	}catch(...){}

	P_V(form_table_raw.size(), P_VAR);
	for(uint64_t i = 0;i < form_table_raw.size();i++){
		P_V_S(form_table_raw[i].first, P_VAR);
		P_V_S(form_table_raw[i].second, P_VAR);
	}
	
	net_http_form_data_t retval;
	retval.set_table(
		form_table_raw);
	return retval;
}

#pragma message("http::socket::payload::read doesn't support files yet")

/*
  NOTE: probably wasn't a good idea to put boundary inside of net_http_payload_t,
  but there is no good reason to allow more than one file at a time right now.

  How files are going to work:
  'Content-Type: multipart/form-data; boundary=[BOUNDARY]' or whatever is
  read directly into the net_http_payload_t

  Individual chunk headers contain the Content-Disposition, Content-Type, etc,
  and are escaped like a normal HTTP header.

  When the file has been read in, it'll be inside the form_data table
 */

void http::socket::payload::read(
	net_http_payload_t *payload,
	id_t_ socket_id){
	net_socket_t *socket_ptr =
		PTR_DATA(socket_id,
			 net_socket_t);
	PRINT_IF_NULL(socket_ptr, P_ERR);

	std::vector<net_http_chunk_t> chunk_vector =
		payload->get_chunks();

	if(chunk_vector.size() == 0){
		chunk_vector.push_back(net_http_chunk_t());
	}
	
	const std::vector<std::vector<std::string> > tmp_header =
		net_http_parse_pull_header_from_socket(
			socket_ptr);
	if(tmp_header.size() == 0){
		return;
	}
	const net_http_form_data_t form_data =
		net_http_form_data_from_header(
			tmp_header);
	
	chunk_vector[chunk_vector.size()-1].header.set_payload(tmp_header);
	chunk_vector[chunk_vector.size()-1].header.set_major_divider("\r\n"); // standard
	chunk_vector[chunk_vector.size()-1].header.set_minor_divider(" ");

	// TODO: to allow for files, I need to allow a hot-add
	payload->set_chunks(chunk_vector);
	payload->form_data.append_table(
		form_data.get_table());
	
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

std::string http::tags::html_table(
	std::vector<std::vector<std::string> > table){
	std::string retval;
	for(uint64_t a = 0;a < table.size();a++){
		for(uint64_t b = 0;b < table[a].size();b++){
			retval += "<th>" + table[a][b] + "<//th>";
		}
		retval = "<tr>" + retval + "<//tr>";
	}
	retval += "<table>" + retval + "<//table>";
	return retval;
}
