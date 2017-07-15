#include "id.h"
#include "id_api.h"

#include "../tv/tv_dev_audio.h"
#include "../tv/tv_dev_video.h"
#include "../tv/tv_frame_audio.h"
#include "../tv/tv_frame_video.h"
#include "../tv/tv_frame_caption.h"
#include "../tv/tv_channel.h"
#include "../tv/tv_window.h"
#include "../tv/tv_menu.h"
#include "../tv/tv_item.h"
#include "../net/proto/inbound/net_proto_inbound_data.h"
#include "../net/proto/outbound/net_proto_outbound_data.h"
#include "../net/proto/net_proto_con_req.h"
#include "../net/proto/net_proto_peer.h"
#include "../net/proto/net_proto_con_req.h"
#include "../net/proto/net_proto.h"
#include "../net/proto/net_proto_socket.h"
#include "../net/net_cache.h"
#include "../net/net.h"
#include "../input/input.h"
#include "../input/input_ir.h"
#include "../settings.h"
#include "../system.h"
#include "../cryptocurrency.h"
#include "../compress/compress.h"
#include "../encrypt/encrypt.h"

/*
  TODO (SERIOUS):
  Even though there is no cryptographic proof of ownership, make sure that
  the hash of any requests come from a socket whose bound peer has the same
  hash, since there is no need (right now, for these simple types
  specifically) to forward them. There might be a case in the future where
  another type is allowed to be forwarded to find really obscure information,
  but that's pretty far off.

  (Also forwardable types are probably prone to DoS, unless we do something
  crafty with rules and rule enforcement with public key hashes)
 */

std::vector<type_t_> encrypt_blacklist = {
	TYPE_ENCRYPT_PUB_KEY_T,
	TYPE_ENCRYPT_PRIV_KEY_T,
	TYPE_NET_PROTO_CON_REQ_T,
	TYPE_NET_PROTO_TYPE_REQUEST_T,
	TYPE_NET_PROTO_LINKED_LIST_REQUEST_T,
	TYPE_NET_PROTO_ID_REQUEST_T
};

std::vector<id_t_> id_api::sort::fingerprint(std::vector<id_t_> tmp){
	// TODO: actually get the finerprints
	return tmp;
}

void id_api::linked_list::link_vector(std::vector<id_t_> vector){
	switch(vector.size()){
	case 0:
		print("vector is empty", P_NOTE);
		return;
	case 1:
		print("vector has one entry, can't link to anything" , P_NOTE);
		return;
	case 2:
		PTR_ID(vector[0], )->set_linked_list(
			std::make_pair(
				std::vector<id_t_>(),
				std::vector<id_t_>({vector[1]})));
		PTR_ID(vector[1], )->set_linked_list(
			std::make_pair(
				std::vector<id_t_>({vector[0]}),
				std::vector<id_t_>()));
		return;
	default:
		break;
	}
	data_id_t *first = PTR_ID(vector[0], );
	if(first != nullptr){
		first->set_linked_list(
			std::make_pair(
				std::vector<id_t_>({}),
				std::vector<id_t_>({vector[1]})));
	}else{
		print("first entry is a nullptr, this can be fixed, but I didn't bother and this shouldn't happen anyways", P_ERR);
	}
	for(uint64_t i = 1;i < vector.size()-1;i++){
		data_id_t *id = PTR_ID(vector[i], );
		if(id == nullptr){
			print("can't link against an ID that doesn't exist", P_ERR);
		}
		id->set_linked_list(
			std::make_pair(
				std::vector<id_t_>({vector[i-1]}),
				std::vector<id_t_>({vector[i+1]})));
	}
	PTR_ID(vector[vector.size()-1], )->set_linked_list(
		std::make_pair(
			std::vector<id_t_>({vector[vector.size()-2]}),
			std::vector<id_t_>({})));
}

/*
  These functions only operate in one direction per call, but can be
  made to run in both (call backwards in vector A, call forwards
  in vector B, insert A, then start_id, then B).
 */

#pragma message("id_api::linked_list::list::by_distance can be optimized pretty easily, just haven't done that yet")

std::vector<id_t_> id_api::linked_list::list::by_distance(id_t_ start_id,
							  int64_t pos){
	ASSERT(pos != 0, P_ERR);
	ASSERT(start_id != ID_BLANK_ID, P_ERR);
	std::vector<id_t_> retval;
	bool forwards = pos > 0;
	int64_t cur_pos = 0;
	data_id_t *id_ptr =
		PTR_ID(start_id, );
	try{
		while(id_ptr != nullptr &&
		      abs(pos) >= abs(cur_pos)){
			const id_t_ next_id =
				(forwards) ?
				id_ptr->get_linked_list().second.at(0) :
				id_ptr->get_linked_list().first.at(0);
			id_ptr =
				PTR_ID(next_id, );
			retval.push_back(
				next_id);
			if(forwards){
				cur_pos++;
			}else{
				cur_pos--;
			}
		}
	}catch(...){}
	return retval;
}

