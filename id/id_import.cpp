#include "id.h"
#include "id_api.h"
#include "id_import.h"
#include "id_transport.h"
#include "id_transport_data_ptr.h"

#include "tier/memory/id_tier_memory_special.h" /// mem_del/add_id

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
	IMPORT_STATIC(
		data,
		modification_incrementor);
	try{
		while(data.size() > 0){
			import_ptr_to_data_id_ptr(
				&data,
				&data_vector);
		}
	}catch(...){}
	id_tier_mem_regen_state_cache();
	id_tier_mem_update_state_cache(
		PTR_DATA(id_tier::state_tier::only_state_of_tier(
				 0, 0),
			 id_tier_state_t));
}
