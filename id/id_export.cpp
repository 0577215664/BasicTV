#include "id.h"
#include "id_api.h"
#include "../net/proto/net_proto.h"
#include "id_transport.h"

/*
  EXPORT FORMAT:
  Prefix:
  [EXTRA][ID]

  Each data type has the following: 
  [8-BIT ESCAPED PERMISSIONS][DYNAMIC SIZED PAYLOAD]
  
  Format for permissions is defined in id_transport

 */

static void sanity_check(
	data_id_t *id_ptr,
	extra_t_ extra){
	ASSERT(get_id_hash(id_ptr->get_id()) == get_id_hash(production_priv_key_id), P_ERR);
	ASSERT((extra & 0b11111100) == 0, P_ERR);
}

std::vector<uint8_t> data_id_t::export_data(
	uint8_t extra){
	std::vector<uint8_t> retval;
	sanity_check(
		this,
		extra);
	EXPORT_STATIC(
		retval,
		extra);
	EXPORT_STATIC(
		retval,
		id);
	for(uint64_t i = 0;i < data_vector.size();i++){
		try{
			export_ptr_from_data_id_ptr(
				&retval,
				&data_vector[i],
				static_cast<transport_i_t>(i));
		}catch(...){
			print("unable to export datum", P_ERR);
		}
	}
	return retval;
}