std::vector<id_t_> id_api::linked_list::list::by_distance_until_match(id_t_ start_id,
								      int64_t pos,
								      id_t_ target_id){
	ASSERT(pos != 0, P_ERR);
	ASSERT(start_id != ID_BLANK_ID, P_ERR);
	ASSERT(target_id != ID_BLANK_ID, P_ERR);
	std::vector<id_t_> retval;
	bool forwards = pos > 0;
	int64_t cur_pos = 0;
	data_id_t *id_ptr =
		PTR_ID(start_id, );
	try{
		while(id_ptr != nullptr &&
		      id_ptr->get_id() != target_id &&
		      abs(pos) >= abs(cur_pos)){
			const id_t_ next_id =
				(forwards) ?
				id_ptr->get_linked_list().second.at(0) :
				id_ptr->get_linked_list().first.at(0);
			id_ptr =
				PTR_ID(next_id, );
			retval.push_back(
				next_id);
			if(forwards){
				cur_pos++;
			}else{
				cur_pos--;
			}
		}
	}catch(...){}
	if(!forwards){
		std::reverse(
			retval.begin(),
			retval.end());
	}
	return retval;
}

/*
  TODO: something here is broken, check over these functions and make sure there
  aren't any widespread errors
 */

std::vector<uint8_t> id_api::raw::encrypt(std::vector<uint8_t> data){
	id_t_ id = fetch_id(data);
	P_V_S(convert::array::id::to_hex(id), P_VAR);
	if(get_id_type(id) == TYPE_ENCRYPT_PUB_KEY_T){
		print("can't encrypt public key", P_WARN);
	}else if(get_id_type(id) == TYPE_ENCRYPT_PRIV_KEY_T){
		print("can't encrypt private key", P_WARN);
	}else{
		try{
			id_t_ priv_key_id =
				encrypt_api::search::priv_key_from_hash(
					get_id_hash(id));
			P_V_S(convert::array::id::to_hex(priv_key_id), P_VAR);
			std::vector<uint8_t> unencrypt_chunk =
				std::vector<uint8_t>(
					data.begin()+ID_PREAMBLE_SIZE,
					data.end());
			P_V(unencrypt_chunk.size(), P_VAR);
			if(unencrypt_chunk.size() == 0){
				// having no actual payload still works
				print("unencrypt_chunk is empty for " + id_breakdown(id) + "(function call for supplied data probably had exporting restrictions of some sort)", P_SPAM);
				return data;
			}
			std::vector<uint8_t> encrypt_chunk =
				encrypt_api::encrypt(
					unencrypt_chunk,
					priv_key_id);
			P_V(encrypt_chunk.size(), P_VAR);
			if(encrypt_chunk.size() != 0){
				data.erase(
					data.begin()+ID_PREAMBLE_SIZE,
					data.end());
				data.insert(
					data.end(),
					encrypt_chunk.begin(),
					encrypt_chunk.end());
				data[0] |= ID_EXTRA_ENCRYPT;
			}else{
				print("can't encrypt empty chunk", P_WARN);
				// should be checked before call, but it's no
				// biggie right now
			}
		}catch(...){
			P_V_S(convert::type::from(get_id_type(id)), P_VAR);
			print("can't encrypt exported id information, either not owner or a bug", P_ERR);
		}
	}
	return data;
}

std::vector<uint8_t> id_api::raw::decrypt(std::vector<uint8_t> data){
	id_t_ id = fetch_id(data);
	if(!(data[0] & ID_EXTRA_ENCRYPT)){
		print("can't decrypt pre-decrypted data", P_WARN);
	}else{
		data[0] &= ~ID_EXTRA_ENCRYPT;
		id_t_ pub_key_id =
			encrypt_api::search::pub_key_from_hash(
				get_id_hash(id));
		if(pub_key_id == ID_BLANK_ID){
			print("couldn't find public key for ID decryption" + id_breakdown(id), P_ERR);
		}else{
			std::vector<uint8_t> decrypt_chunk =
				encrypt_api::decrypt(
					std::vector<uint8_t>(
						data.begin()+ID_PREAMBLE_SIZE,
						data.end()),
					pub_key_id);
			data.erase(
				data.begin()+ID_PREAMBLE_SIZE,
				data.end());
			data.insert(
				data.end(),
				decrypt_chunk.begin(),
				decrypt_chunk.end());
		}
	}
	return data;
}

