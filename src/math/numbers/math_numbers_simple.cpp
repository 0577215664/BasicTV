#include "math_numbers.h"
#include "../math.h"

/*
  Simple operations that access the number directly through
  the vector

  It is PRETTY DARN IMPORTANT that nothing here actually refers
  to another function, and that this entirely
 */

/*
  Might be a bit verbose and much, but it should work fine
 */

std::vector<uint8_t> math::number::simple::set_sign(
	std::vector<uint8_t> data,
	uint8_t sign){
	print("implement me", P_CRIT);
}

uint8_t math::number::simple::get_sign(
	std::vector<uint8_t> data){
	print("implement me", P_CRIT);
}

std::vector<uint8_t> math::number::simple::flip_sign(
	std::vector<uint8_t> data){
	print("implement me", P_CRIT);
}
