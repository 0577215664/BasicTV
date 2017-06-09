#include "id.h"
#include "id_api.h"
#include "../net/proto/net_proto.h"

static std::array<std::string, 2> encrypt_blacklist = {
	"encrypt_pub_key_t",
	"encrypt_priv_key_t"
};

static bool encrypt_blacklist_type(std::string type_){
	return std::find(
		encrypt_blacklist.begin(),
		encrypt_blacklist.end(),
		type_) != encrypt_blacklist.end();
}


static void id_export_raw(std::vector<uint8_t> tmp, std::vector<uint8_t> *vector){
	if(tmp.size() == 0){
		print("attempted to export completely blank data set", P_WARN);
		return;
	}
	tmp = convert::nbo::to(tmp);
	vector->insert(vector->end(), tmp.begin(), tmp.end());
}


// first half is the datum (local), second half are passed parameters
static bool should_export(std::pair<uint8_t, uint8_t> network_flags,
			  std::pair<uint8_t, uint8_t> export_flags,
			  std::pair<uint8_t, uint8_t> peer_flags){
	return (network_flags.second <= network_flags.first || network_flags.second == ID_DATA_RULE_UNDEF) &&
		(export_flags.second <= export_flags.first || export_flags.second == ID_DATA_RULE_UNDEF) &&
		(peer_flags.second <= peer_flags.first || peer_flags.second == ID_DATA_RULE_UNDEF);
}

//#define ID_EXPORT(var, list) id_export_raw((uint8_t*)&var, sizeof(var), &list)
#define ID_EXPORT(var, list) id_export_raw(std::vector<uint8_t>((uint8_t*)&var, (uint8_t*)&var+sizeof(var)), &list)

std::vector<uint8_t> data_id_t::export_data(
	uint8_t flags_,
	uint8_t extra,
	uint8_t network_rules,
	uint8_t export_rules,
	uint8_t peer_rules){
	if(flags_ != 0){
		print("we have no current use for a generic flag", P_WARN);
	}
	std::vector<uint8_t> retval;
	if(encrypt_blacklist_type(
		   convert::type::from(get_id_type(id)))){
		print("forcing no encryption on basis of encryption blacklist", P_SPAM);
		extra &= ~ID_EXTRA_ENCRYPT;
	}
	if(get_id_hash(id) != get_id_hash(
		   net_proto::peer::get_self_as_peer())){
		print("can't export somebody else's modified data", P_ERR);
	}
	uint8_t current_extra = 0;
	ID_EXPORT(current_extra, retval); // current extra is nothing
	ID_EXPORT(id, retval);
	ID_EXPORT(modification_incrementor, retval);
	for(uint64_t i = 0;i < data_vector.size();i++){
		if(should_export(std::make_pair(data_vector[i].get_network_rules(), network_rules),
				 std::make_pair(data_vector[i].get_export_rules(), export_rules),
				 std::make_pair(data_vector[i].get_peer_rules(), peer_rules)) == false){
			print("skipping based on export rules", P_SPAM);
			continue;
		}
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
		}else if(data_vector[i].get_flags() & ID_DATA_ID_VECTOR){
			//print("reading in an ID vector", P_SPAM);
			std::vector<id_t_> *vector =
				(std::vector<id_t_>*)data_vector[i].get_ptr();
			if(vector->data() == nullptr){
				//print("vector is empty, skipping", P_SPAM);
				continue;
			}
			data_to_export =
				std::vector<uint8_t>(
					(uint8_t*)vector->data(),
					(uint8_t*)vector->data()+
					(sizeof(id_t_)*vector->size()));
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
			for(uint64_t c = 0;c < vector->size();c++){
				transport_size_t trans_size =
					(*vector)[c].size();
				data_to_export.insert(
					data_to_export.end(),
					&trans_size,
					&trans_size+1);
				if((*vector)[c].data() == nullptr){
					//print("vector is empty, skipping", P_SPAM);
					continue;
					// don't export anything
				}
				data_to_export.insert(
					data_to_export.end(),
					(uint8_t*)(*vector)[c].data(),
					(uint8_t*)(*vector)[c].data()+trans_size);
			}

		}else{
			data_to_export =
				std::vector<uint8_t>(
					(uint8_t*)data_vector[i].get_ptr(),
					(uint8_t*)data_vector[i].get_ptr()+
					data_vector[i].get_length());
		}
		if(data_to_export.size() == 0){
			continue;
		}
		transport_i_t trans_i = i; // size fixing
		transport_size_t trans_size = data_to_export.size();
		//P_V(trans_i, P_SPAM);
		//P_V(trans_size, P_SPAM);
		ID_EXPORT(trans_i, retval);
		ID_EXPORT(trans_size, retval);
		id_export_raw(data_to_export, &retval);
	}
	if(extra & ID_EXTRA_COMPRESS){
		retval = id_api::raw::compress(retval);
	}
	if(extra & ID_EXTRA_ENCRYPT){
		retval = id_api::raw::encrypt(retval);
	}
	return retval;
}
