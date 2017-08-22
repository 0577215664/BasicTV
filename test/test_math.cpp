#include "test.h"
#include "test_math.h"

#include "../math/math.h"

void test::math::number::add(){
	std::vector<uint8_t> number =
		::math::number::create(
			0UL, UNIT(0, 0, 0));
	uint64_t sys_num = 0;
	for(uint64_t i = 0;i < 65536;i++){
		sys_num++;
		number = ::math::number::calc::add(
			{number,
			::math::number::create(
					static_cast<uint64_t>(1UL),
					UNIT(0, 0, 0))});
		ASSERT(static_cast<uint64_t>(::math::number::get::number(number)) == sys_num, P_ERR);
	}
}

void test::math::number::sub(){
	std::vector<uint8_t> number =
		::math::number::create(
			0L, UNIT(0, 0, 0));
	uint64_t sys_num = 0;
	for(uint64_t i = 0;i < 65536;i++){
		sys_num--;
		number = ::math::number::calc::add(
			{number,
			::math::number::create(
					static_cast<int64_t>(-1),
					UNIT(0, 0, 0))});
		ASSERT(static_cast<uint64_t>(::math::number::get::number(number)) == sys_num, P_ERR);
	}
}
