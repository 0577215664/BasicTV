#include "id_transport.h"
#include "id_transport_data_ptr.h"
#include "id.h"
#include "../util.h"

void export_static_size_payload(
	std::vector<uint8_t> *pusher,
	uint8_t* data,
	uint64_t size){
	pusher->insert(
		pusher->end(),
		data,
		data+size);
	convert::nbo::to(
		pusher->data()+pusher->size()-size,
		size);
}

void import_static_size_payload(
	std::vector<uint8_t> *puller,
	uint8_t* data,
	uint64_t size){
	ASSERT(puller->size() >= size, P_ERR);
	std::memcpy(
		data,
		puller->data(),
		size);
	convert::nbo::from(
		data,
		size);
	puller->erase(
		puller->begin(),
		puller->begin()+size);
}

void export_8bit_size_payload(
	std::vector<uint8_t> *pusher,
	std::vector<uint8_t> payload){
	ASSERT(payload.size() <= 255, P_ERR);
	pusher->push_back(
		static_cast<uint8_t>(
			payload.size()));
	convert::nbo::to(
		payload.data(),
		payload.size());
	pusher->insert(
		pusher->end(),
		payload.begin(),
		payload.end());
}

std::vector<uint8_t> import_8bit_size_payload(
	std::vector<uint8_t> *puller){
	const uint8_t size =
		puller->at(0);
	puller->erase(puller->begin());
	const std::vector<uint8_t> retval =
		convert::nbo::from(
			std::vector<uint8_t>(
				puller->begin(),
				puller->begin()+size));
	puller->erase(
		puller->begin(),
		puller->begin()+size);
	return retval;
}

// NBO
#define DYNAMIC_SIZE_LOGIC(upper_limit, byte_count)			\
	if(size <= upper_limit){					\
		uint64_t payload_size =					\
			static_cast<uint64_t>(				\
				size);					\
		retval.push_back(					\
			byte_count);					\
		convert::nbo::to(					\
			reinterpret_cast<uint8_t*>(&payload_size),	\
			byte_count);					\
		retval.insert(						\
			retval.end(),					\
			reinterpret_cast<uint8_t*>(&payload_size),	\
			reinterpret_cast<uint8_t*>(&payload_size)+byte_count); \
		return retval;						\
	}								\
	

#define DYNAMIC_SIZE_WRAPPER(x) DYNAMIC_SIZE_LOGIC((static_cast<uint64_t>(1) << x*8)-1, x)

std::vector<uint8_t> export_gen_dynamic_size(
	uint64_t size){
	std::vector<uint8_t> retval;
	DYNAMIC_SIZE_WRAPPER(1);
	DYNAMIC_SIZE_WRAPPER(2);
	DYNAMIC_SIZE_WRAPPER(3);
	DYNAMIC_SIZE_WRAPPER(4); // <-- This should be the upper limit IMHO
	print("going beyond 4GB for one entry, this is pretty wacky", P_WARN);
	DYNAMIC_SIZE_WRAPPER(5);
	DYNAMIC_SIZE_WRAPPER(6);
	DYNAMIC_SIZE_WRAPPER(7);
	// DYNAMIC_SIZE_WRAPPER(8);
	print("...", P_ERR);
	return retval;
}

void export_dynamic_size_payload(
	std::vector<uint8_t> *pusher,
	std::vector<uint8_t> payload){
	const std::vector<uint8_t> size_data =
		export_gen_dynamic_size(
			payload.size());
	pusher->insert(
		pusher->end(),
		size_data.begin(),
		size_data.end());
	convert::nbo::to(
		payload.data(),
		payload.size());
	pusher->insert(
		pusher->end(),
		payload.begin(),
		payload.end());
}

uint64_t import_gen_dynamic_size(
	std::vector<uint8_t> *puller){
	const uint8_t size_size = // 
		puller->at(0);
	puller->erase(
		puller->begin());
	uint64_t retval = 0;
	ASSERT(puller->size() >= size_size, P_ERR);
	ASSERT(size_size <= 8, P_ERR);
	std::memcpy(
		reinterpret_cast<uint8_t*>(&retval),
		puller->data(),
		size_size);
	puller->erase(
		puller->begin(),
		puller->begin()+size_size);
	convert::nbo::from(
		reinterpret_cast<uint8_t*>(&retval),
		size_size);
	P_V(size_size, P_DEBUG);
	P_V(retval, P_DEBUG);
	return retval;
}

std::vector<uint8_t> import_dynamic_size_payload(
	std::vector<uint8_t> *puller){
	uint64_t size =
		import_gen_dynamic_size(
			puller);
	ASSERT(puller->size() >= size, P_ERR);
	std::vector<uint8_t> retval(
		puller->begin(),
		puller->begin()+size);
	convert::nbo::to(
		retval.data(),
		retval.size());
	puller->erase(
		puller->begin(),
		puller->begin()+size);
	return retval;
}

