#ifndef NET_HTTP_PAYLOAD_H
#define NET_HTTP_PAYLOAD_H

#include "../../id/id.h"

/*
  Standard interface for any HTTP transaction

  Made for files, but can be applied to standard transactions
  given enough time

  HTTP chunk_t
 */

#define NET_HTTP_PAYLOAD_UNDEFINED 0
#define NET_HTTP_PAYLOAD_IN 1
#define NET_HTTP_PAYLOAD_OUT 2

#define NET_HTTP_PAYLOAD_CHUNK_SENT (1 << 0)

typedef std::pair<std::vector<std::pair<std::vector<std::string>, uint8_t> >, uint8_t> net_http_header_chunk_header_t;

struct net_http_chunk_t{
private:
	net_http_header_chunk_header_t header;
	std::vector<uint8_t> payload;
	bool sent = false;

public:
	net_http_chunk_t();
	~net_http_chunk_t();

	bool operator==(const net_http_chunk_t &rhs) const {
		return header == rhs.header &&
		payload == rhs.payload &&
		sent == rhs.sent;
	}

	bool operator!=(const net_http_chunk_t &rhs) const {
		return ~(*this == rhs);
	}
	
	GET_SET_S(header, net_http_header_chunk_header_t);
	GET_SET_S(payload, std::vector<uint8_t>);
	GET_SET_S(sent, bool);

	std::vector<uint8_t> assemble();
};

struct net_http_payload_t{
private:
	uint8_t direction = NET_HTTP_PAYLOAD_UNDEFINED;
	std::vector<net_http_chunk_t> chunks;
	std::string divider;

public:
	net_http_payload_t();
	~net_http_payload_t();

	ADD_DEL_VECTOR_S(chunks, net_http_chunk_t);
	GET_SET_S(direction, uint8_t);

	std::vector<uint8_t> pull();
};

#endif
