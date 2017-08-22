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
static std::vector<uint8_t> math_simple_add(
	std::vector<uint8_t> x_,
	std::vector<uint8_t> y_){
	std::pair<std::vector<uint8_t>, std::vector<uint8_t> >  raw_number_data;
	std::pair<std::vector<uint8_t>, std::vector<uint8_t> > x =
		math::number::get::raw_species(
			x_);
	std::pair<std::vector<uint8_t>, std::vector<uint8_t> > y =
		math::number::get::raw_species(
			x_);
	if(std::get<0>(x).size() == 0 ||
	   std::get<1>(x).size() == 0){
		return y_;
	}
	if(std::get<0>(y).size() == 0 ||
	   std::get<1>(y).size() == 0){
		return x_;
	}
	const uint64_t larger_size_minor =
		(std::get<1>(x).size() > std::get<1>(y).size()) ? std::get<1>(x).size()-1 : std::get<1>(y).size()-1;
	uint8_t carry = 0;
	// minor species
	for(uint64_t i = 0;i < larger_size_minor;i++){
		const uint8_t x_val = (std::get<1>(x).size() < i) ? 0 : std::get<1>(x)[i];
		const uint8_t y_val = (std::get<1>(y).size() < i) ? 0 : std::get<1>(y)[i];
		std::get<1>(raw_number_data).push_back(
			x_val+y_val+carry);
		int16_t carry_data =
			static_cast<uint16_t>(x_val)+static_cast<uint16_t>(y_val);
		if(carry_data > UINT8_MAX){
			carry = 1;
		}else if(carry_data < 0){
			carry = -1;
		}else{
			carry = 0;
		}
		P_V(carry, P_SPAM);
	}
	const uint64_t larger_size_major =
		(std::get<0>(x).size() > std::get<0>(y).size()) ? std::get<0>(x).size()-1 : std::get<0>(y).size()-1;
	// major species
	for(uint64_t i = 0;i < larger_size_major;i++){
		const uint8_t x_val = (std::get<0>(x).size() < i) ? 0 : std::get<1>(x)[i];
		const uint8_t y_val = (std::get<0>(y).size() < i) ? 0 : std::get<1>(y)[i];
		std::get<0>(raw_number_data).push_back(
			x_val+y_val+carry);
		int16_t carry_data =
			static_cast<uint16_t>(x_val)+static_cast<uint16_t>(y_val);
		if(carry_data > UINT8_MAX){
			carry = 1;
		}else if(carry_data < 0){
			carry = -1;
		}else{
			carry = 0;
		}
		P_V(carry, P_SPAM);
	}
	return math::number::create(
		raw_number_data.first,
		raw_number_data.second,
		math_unit_logic(
			math::number::get::unit(x_),
			math::number::get::unit(y_)));
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
