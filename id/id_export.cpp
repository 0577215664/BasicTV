#include "id.h"
#include "id_api.h"
#include "../net/proto/net_proto.h"

static void id_export_raw(std::vector<uint8_t> tmp, std::vector<uint8_t> *vector){
	if(tmp.size() == 0){
		print("attempted to export completely blank data set", P_WARN);
		return;
	}
	tmp = convert::nbo::to(tmp);
	vector->insert(vector->end(), tmp.begin(), tmp.end());
}

//#define ID_EXPORT(var, list) id_export_raw((uint8_t*)&var, sizeof(var), &list)
/*
  ID_EXPORT exports the size of the payload, and the payload itself in NBO
  
  The only variable not used for this is the beginning extra byte
 */
#define ID_EXPORT(var, list) id_export_raw(std::vector<uint8_t>((uint8_t*)&var, (uint8_t*)&var+sizeof(var)), &list)

/*
  For now, anything that needs restrictions is either public or destroyed on
  a delete (i.e. not listed with ID subsystem)
 */

std::vector<uint8_t> data_id_t::export_data(
	uint8_t extra){
	std::vector<uint8_t> retval;
	if(encrypt_blacklist_type(
		   get_id_type(id))){
		extra &= ~ID_EXTRA_ENCRYPT;
	}
	if(get_id_hash(id) != get_id_hash(
		   production_priv_key_id)){
		std::raise(SIGINT);
		print("can't export somebody else's modified data", P_ERR);
	}
	retval.push_back(0); // current_extra
	// ID_EXPORT(current_extra, retval);
	ID_EXPORT(id, retval);
	ID_EXPORT(modification_incrementor, retval);
	for(uint64_t i = 0;i < data_vector.size();i++){
		std::vector<uint8_t> data_to_export;
		if(data_vector[i].get_flags() & ID_DATA_BYTE_VECTOR){
			//print("reading in a byte vector", P_SPAM);
			std::vector<uint8_t> *vector =
				(std::vector<uint8_t>*)data_vector[i].get_ptr();
			if(vector->data() == nullptr){
				//print("vector is empty, skipping", P_SPAM);
				continue;
			}
			data_to_export =
				std::vector<uint8_t>(
					(uint8_t*)vector->data(),
					(uint8_t*)vector->data()+
					(sizeof(uint8_t)*vector->size()));
		}else if(data_vector[i].get_flags() & ID_DATA_EIGHT_BYTE_VECTOR){
			//print("reading in a 64-bit vector", P_SPAM);
			std::vector<uint64_t> *vector =
				(std::vector<uint64_t>*)data_vector[i].get_ptr();
			if(vector->data() == nullptr){
				//print("vector is empty, skipping", P_SPAM);
				continue;
			}
			data_to_export =
				std::vector<uint8_t>(
					(uint8_t*)vector->data(),
					(uint8_t*)vector->data()+
					(sizeof(uint8_t)*vector->size()));
		}else if(data_vector[i].get_flags() & ID_DATA_BYTE_VECTOR_VECTOR){
			// nested vectors work a bit differently
			// this code makes two (safe) assumptions
			// 1. This data will only be read from a data_vector[i] on
			// the other side with a BYTE_VECTOR_VECTOR flag
			// 2. fail-safe
			std::vector<std::vector<uint8_t> > *vector =
				(std::vector<std::vector<uint8_t> >*)data_vector[i].get_ptr();
			if(vector->data() == nullptr){
				//print("vector is empty, skipping", P_SPAM);
				continue;
			}
			transport_size_t vector_size =
				vector->size();
			data_to_export.insert(
				data_to_export.end(),
				reinterpret_cast<uint8_t*>(&vector_size),
				reinterpret_cast<uint8_t*>(&vector_size)+sizeof(transport_size_t));
			P_V(vector->size(), P_VAR);
			for(uint64_t c = 0;c < vector_size;c++){
				transport_size_t trans_size_tmp =
					(*vector)[c].size();
				data_to_export.insert(
					data_to_export.end(),
					reinterpret_cast<uint8_t*>(&trans_size_tmp),
					reinterpret_cast<uint8_t*>(&trans_size_tmp)+sizeof(transport_size_t));
				if((*vector)[c].data() == nullptr){
					//print("vector is empty, skipping", P_SPAM);
					continue;
					// don't export anything
				}
				data_to_export.insert(
					data_to_export.end(),
					(uint8_t*)(*vector)[c].data(),
					(uint8_t*)(*vector)[c].data()+trans_size_tmp);
			}
		}else{
			uint64_t length = data_vector[i].get_length();
			if(data_vector[i].get_flags() & ID_DATA_ID){
				length *= sizeof(id_t_);
			}
			// P_V(data_vector[i].get_length(), P_SPAM);
			data_to_export =
				std::vector<uint8_t>(
					(uint8_t*)data_vector[i].get_ptr(),
					(uint8_t*)data_vector[i].get_ptr()+
					length);
		}
		if(data_to_export.size() == 0){
			continue;
		}
		transport_i_t trans_i = i; // size fixing
		transport_size_t trans_size = data_to_export.size();
		ID_EXPORT(trans_i, retval);
		ID_EXPORT(trans_size, retval);
		id_export_raw(data_to_export, &retval);
	}
	ASSERT((0b11111100 & extra) == 0, P_ERR);
	// P_V(extra, P_SPAM);
	if(extra & ID_EXTRA_COMPRESS){
		retval = id_api::raw::compress(retval);
	}
	if(extra & ID_EXTRA_ENCRYPT){
		retval = id_api::raw::encrypt(retval);
	}
	return retval;
}