std::vector<uint8_t> id_api::raw::compress(std::vector<uint8_t> data){
	if(data[0] & ID_EXTRA_COMPRESS){
		print("can't compress pre-compressed data", P_WARN);
		print("TODO: compress pre-compressed data", P_NOTE);
	}else{
		data[0] |= ID_EXTRA_COMPRESS;
		std::vector<uint8_t> compress_chunk =
			compressor::compress(
				std::vector<uint8_t>(
					data.begin()+ID_PREAMBLE_SIZE,
					data.end()),
				9,
				0); // type isn't used currently
		data.erase(
			data.begin()+ID_PREAMBLE_SIZE,
			data.end());
		data.insert(
			data.end(),
			compress_chunk.begin(),
			compress_chunk.end());
	}
	return data;
}

std::vector<uint8_t> id_api::raw::decompress(std::vector<uint8_t> data){
	if(!(data[0] & ID_EXTRA_COMPRESS)){
		print("can't decompress uncompressed data", P_WARN);
	}else{
		data[0] &= ~ID_EXTRA_COMPRESS;
		std::vector<uint8_t> raw_chunk =
			std::vector<uint8_t>(
				data.begin()+ID_PREAMBLE_SIZE,
				data.end());
		std::vector<uint8_t> compress_chunk =
			compressor::decompress(
				raw_chunk);
		data.erase(
			data.begin()+ID_PREAMBLE_SIZE,
			data.end());
		data.insert(
			data.end(),
			compress_chunk.begin(),
			compress_chunk.end());
	}
	return data;
}

/*
  Following variables are guaranteed to be not encrypted
 */

static void fetch_size_sanity_check(uint64_t needed_size, uint64_t vector_size){
	if(needed_size > vector_size){
		P_V(needed_size, P_WARN);
		P_V(vector_size, P_WARN);
		print("vector is not large enough to contain requested information", P_ERR);
	}
}

static void generic_fetch(uint8_t *ptr, uint64_t start, uint64_t size, uint8_t *byte_vector){
	if(unlikely(byte_vector == nullptr)){
		// should have been seen in size sanity check
		print("byte_vector is a nullptr", P_ERR);
	}
	memcpy(ptr, byte_vector+start, size);
	convert::nbo::from(ptr, size); // checks for one byte length
}

#pragma warning("only memory is used with the new tier system at the moment")

void id_api::add_data(std::vector<uint8_t> data){
	id_tier::operation::add_data_to_state(
		{id_tier::state_tier::only_state_of_tier(0, 0)},
		{data});
}

id_t_ id_api::raw::fetch_id(std::vector<uint8_t> data){
	id_t_ retval = ID_BLANK_ID;
	const uint64_t start = sizeof(extra_t_);
	const uint64_t size = sizeof(retval);
	fetch_size_sanity_check(start+size, data.size());
	generic_fetch(&(retval[0]), start, size, data.data());
	return retval;
}

extra_t_ id_api::raw::fetch_extra(std::vector<uint8_t> data){
	extra_t_ retval = 0;
	const uint64_t start = 0;
	const uint64_t size = sizeof(retval);
	fetch_size_sanity_check(start+size, data.size());
	generic_fetch(&retval, start, size, data.data());
	return retval;
}

type_t_ id_api::raw::fetch_type(std::vector<uint8_t> data){
	return get_id_type(fetch_id(data));
}

mod_inc_t_ id_api::raw::fetch_mod_inc(std::vector<uint8_t> data){
	mod_inc_t_ retval = 0;
	const uint64_t start = sizeof(extra_t_)+sizeof(id_t_);
	const uint64_t size = sizeof(retval);
	fetch_size_sanity_check(start+size, data.size());
	generic_fetch((uint8_t*)&retval, start, size, data.data());
	P_V_B(retval, P_VAR);
	return retval;
}

int64_t id_api::linked_list::pos_in_linked_list(id_t_ ref_id, id_t_ goal_id, uint64_t max_search_radius){
	std::pair<id_t_, int64_t> lower_bound =
		std::make_pair(
			ref_id, 0);
	std::pair<id_t_, int64_t> upper_bound =
		std::make_pair(
			ref_id, 0);
	while((uint64_t)abs(lower_bound.second) < max_search_radius && lower_bound.first != ID_BLANK_ID){
		data_id_t *tmp_id_ptr =
			PTR_ID(lower_bound.first, );
		if(tmp_id_ptr == nullptr){
			break;
		}
		std::pair<std::vector<id_t_>, std::vector<id_t_> > linked_list =
			tmp_id_ptr->get_linked_list();
		if(linked_list.first.size() == 0){
			lower_bound.first = ID_BLANK_ID;
		}else{
			lower_bound.first =
				linked_list.first[0];
			lower_bound.second--;
		}
	}
	if(lower_bound.first == goal_id){
		return lower_bound.second;
	}
	while((uint64_t)abs(upper_bound.second) < max_search_radius && upper_bound.first != ID_BLANK_ID){
		data_id_t *tmp_id_ptr =
			PTR_ID(upper_bound.first, );
		if(tmp_id_ptr == nullptr){
			break;
		}
		std::pair<std::vector<id_t_>, std::vector<id_t_> > linked_list =
			tmp_id_ptr->get_linked_list();
		if(linked_list.first.size() == 0){
			upper_bound.first = ID_BLANK_ID;
		}else{
			upper_bound.first =
				linked_list.first[0];
			upper_bound.second++;
		}
	}
	if(upper_bound.first == goal_id){
		return upper_bound.second;
	}
	print("unable to find ID in linked list", P_ERR);
	return 0;
}