static std::vector<uint8_t> stringify_rules_tier(
	std::vector<std::pair<uint8_t, uint8_t> > tier_vector){
	std::vector<uint8_t> retval;
	for(uint64_t i = 0;i < tier_vector.size();i++){
		retval.push_back(
			tier_vector[i].first);
		retval.push_back(
			tier_vector[i].second);
	}
	return retval;
}

static std::vector<uint8_t> stringify_rules_intermediary(
	std::vector<uint8_t> intermediary){
	return intermediary; // simple enough
}

void stringify_rules(
	std::vector<uint8_t> *pusher, 
	data_id_transport_rules_t rules){
	export_8bit_size_payload(
		pusher,
		stringify_rules_tier(
			rules.tier));
	export_8bit_size_payload(
		pusher,
		stringify_rules_intermediary(
			rules.intermediary));
}

static std::vector<std::pair<uint8_t, uint8_t> > unstringify_rules_tier(
	std::vector<uint8_t> str){
	std::vector<std::pair<uint8_t, uint8_t> > retval;
	ASSERT(str.size()%2 == 0, P_ERR);
	for(uint64_t i = 0;i < str.size();i+=2){
		retval.push_back(
			std::make_pair(
				str[i],
				str[i+1]));
	}
	return retval;
}

static std::vector<uint8_t> unstringify_rules_intermediary(
	std::vector<uint8_t> str){
	return str;
}

data_id_transport_rules_t unstringify_rules(
	std::vector<uint8_t> *puller){
	const std::vector<std::pair<uint8_t, uint8_t >> rules =
		unstringify_rules_tier(
			import_8bit_size_payload(
				puller));
	const std::vector<uint8_t> intermediary =
		unstringify_rules_intermediary(
			import_8bit_size_payload(
				puller));
	return data_id_transport_rules_t(
		rules,
		intermediary);
}

void export_ptr_from_data_id_ptr(
	std::vector<uint8_t> *pusher,
	data_id_ptr_t *data_id_ptr,
	transport_i_t trans_i){
	const std::vector<std::tuple<uint8_t*, transport_i_t, uint64_t> > standardized =
		export_standardize_data_ptr(
			data_id_ptr);
	if(standardized.size() == 0){
		return;
	}
	ASSERT(standardized.size() == 1, P_ERR);
	EXPORT_STATIC(
		*pusher,
		trans_i);
	stringify_rules(
		pusher,
		data_id_ptr->transport_rules);
	const std::vector<uint8_t> vector_length =
		export_gen_dynamic_size(
			standardized.size());
	pusher->insert(
		pusher->end(),
		vector_length.begin(),
		vector_length.end());
	for(uint64_t i = 0;i < standardized.size();i++){
		ASSERT(std::get<0>(standardized[i]) != nullptr &&
		       std::get<2>(standardized[i]) != 0, P_ERR);
		const std::vector<uint8_t> vector_iter =
			export_gen_dynamic_size(
				std::get<1>(standardized[i]));
		P_V(std::get<1>(standardized[i]), P_VAR);
		pusher->insert(
			pusher->end(),
			vector_iter.begin(),
			vector_iter.end());
		export_dynamic_size_payload(
			pusher,
			std::vector<uint8_t>(
				std::get<0>(standardized[i]),
				std::get<0>(standardized[i])+std::get<2>(standardized[i])));
	}
}

static void nether_add_at_pos(
	std::vector<std::vector<uint8_t> > *nether,
	std::vector<uint8_t> data,
	uint64_t pos){
	while(nether->size() < pos+1){
		nether->push_back(
			std::vector<uint8_t>({}));
	}
	ASSERT(nether->at(pos).size() == 0, P_ERR);
	nether->at(pos) = data;
}

// Entire vector needs to be passed for missing or skipped datum
void import_ptr_to_data_id_ptr(
	std::vector<uint8_t> *puller,
	std::vector<data_id_ptr_t> *data_id_ptr){
	transport_i_t trans_i = 0;
	IMPORT_STATIC(
		*puller,
		trans_i);
	data_id_transport_rules_t transport_rules =
		unstringify_rules(
			puller);
	uint64_t vector_length =
		import_gen_dynamic_size(
			puller);
	// nether is a working copy of the 2D vector we are inserting,
	// only types currently using more than 1D is a byte vector vector
	std::vector<std::vector<uint8_t> > nether;
	for(uint64_t i = 0;i < vector_length;i++){
		uint64_t tmp_i =
			import_gen_dynamic_size(
				puller);
		nether_add_at_pos(
			&nether,
			import_dynamic_size_payload(
				puller),
			tmp_i);
	}
	std::vector<uint64_t> standardized_nether_size;
	for(uint64_t i = 0;i < nether.size();i++){
		standardized_nether_size.push_back(
			nether[i].size());
	}
	std::vector<uint8_t*> standardized_input =
		import_standardize_data_ptr(
			&(data_id_ptr->at(trans_i)),
			standardized_nether_size);
	ASSERT(standardized_input.size() == standardized_nether_size.size(), P_ERR);
	for(uint64_t i = 0;i < standardized_input.size();i++){
		std::memcpy(
			standardized_input[i],
			nether[i].data(),
			nether[i].size());
	}
}
