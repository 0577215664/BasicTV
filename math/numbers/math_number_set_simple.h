#ifndef MATH_NUMBER_SET_SIMPLE_H
#define MATH_NUMBER_SET_SIMPLE_H

#include "../../id/id.h"
#include "math_number_units.h"

struct math_number_set_simple_t{
private:
	uint16_t height = 0;
	std::vector<uint8_t> prop;
	std::vector<std::vector<uint8_t> > data;
	
	data_id_t *id = nullptr;
public:
	math_number_set_simple_t();
	~math_number_set_simple_t();
	void list_virtual_data(data_id_t *id_);
	void set_height(uint16_t height_, std::vector<uint8_t> prop);
	GET_V(height, uint16_t);

	void add_data(std::vector<std::vector<uint8_t> > data_);
	GET_V(data, std::vector<std::vector<uint8_t> >);
};

#endif
