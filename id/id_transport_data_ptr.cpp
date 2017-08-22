#include "id_transport.h"

std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> > export_standardize_data_ptr(
	data_id_ptr_t *data_id_ptr){
	ASSERT(data_id_ptr != nullptr, P_ERR);
	ASSERT(data_id_ptr->get_ptr() != nullptr, P_ERR);
	switch(data_id_ptr->get_flags()){
	case 0:
		return std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> >(
			{std::make_tuple(
					static_cast<uint8_t*>(data_id_ptr->get_ptr()),
					0, 
					static_cast<uint64_t>(data_id_ptr->get_length()))});
	case ID_DATA_ID:
		return std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> >(
			{std::make_tuple(
					static_cast<uint8_t*>(data_id_ptr->get_ptr()),
					0, 
					static_cast<uint64_t>(data_id_ptr->get_length()))});
	case ID_DATA_BYTE_VECTOR:
		if(true){
			std::vector<uint8_t> *byte_vector =
				reinterpret_cast<std::vector<uint8_t>* >(
					data_id_ptr->get_ptr());
			if(byte_vector->size() == 0){
				return {};
			}
			return std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> >(
				{std::make_tuple(
						byte_vector->data(),
						0, 
						byte_vector->size())});
		}
		break;
	case ID_DATA_EIGHT_BYTE_VECTOR:
		if(true){
			std::vector<uint64_t> *byte_vector =
				reinterpret_cast<std::vector<uint64_t> *>(
					data_id_ptr->get_ptr());
			if(byte_vector->size() == 0){
				return {};
			}
			return std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> >(
				{std::make_tuple(
						reinterpret_cast<uint8_t*>(byte_vector->data()),
						0, 
						byte_vector->size()*8)});
		}
		break;
	case ID_DATA_BYTE_VECTOR_VECTOR:
		if(true){
			std::vector<std::vector<uint8_t> > *byte_vector_vector =
				reinterpret_cast<std::vector<std::vector<uint8_t> > *>(
					data_id_ptr->get_ptr());
			std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> > retval;
			for(transport_i_t i = 0;i < byte_vector_vector->size();i++){
				if((*byte_vector_vector)[i].size() == 0){
					continue;
				}
				retval.push_back(
					std::make_tuple(
						reinterpret_cast<uint8_t*>(
							(*byte_vector_vector)[i].data()),
						i,
						(*byte_vector_vector)[i].size()));
			}
			return retval;
		}
	default:
		print("invalid flags for data export", P_ERR);
	}
	return std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> > ({
			std::make_tuple(nullptr, 0, 0)});
}

// import returns all pointers in a vector, so the transport_i_t can directly
// map to the vector position
std::vector<uint8_t*> import_standardize_data_ptr(
	data_id_ptr_t *data_id_ptr,
	std::vector<uint64_t> target_size){
	ASSERT(data_id_ptr != nullptr, P_ERR);
	ASSERT(data_id_ptr->get_ptr() != nullptr, P_ERR);
	switch(data_id_ptr->get_flags()){
	case 0:
		ASSERT(target_size.size() == 1, P_ERR);
		ASSERT(data_id_ptr->get_length() == target_size[0], P_ERR);
		return std::vector<uint8_t*>({
				reinterpret_cast<uint8_t*>(data_id_ptr->get_ptr())});
	case ID_DATA_ID:
		ASSERT(target_size.size() == 1, P_ERR);
		ASSERT(data_id_ptr->get_length() == target_size[0], P_ERR);
		return std::vector<uint8_t*>({
				reinterpret_cast<uint8_t*>(data_id_ptr->get_ptr())});
	case ID_DATA_BYTE_VECTOR:
		if(true){
			ASSERT(target_size.size() == 1, P_ERR);
			std::vector<uint8_t> *byte_vector =
				reinterpret_cast<std::vector<uint8_t>* >(
					data_id_ptr->get_ptr());
			byte_vector->clear();
			(*byte_vector) =
				std::vector<uint8_t>(
					target_size[0], 0);
			return std::vector<uint8_t*>({
					reinterpret_cast<uint8_t*>(byte_vector->data())});
		}
		break;
	case ID_DATA_EIGHT_BYTE_VECTOR:
		if(true){
			ASSERT(target_size.size() == 1, P_ERR);
			std::vector<uint64_t> *byte_vector =
				reinterpret_cast<std::vector<uint64_t> *>(
					data_id_ptr->get_ptr());
			ASSERT(target_size[0]%8 == 0, P_ERR);
			(*byte_vector) =
				std::vector<uint64_t>(
					target_size[0]/8, 0);
			return std::vector<uint8_t*>({
					reinterpret_cast<uint8_t*>(byte_vector->data())});
		}
		break;
	case ID_DATA_BYTE_VECTOR_VECTOR:
		ASSERT(target_size.size() == 2, P_ERR);
		if(true){
			std::vector<uint8_t*> retval;
			std::vector<std::vector<uint8_t> > *byte_vector_vector =
				reinterpret_cast<std::vector<std::vector<uint8_t> >*>(
					data_id_ptr->get_ptr());
			byte_vector_vector->clear();
			for(uint64_t i = 0;i < target_size.size();i++){
				byte_vector_vector->push_back(
					std::vector<uint8_t>(
						target_size[i],
						0));
				retval.push_back(
					reinterpret_cast<uint8_t*>(
						(*byte_vector_vector)[byte_vector_vector->size()-1].data()));
			}
			return retval;
		}
	default:
		print("invalid flags for data export", P_ERR);
	}
	return {};
}
