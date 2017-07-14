#include "test.h"

#include "../escape.h"

void test::escape::proper(){
	const std::vector<uint8_t> payload =
		true_rand_byte_vector(
			true_rand(
				0, 65536));
	std::pair<std::vector<uint8_t>, std::vector<uint8_t> > unescaped =
		unescape_vector(
			escape_vector(
				payload, 0x80),
			0x80);
	if(unescaped.first.size() != payload.size()){
		P_V(unescaped.first.size(), P_WARN);
		P_V(payload.size(), P_WARN);
		ASSERT(unescaped.first.size() == payload.size(), P_ERR);
	}
	ASSERT(unescaped.first == payload, P_ERR);
	ASSERT(unescaped.second.size() == 0, P_ERR);
}
