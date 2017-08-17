#include "id.h"
#include "id_api.h"
#include "id_import.h"
#include "id_transport.h"
#include "id_transport_data_ptr.h"

void data_id_t::import_data(
	std::vector<uint8_t> data){
	data = id_api::raw::force_to_extra(
		data,
		0);
	ASSERT(data.size() > 0, P_UNABLE);
	ASSERT(data[0] == 0, P_ERR);
	data.erase(data.begin()); // extra is zero
	IMPORT_STATIC(
		data,
		id);
	try{
		while(data.size() > 0){
			const std::tuple<std::vector<uint8_t>, data_id_transport_rules_t, transport_i_t> tmp_data =
				import_ptr_to_data_id_ptr(
					&data);
			ASSERT(data_vector.size() > std::get<2>(tmp_data), P_ERR);
			std::memcpy(							
				import_standardize_data_ptr(				
					&data_vector[std::get<2>(tmp_data)],		
					std::get<0>(tmp_data).size()),			
				std::get<0>(tmp_data).data(),				
				std::get<0>(tmp_data).size());				
		}
	}catch(...){}
}