bool encrypt_blacklist_type(type_t_ type_){
	return std::find(
		encrypt_blacklist.begin(),
		encrypt_blacklist.end(),
		type_) != encrypt_blacklist.end();
}

#define ID_SHIFT(x) vector_pos += sizeof(x)
#define ID_IMPORT(x) memcpy(&x, data.data()+vector_pos, sizeof(x));vector_pos += sizeof(x)

#pragma warning("strip_to_lowest_rules removed a lot of sanity checks for clarity, should REALLY re-add");

std::vector<uint8_t> id_api::raw::strip_to_lowest_rules(
	std::vector<uint8_t> data,
	uint8_t network_rules,
	uint8_t export_rules,
	uint8_t peer_rules){

	uint32_t vector_pos = 0;
	id_t_ trans_id = ID_BLANK_ID;
	uint8_t extra =
		data[0];
	ASSERT((0b11111100 & extra) == 0, P_ERR);
	if(extra & ID_EXTRA_ENCRYPT){
		print("try and optimize code so strip_to_rules doesn't have to handle encryption", P_WARN);
		data = id_api::raw::decrypt(data);
	}
	if(extra & ID_EXTRA_COMPRESS){
		print("try and optimize code so strip_to_rules doesn't have to handle encryption", P_WARN);
		data = id_api::raw::decompress(data);
	}
	mod_inc_t_ modification_incrementor = 0;
	ID_SHIFT(extra); // just to remove it
	ID_SHIFT(trans_id);
	ID_SHIFT(modification_incrementor);

	transport_i_t trans_i = 0;
	transport_size_t trans_size = 0;
	uint8_t network_rules_tmp = 0;
	uint8_t export_rules_tmp = 0;
	uint8_t peer_rules_tmp = 0;
	while(data.size() > sizeof(transport_i_t) + sizeof(transport_size_t)){
		ID_SHIFT(trans_i);
		ID_IMPORT(network_rules_tmp);
		ID_IMPORT(export_rules_tmp);
		ID_IMPORT(peer_rules_tmp);
		ID_IMPORT(trans_size);
		const bool network_allows = (network_rules_tmp >= network_rules || network_rules == ID_DATA_RULE_UNDEF);
		const bool export_allows = (export_rules_tmp >= export_rules || export_rules == ID_DATA_RULE_UNDEF);
		const bool peer_allows = (peer_rules_tmp >= peer_rules || peer_rules == ID_DATA_RULE_UNDEF);
		if(!(network_allows &&
		     export_allows &&
		     peer_allows)){
			data.erase(
				data.begin()+vector_pos,
				data.begin()+vector_pos+trans_size);
			vector_pos -= trans_size;
		}
	}
	return data;
}

void id_api::print_id_vector(std::vector<id_t_> id_vector, uint32_t p_l){
	for(uint64_t i = 0;i < id_vector.size();i++){
		print(id_breakdown(id_vector[i]), p_l);
	}
}

void id_api::assert_valid_id(id_t_ id){
	type_t_ type =
		get_id_type(id);
	if(type > 0 && type < TYPE_COUNT){ // update this to match the type count
		ASSERT(get_id_uuid(id) != 0 ||
		       get_id_hash(id) != blank_hash, P_ERR);
	}else if(type == 0){
		ASSERT(get_id_uuid(id) == 0 ||
		       get_id_hash(id) == blank_hash, P_ERR);
	}else{
		ASSERT(type >= TYPE_COUNT, P_ERR);
	}
}

void id_api::assert_valid_id(std::vector<id_t_> id){
	for(uint64_t i = 0;i < id.size();i++){
		const hash_t_ hash =
			get_id_hash(id[i]);
		P_V(get_id_uuid(id[i]), P_VAR);
		P_V_S(convert::number::to_hex(
			    std::vector<uint8_t>(
				    hash.begin(),
				    hash.end())), P_VAR);
		P_V(get_id_type(id[i]), P_VAR);
		assert_valid_id(id[i]);
	}
}
