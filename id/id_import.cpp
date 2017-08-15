#include "id.h"
#include "id_api.h"
#include "id_import.h"
#include "id_transport.h"
#include "id_transport_data_ptr.h"

#define ITER_TO_DATA_PTR(data_ptr_, trans_i_)			\
	ASSERT(data_vector.size() > trans_i_, P_ERR);			\
	ASSERT(data_vector[trans_i_].get_ptr() != nullptr, P_ERR);	\
	data_ptr_ = &(data_vector[trans_i_]);

#define LOOKUP_WRITE_DATA_PTR(tmp_data)					\
	ASSERT(data_vector.size() > std::get<2>(tmp_data), P_ERR);	\
	std::memcpy(							\
		import_standardize_data_ptr(				\
			&data_vector[std::get<2>(tmp_data)],		\
			std::get<0>(tmp_data).size()),			\
		std::get<0>(tmp_data).data(),				\
		std::get<0>(tmp_data).size());				\
	

void data_id_t::import_data(
	std::vector<uint8_t> data){
	data = id_api::raw::force_to_extra(
		data,
		0);
	data.erase(data.begin()); // extra is zero
	IMPORT_STATIC(
		data,
		id);
	try{
		while(data.size() > 0){
			const std::tuple<std::vector<uint8_t>, data_id_transport_rules_t, transport_i_t> tmp_data =
				import_ptr_to_data_id_ptr(
					&data);
			LOOKUP_WRITE_DATA_PTR(tmp_data);
		}
	}catch(...){}
}
