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
		&((*pusher)[pusher->size()-size]),
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
#define DYNAMIC_SIZE_LOGIC(upper_limit, byte_count)	\
	if(size <= upper_limit){			\
		retval.push_back(			\
			byte_count);			\
		uint64_t payload_size =			\
			static_cast<uint64_t>(		\
				size);			\
		convert::nbo::to(			\
			reinterpret_cast<uint8_t*>(&payload_size),	\
			byte_count);			\
		retval.insert(				\
			retval.end(),			\
			&payload_size,			\
			&payload_size+byte_count);	\
		return retval;				\
	}						\


#define DYNAMIC_SIZE_WRAPPER(x) DYNAMIC_SIZE_LOGIC((static_cast<uint64_t>(1) << x), x/8)
static std::vector<uint8_t> export_gen_dynamic_size(
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
	DYNAMIC_SIZE_WRAPPER(8);
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

static uint64_t import_gen_dynamic_size(
	std::vector<uint8_t> *puller){
	uint8_t size_size =
		puller->at(0);
	puller->erase(
		puller->begin());
	uint64_t retval = 0;
	ASSERT(puller->size() >= size_size, P_ERR);
	std::memcpy(
		reinterpret_cast<uint8_t*>(&retval),
		puller->data(),
		size_size);
	puller->erase(
		puller->begin(),
		puller->begin()+size_size);
	return retval;
}

std::vector<uint8_t> import_dynamic_size_payload(
	std::vector<uint8_t> *puller){
	uint64_t size =
		import_gen_dynamic_size(
			puller);
	ASSERT(puller->size() >= size, P_ERR);
	const std::vector<uint8_t> retval(
		puller->begin(),
		puller->begin()+size);
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
	return data_id_transport_rules_t(
		unstringify_rules_tier(
			import_8bit_size_payload(
				puller)),
		unstringify_rules_intermediary(
			import_8bit_size_payload(
				puller)));
}

void export_ptr_from_data_id_ptr(
	std::vector<uint8_t> *pusher,
	data_id_ptr_t *data_id_ptr,
	transport_i_t trans_i){
	const std::pair<uint8_t*, uint64_t> standardized =
		export_standardize_data_ptr(
			data_id_ptr);
	if(standardized.first != nullptr){
		EXPORT_STATIC(
			*pusher,
			trans_i);
		stringify_rules(
			pusher,
			data_id_ptr->transport_rules);
		export_dynamic_size_payload(
			pusher,
			std::vector<uint8_t>(
				standardized.first,
				standardized.first+standardized.second));
	} // fails when we try to export an empty vector
}

std::tuple<std::vector<uint8_t>, data_id_transport_rules_t, transport_i_t> import_ptr_to_data_id_ptr(
	std::vector<uint8_t> *puller){
	std::tuple<std::vector<uint8_t>, data_id_transport_rules_t, transport_i_t> retval =
		{{}, data_id_transport_rules_t({}, {}), 0};
	IMPORT_STATIC(
		*puller,
		std::get<2>(retval));
	std::get<1>(retval) =
		unstringify_rules(
			puller);
	std::get<0>(retval) =
		import_dynamic_size_payload(
			puller);
	return retval;
}


