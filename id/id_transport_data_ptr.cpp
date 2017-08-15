#include "id_transport.h"

std::pair<uint8_t*, uint64_t> export_standardize_data_ptr(
	data_id_ptr_t *data_id_ptr){
	ASSERT(data_id_ptr != nullptr, P_ERR);
	ASSERT(data_id_ptr->get_ptr() != nullptr, P_ERR);
	switch(data_id_ptr->get_flags()){
	case 0:
		return std::make_pair(
			static_cast<uint8_t*>(data_id_ptr->get_ptr()),
			static_cast<uint64_t>(data_id_ptr->get_length()));
	case ID_DATA_ID:
		// ID is just a hint, not actually any different
		return std::make_pair(
			static_cast<uint8_t*>(data_id_ptr->get_ptr()),
			static_cast<uint64_t>(data_id_ptr->get_length()));
	case ID_DATA_BYTE_VECTOR:
		if(true){
			std::vector<uint8_t> *byte_vector =
				reinterpret_cast<std::vector<uint8_t>* >(
					data_id_ptr->get_ptr());
			return std::make_pair(
				byte_vector->data(),
				byte_vector->size());
		}
		break;
	case ID_DATA_EIGHT_BYTE_VECTOR:
		if(true){
			std::vector<uint64_t> *byte_vector =
				reinterpret_cast<std::vector<uint64_t> *>(
					data_id_ptr->get_ptr());
			return std::make_pair(
				reinterpret_cast<uint8_t*>(byte_vector->data()),
				byte_vector->size()*8);
		}
		break;
	case ID_DATA_BYTE_VECTOR_VECTOR:
		print("flag is obsolete, PLEASE use another system", P_CRIT);
	default:
		print("invalid flags for data export", P_ERR);
	}
	return std::make_pair(nullptr, 0);
}

uint8_t *import_standardize_data_ptr(
	data_id_ptr_t *data_id_ptr,
	uint64_t target_size){
	ASSERT(data_id_ptr != nullptr, P_ERR);
	ASSERT(data_id_ptr->get_ptr() != nullptr, P_ERR);
	switch(data_id_ptr->get_flags()){
	case 0:
		ASSERT(data_id_ptr->get_length() == target_size, P_ERR);
		return reinterpret_cast<uint8_t*>(data_id_ptr->get_ptr());
	case ID_DATA_ID:
		ASSERT(data_id_ptr->get_length() == target_size, P_ERR);
		return reinterpret_cast<uint8_t*>(data_id_ptr->get_ptr());
	case ID_DATA_BYTE_VECTOR:
		if(true){
			std::vector<uint8_t> *byte_vector =
				reinterpret_cast<std::vector<uint8_t>* >(
					data_id_ptr->get_ptr());
			byte_vector->clear();
			(*byte_vector) =
				std::vector<uint8_t>(
					target_size, 0);
			return reinterpret_cast<uint8_t*>(byte_vector->data());
		}
		break;
	case ID_DATA_EIGHT_BYTE_VECTOR:
		if(true){
			std::vector<uint64_t> *byte_vector =
				reinterpret_cast<std::vector<uint64_t> *>(
					data_id_ptr->get_ptr());
			ASSERT(target_size%8 == 0, P_ERR);
			(*byte_vector) =
				std::vector<uint64_t>(
					target_size/8, 0);
			return reinterpret_cast<uint8_t*>(byte_vector->data());
		}
		break;
	case ID_DATA_BYTE_VECTOR_VECTOR:
		print("flag is obsolete, PLEASE use another system", P_CRIT);
	default:
		print("invalid flags for data export", P_ERR);
	}
	return nullptr;
}
