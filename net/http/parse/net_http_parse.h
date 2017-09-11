#ifndef NET_HTTP_PARSE_H
#define NET_HTTP_PARSE_H

/*
  Only called inside batch loops for now, so we don't have to deal
  with passing IDs
 */

#include "../../net_socket.h"
#include "../net_http_payload.h"

/*
  HTTP GET and HTTP POST are actually combined into the same list.
  HTTP POST variables take precedence over HTTP GET if there is
  a conflict.

  TODO: should make a stark difference between HTTP GET and
  HTTP POST. Reserve HTTP POST for forms, and HTTP GET for
  variables for playback (service_id)
 */

namespace http{
	namespace raw{
		std::string get_item_from_line(
			std::vector<std::vector<std::string> > payload,
			std::string search,
			uint8_t offste_from_first);
	};
	namespace socket{
		namespace payload{
			void read(
				net_http_payload_t *payload,
				id_t_ socket_id);
			void write(
				net_http_payload_t *payload,
				id_t_ socket_id);
		};
	};
	namespace tags{
		std::string html_table(
			std::vector<std::vector<std::string> > table);
	};
};

const net_http_chunk_header_t standard_header =
{
	net_http_chunk_header_t(
		std::vector<std::vector<std::string> >({
				{"HTTP/1.1", "200", "OK"},
				{"Date:"},
				{"Content-Type: "},
				{"Server:", "BasicTV"},
				{"Connnection:", "close"}}),
		"\r\n",
		" ",
		"text/html")
};

#endif

