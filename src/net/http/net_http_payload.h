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

/*
  Header is just a vectorized prefix to the packet, so borders
  and other stuff can go there, even if it isn't strictly a
  HTTP request/response header

  Any HTTP header information encoded as a string isn't put inline
  in the payload, HOWEVER, any OTF generated information is put
  wherever the first half is
 */

struct net_http_chunk_header_t{
private:
	std::vector<std::vector<std::string> > payload;
	std::string major_divider; // \r\n
	std::string minor_divider; // space
	std::string mime_type; // should I break off into major and minor?

public:
	net_http_chunk_header_t(
		std::vector<std::vector<std::string> > payload_,
		std::string major_divider_,
		std::string minor_divider_,
		std::string mime_type_){
		payload = payload_;
		major_divider = major_divider_;
		minor_divider = minor_divider_;
		mime_type = mime_type_;
	}

	net_http_chunk_header_t(){}
	
	void set_payload(std::vector<std::vector<std::string> > payload_);
	std::vector<std::vector<std::string> > get_payload();

	GET_SET_S(major_divider, std::string);
	GET_SET_S(minor_divider, std::string);

	GET_SET_S(mime_type, std::string);
	
	// special types
	// if not blank, these are automatically appended as
	// needed to wherever in the header
	// only enter the actual data into here, the key component is added
	// laster on
	
	// header needs to be in readable ASCII
	std::string assemble();

	std::vector<std::string> fetch_line_from_start(
		std::string start);
	
	bool operator==(const net_http_chunk_header_t &rhs) const {
		return payload == rhs.payload &&
		major_divider == rhs.major_divider &&
		minor_divider == rhs.minor_divider;
	}

	bool operator!=(const net_http_chunk_header_t &rhs){
		return !(*this == rhs);
	}
};

// Most requests are going to use one chunk with all the data inline
// Files and multipart forms use multiple chunks, and set any border
// information in the header

struct net_http_chunk_t{
private:
	std::vector<uint8_t> payload;
	std::string boundary;
	// technically these can be nested per HTTP, but it's not a big deal
	// if it doesn't here, since only one file in one form enables this
	// at a time
	bool sent = false;

public:
	net_http_chunk_t();
	~net_http_chunk_t();
	net_http_chunk_header_t header;

	bool operator==(const net_http_chunk_t &rhs) const {
		return header == rhs.header &&
		payload == rhs.payload &&
		sent == rhs.sent;
	}

	bool operator!=(const net_http_chunk_t &rhs) const {
		return ~(*this == rhs);
	}
	
	GET_SET_S(header, net_http_chunk_header_t);
	GET_SET_S(payload, std::vector<uint8_t>);
	GET_SET_S(sent, bool);

	std::vector<uint8_t> assemble();
};

typedef std::pair<std::string, std::string> net_http_form_data_entry_t;

struct net_http_form_data_t{
private:
	std::vector<std::pair<std::string, std::string> > table;

public:
	GET_SET_S(table, std::vector<net_http_form_data_entry_t>);
	ADD_DEL_VECTOR_S(table, net_http_form_data_entry_t);
	
	// useful getters
	std::string get_str(std::string key);
	id_t_ get_id(std::string key);
	int64_t get_int(std::string key);
};

struct net_http_payload_t{
private:
	uint8_t direction = NET_HTTP_PAYLOAD_UNDEFINED;
	std::vector<net_http_chunk_t> chunks;

	bool finished = false;
public:
	net_http_payload_t();
	~net_http_payload_t();

	net_http_form_data_t form_data;
	
	ADD_DEL_VECTOR_S(chunks, net_http_chunk_t);
	GET_SET_S(chunks, std::vector<net_http_chunk_t>);
	GET_SET_S(direction, uint8_t);
	GET_SET_S(finished, uint8_t);
	std::vector<uint8_t> pull(); // adds boundaries

	bool operator==(const net_http_payload_t &rhs) const {
		return direction == rhs.direction &&
		chunks == rhs.chunks &&
		finished == rhs.finished;
	}

	bool operator!=(const net_http_payload_t &rhs) const {
		return !(*this == rhs);
	}
};

#endif
