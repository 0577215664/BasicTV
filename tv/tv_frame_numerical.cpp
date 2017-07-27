#include "tv_frame_numerical.h"
#include "tv_frame_standard.h"

tv_frame_numerical_t::tv_frame_numerical_t() : id(this, TYPE_TV_FRAME_NUMERICAL_T){
	list_virtual_data(&id);
	id.add_data_one_byte_vector(&escaped_one_dimension_data, ~0);
}

tv_frame_numerical_t::~tv_frame_numerical_t(){
}
