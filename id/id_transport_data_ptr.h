#ifndef ID_TRANSPORT_DATA_T
#define ID_TRANSPORT_DATA_T

#include "id.h"

std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> > export_standardize_data_ptr(
	data_id_ptr_t *data_id_ptr);

std::vector<uint8_t*> import_standardize_data_ptr(
	data_id_ptr_t *data_id_ptr,
	std::vector<uint64_t> target_size);

#endif
