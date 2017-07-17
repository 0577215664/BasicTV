#include "escape.h"
#include "util.h"

// just adds size for decoding

std::vector<uint8_t> escape_vector(
	std::vector<uint8_t> vector,
	uint8_t escape_char){
	for(uint64_t i = 0;i < vector.size();i++){
		if(unlikely(vector[i] == escape_char)){
			vector.insert(
				vector.begin()+i,
				escape_char);
			while(i < vector.size() &&
			      vector[i] == escape_char){
				i++;
			}
			i--;
		}
	}
	// insert the escaped size as a prefix
	uint32_t escaped_length =
		NBO_32(vector.size());
	vector.insert(
		vector.begin(),
		((uint8_t*)&escaped_length),
		((uint8_t*)&escaped_length)+4);
	vector.insert(
		vector.begin(),
		&(escape_char),
		&(escape_char)+1);
	return vector;
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t> > unescape_vector(
	std::vector<uint8_t> vector,
	uint8_t escape_char){

	std::pair<std::vector<uint8_t>, std::vector<uint8_t> > retval;
	std::vector<uint8_t> payload;

	ASSERT(vector[0] == escape_char, P_ERR);
	if(vector.size() <= 5){
		return std::make_pair(
			std::vector<uint8_t>({}),
			vector);
	}
	uint32_t escaped_length =
		NBO_32(*reinterpret_cast<uint32_t*>(
			       &(vector[1])));
	if(escaped_length+sizeof(uint32_t)+sizeof(uint8_t) <= vector.size()){
		vector.erase(
			vector.begin(),
			vector.begin()+sizeof(uint32_t)+sizeof(uint8_t));
		payload = std::vector<uint8_t>(
			vector.begin(),
			vector.begin()+escaped_length);
		vector.erase(
			vector.begin(),
			vector.begin()+escaped_length);
	}
	for(uint64_t i = 0;i < payload.size();i++){
		if(payload[i] == escape_char){
			payload.erase(
				payload.begin()+i);
			while(i < payload.size() &&
			      payload[i] == escape_char){
				i++;
			}
			i--;
		}
	}
	return std::make_pair(
		payload, vector);
}

std::pair<std::vector<std::vector<uint8_t> >, std::vector<uint8_t> > unescape_all_vectors(
	std::vector<uint8_t> vector,
	uint8_t escape_char){
	// list of all exported vectors and the extra
	std::pair<std::vector<std::vector<uint8_t> >, std::vector<uint8_t> > retval;
	uint64_t old_size = 0;
	while(vector.size() != old_size){
		old_size = vector.size();
		std::pair<std::vector<uint8_t>, std::vector<uint8_t> > tmp =
			unescape_vector(vector, escape_char);
		if(tmp.first.size() != 0){
			retval.first.push_back(
				tmp.first);
		}
		vector = tmp.second;
	}
	if(vector.size() != 0){
		print("non-zero vector size at end of bulk vector unescaping, returning as cruft", P_SPAM);
	}
	retval.second = vector;
	return retval;
}
