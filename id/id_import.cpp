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
			std::raise(SIGINT);
			import_ptr_to_data_id_ptr(
				&data,
				&data_vector);
		}
	}catch(...){}
}
