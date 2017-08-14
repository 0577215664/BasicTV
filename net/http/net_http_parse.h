#ifndef NET_HTTP_PARSE_H
#define NET_HTTP_PARSE_H

/*
  Only called inside batch loops for now, so we don't have to deal
  with passing IDs
 */

#include "../net_socket.h"

namespace http{
	namespace header{
		std::string make_header(
			std::string mime_type,
			uint8_t payload_status,
			uint64_t payload_size = 0);
		std::vector<std::vector<std::string> > pull_from_socket(
			net_socket_t *socket_ptr);
		std::string pull_value(
			std::vector<std::vector<std::string> > header,
			std::string data,
			uint8_t offset_from_first = 1);
		namespace get{
			std::vector<std::pair<std::string, std::string> > var_list(std::string url);
			std::string value_from_var_list(
				std::vector<std::pair<std::string, std::string> > var_list,
				std::string var);
			id_t_ pull_id(
				std::vector<std::pair<std::string, std::string> > var_list,
				std::string var);
		};
	};
};

#endif

