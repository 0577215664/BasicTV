#include "math_number_set_simple.h"
#include "math_number_set.h"

math_number_set_simple_t::math_number_set_simple_t(){
}

math_number_set_simple_t::~math_number_set_simple_t(){
}

void math_number_set_simple_t::add_data(std::vector<std::vector<uint8_t> > data_){
	ASSERT(data_.size() == height, P_ERR);
	ASSERT(prop.size() == height, P_ERR);
	for(uint64_t i = 0;i < height;i++){
		if(prop[i] == MATH_NUMBER_DIM_CAT){
			data_[i] = convert::nbo::to(data_[i]);
		}
		data.push_back(
			data_[i]);
	}
}

void math_number_set_simple_t::set_height(uint16_t height_, std::vector<uint8_t> prop_){
	data.clear();
	height = height_;
	prop = prop_;
}

void math_number_set_simple_t::list_virtual_data(data_id_t *id_){
	id = id_;
	id->add_data_one_byte_vector_vector(&data, ~0, ~0, public_ruleset);
	id->add_data_one_byte_vector(&prop, ~0, public_ruleset);
	id->add_data_raw(&height, sizeof(height));
}
