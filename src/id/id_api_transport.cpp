#include "tier/id_tier.h"
#include "id_transport.h"
#include "id_api.h"

// Any IDs that directly reference each other can actually be shipped with each
// other as long as the addition of a new data type doesn't increase the actual
// payload (doesn't push the RSA key length)

// This encapsulation format is just an escaped list with dynamic sizing

std::vector<std::pair<std::vector<uint8_t>, std::vector<id_t_> > > id_api::transport::smart_id_export(
	std::vector<id_t_> id_list){
	std::vector<std::vector<id_t_> > reference_list;
	for(uint64_t i = 0;i < id_list.size();i++){
		data_id_t *ptr_id =
			PTR_ID(id_list[i], );
		CONTINUE_IF_NULL(ptr_id, P_WARN);
		const std::vector<id_t_> tmp_payload =
			ptr_id->all_ids();
		bool wrote = true;
		for(uint64_t c = 0;c < reference_list.size();c++){
			if(std::find_first_of(
				   tmp_payload.begin(),
				   tmp_payload.end(),
				   reference_list[c].begin(),
				   reference_list[c].end()) != tmp_payload.end()){
				reference_list[c].push_back(
					id_list[i]);
				wrote = true;
			}
		}
		if(wrote == false){
			reference_list.push_back(
				std::vector<id_t_>({id_list[i]}));
			wrote = true;
		}
	}
	std::vector<std::pair<std::vector<uint8_t>, std::vector<id_t_> > > retval;
	for(uint64_t i = 0;i < reference_list.size();i++){
		std::vector<std::vector<uint8_t> > tmp_payload =
			id_tier::operation::get_data_from_state(
				id_tier::state_tier::optimal_state_vector_of_tier_vector(
					all_tiers),
				reference_list[i]);
		std::vector<uint8_t> escaped;
		for(uint64_t c = 0;c < tmp_payload.size();c++){
			export_dynamic_size_payload(
				&escaped,
				tmp_payload[c]);
		}
	}
	return retval;
}

std::vector<id_t_> id_api::transport::smart_id_import(
	std::vector<uint8_t> data){
	std::vector<id_t_> retval;
	// we don't do any sanity checking right now, let's just assume that
	// GC is good enough to destroy whatever we don't need, and that the
	// extra crap isn't too computationally expensive
	uint64_t data_size = 0;
	while(data_size != data.size()){
		try{
			id_tier::operation::smart::add_data(
				std::vector<std::vector<uint8_t> >({
						import_dynamic_size_payload(
							&data)}));
			
		}catch(...){
			print("cannot add iD to tier via smart call", P_WARN);
		}
	}
	return retval;
}
