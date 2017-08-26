#include "../../main.h"
#include "../../util.h"
#include "math_numbers.h"
#include "math_numbers_calc.h"
#include "../math.h"

static void math_number_same_units(std::vector<std::vector<uint8_t> > data){
	uint64_t unit = 0;
	if(data.size() == 0){
		print("null set of numbers have compatiable units, but that's weird", P_NOTE);
		// probably not a warnable offense, but it might help
	}else{
		unit = math::number::get::unit(data[0]);
	}
	for(uint64_t i = 1;i < data.size();i++){
		if(unlikely(math::number::get::unit(data[i]) !=
			    unit)){
			print("incompat units, fail compat test", P_ERR);
		}
	}
	print("numbers appear to have sane units", P_DEBUG);
}

static uint64_t math_unit_logic(
	uint64_t x_unit,
	uint64_t y_unit){
	if(x_unit == 0 && y_unit != 0){
		return y_unit;
	}else if(x_unit != 0 && y_unit == 0){
		return x_unit;
	}else if(x_unit != 0 && y_unit != 0){
		if(x_unit == y_unit){
			return x_unit;
		}else{
			/*
			  Units are dropped here, more complicated unit logic
			  is taken care of in the caller
			 */
			print("unit mismatch", P_WARN);
			return 0;
		}
	}
	return 0;
}

// TODO: can optimize this a lot with 128-bit operations
/*
  This can subtract as well, so base all other functions off of this and
  comparison functions
 */

// made for carry specifically, but can be used for pretty much anything
static void math_simple_add_byte(std::vector<uint8_t> *vector,
				 int8_t byte){
	for(uint64_t i = 0;i < vector->size()-1;i++){
		if(likely(static_cast<int16_t>((*vector)[i]) + static_cast<int16_t>(byte) <= UINT8_MAX)){
			(*vector)[i] += byte;
			return;
		}
	}
	if(likely(static_cast<int16_t>((*vector)[vector->size()-1]) + static_cast<int16_t>(byte) <= UINT8_MAX)){
		(*vector)[vector->size()-1] += byte;
	}else{
		vector->push_back(
			byte);
	}
}

#define PULL_IF_VALID(x, y) (x.size() > y) ? x[y] : 0

#define SIGN_POS true
#define SIGN_NEG false

// do math as signed of a higher bit 

static std::vector<uint8_t> math_add_raw_unsigned_species(
	std::vector<uint8_t> x,
	std::vector<uint8_t> y,
	bool sign){
	bool computing = true;
	if(x.size() < y.size()){
		std::swap(x, y);
	}
	const bool x_sign = math::number::simple::get_sign();
	for(uint64_t i = 0;i <
	return x;
}

static std::vector<uint8_t> math_simple_add(
	std::vector<uint8_t> x,
	std::vector<uint8_t> y){
}

std::vector<uint8_t> math::number::calc::add(
	std::vector<std::vector<uint8_t> > data){
	math_number_same_units(data);
	if(unlikely(data.size() == 0)){
		return {};
	}
	std::vector<uint8_t> retval =
		math::number::create(
			(int64_t)0,
			math::number::get::unit(
				data[0]));
	std::raise(SIGINT);
	for(uint64_t i = 0;i < data.size();i++){
		retval =
			math_simple_add(
				retval,
				data[i]);
	}
	return retval;
}


std::vector<uint8_t> math::number::calc::sub(
	std::vector<std::vector<uint8_t> > data){
	std::vector<uint8_t> retval =
		data[0];
	for(uint64_t i = 1;i < data.size();i++){
		retval =
			math_simple_add(
				retval,
				math::number::simple::flip_sign(
					data[i]));
	}
	return retval;
}
