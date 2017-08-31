#ifndef MATH_NUMBER_SET_H
#define MATH_NUMBER_SET_H

#include "../../id/id.h"
#include "math_number_set_simple.h"

struct math_number_set_t{
private:
	math_number_set_simple_t number_set;
public:
	data_id_t id;

	math_number_set_t();
	~math_number_set_t();
	
	// wrappers for number_set for simplicity
	void set_height(uint16_t height, std::vector<uint8_t> prop){number_set.set_height(height, prop);}
	uint16_t get_height(){return number_set.get_height();}
	void add_data(std::vector<std::vector<uint8_t> > data_){number_set.add_data(data_);}
	std::vector<std::vector<uint8_t> > get_data(){return number_set.get_data();}
};

#endif
