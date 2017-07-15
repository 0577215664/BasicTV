#include "id.h"
#include "id_api.h"
#include "tier/id_tier_memory.h"
#include "../main.h"
#include "../util.h"
#include "../lock.h"
#include "../convert.h"

// just for all of the data types
#include "../net/net.h"
#include "../net/net_socket.h"
#include "../net/proto/net_proto.h"
#include "../tv/tv.h"
#include "../tv/tv_frame_standard.h"
#include "../tv/tv_frame_video.h"
#include "../tv/tv_frame_audio.h"
#include "../tv/tv_channel.h"
#include "../tv/tv_window.h"
#include "../encrypt/encrypt.h"
#include "../compress/compress.h"

std::array<uint8_t, 32> get_id_hash(id_t_ id){
	std::array<uint8_t, 32> retval;
	memcpy(&(retval[0]), &(id[8]), 32);
	return retval;
}

void set_id_hash(id_t_ *id, std::array<uint8_t, 32> hash){
	memcpy(&((*id)[8]), &(hash[0]), 32);
}

uint64_t get_id_uuid(id_t_ id){
	return *((uint64_t*)&(id[0]));
}

void set_id_uuid(id_t_ *id, uint64_t uuid){
	memcpy(&((*id)[0]), &uuid, 8);
}

type_t_ get_id_type(id_t_ id){
	return id[40];
}

void set_id_type(id_t_ *id, type_t_ type){
	(*id)[40] = type;
}

void data_id_t::init_list_all_data(){
	add_data_id(
		&id, 1);
	add_data_one_byte_vector(
		&(linked_list.first),
		{ID_MAX_LINKED_LIST_SIZE});
	add_data_one_byte_vector(
		&(linked_list.second),
		{ID_MAX_LINKED_LIST_SIZE});
}

/*
  production_priv_key_id is the private key used in the encryption of all of the
  files. This should be changable, but no interface exists to do that yet (and I
  don't really see a need for one, assuming no broadcasted net_peer_ts share
  that key).

  If the key can't be found, then zero out the ID. Every time the ID is
  referenced, check to see if the hash is zero and generate the hash the
  first time production_priv_key_id is valid (throw an exception when
  get_id is called without a valid hash)
 */

/*
  A UUID of 0 is reserved for blank IDs and seperators for id_sets
 */

void data_id_t::init_gen_id(type_t_ type_){
	set_id_uuid(&id, true_rand(1, ~(uint64_t)0));
	set_id_type(&id, type_);
	encrypt_priv_key_t *priv_key =
		PTR_DATA(production_priv_key_id,
			 encrypt_priv_key_t);
	if(priv_key == nullptr){
		print("production_priv_key_id is a nullptr",
		      (running) ? P_WARN : P_NOTE);
		return;
	}
	encrypt_pub_key_t *pub_key =
		PTR_DATA(priv_key->get_pub_key_id(),
			 encrypt_pub_key_t);
	if(pub_key == nullptr){
		print("production_priv_key_id's public key is a nullptr",
		      (running) ? P_WARN : P_NOTE);
		return;
	}
	set_id_hash(&id,
		    encrypt_api::hash::sha256::gen_raw(
			    pub_key->get_encrypt_key().second));
}


data_id_t::data_id_t(void *ptr_, type_t_ type_){
	ptr = ptr_;	
	init_gen_id(type_);
	init_list_all_data();
	mem_add_id(this);
	print("created " + id_breakdown(id), P_DEBUG);
}

data_id_t::~data_id_t(){
	mem_del_id(this);
}

// is mem_del_id/mem_add_id needed anymore?

id_t_ data_id_t::get_id(bool skip){
	return id;
}

void data_id_t::set_id(id_t_ id_){
	set_id_uuid(&id, get_id_uuid(id_));
	set_id_hash(&id, get_id_hash(id_));
}

std::string data_id_t::get_type(){
	return convert::type::from(get_id_type(id));
}

/*
  Even though function can pass information to each other through IDs,
  get_id is used in too many searches to be a healthy benchmark
 */

void *data_id_t::get_ptr(){
	last_access_timestamp_micro_s =
		get_time_microseconds();
	return ptr;
}

void data_id_t::add_data(void *ptr_,
			 std::vector<uint32_t> size_,
			 uint8_t flags_,
			 uint8_t network_rules_,
			 uint8_t export_rules_,
			 uint8_t peer_rules_){
	if(ptr_ == nullptr){
		print("ptr_ is a nullptr", P_ERR);
	}
	data_vector.push_back(
		data_id_ptr_t(
			ptr_,
			size_,
			flags_,
			network_rules_,
			export_rules_,
			peer_rules_));
}

void data_id_t::rsa_decrypt_backlog(){
	for(uint64_t i = 0;i < rsa_backlog.size();i++){
		import_data(rsa_backlog[i]);
	}
	rsa_backlog.clear();
}

data_id_ptr_t::data_id_ptr_t(void *ptr_,
			     std::vector<uint32_t> length_,
			     uint8_t flags_,
			     uint8_t network_rules_,
			     uint8_t export_rules_,
			     uint8_t peer_rules_){
	ptr = ptr_;
	length = length_;
	flags = flags_;
	network_rules = network_rules_;
	export_rules = export_rules_;
	peer_rules = peer_rules_;
}

data_id_ptr_t::~data_id_ptr_t(){
}

void *data_id_ptr_t::get_ptr(){
	return ptr;
}

uint32_t data_id_ptr_t::get_length(){
	if(length.size() == 1){
		return length[0];
	}else{
		print("invalid size of length", P_ERR);
		return 0; // redunant
	}
}

std::vector<uint32_t> data_id_ptr_t::get_length_vector(){
	return length;
}

void data_id_t::set_lowest_global_flag_level(uint8_t network_rules,
					     uint8_t export_rules,
					     uint8_t peer_rules){
	for(uint64_t i = 0;i < data_vector.size();i++){
		if(data_vector[i].get_network_rules() > network_rules){
			data_vector[i].set_network_rules(
				network_rules);
		}
		if(data_vector[i].get_export_rules() > export_rules){
			data_vector[i].set_export_rules(
				export_rules);
		}
		if(data_vector[i].get_peer_rules() > peer_rules){
			data_vector[i].set_peer_rules(
				peer_rules);
		}
	}
}

/*
  Sanity check to see if it makes sense to export a piece of data
 */

void data_id_t::get_highest_global_flag_level(uint8_t *network_rules,
					      uint8_t *export_rules,
					      uint8_t *peer_rules){
	uint8_t network_rules_tmp = 0;
	uint8_t export_rules_tmp = 0;
	uint8_t peer_rules_tmp = 0;
	for(uint64_t i = 0;i < data_vector.size();i++){
		if(data_vector[i].get_network_rules() > network_rules_tmp){
			network_rules_tmp = data_vector[i].get_network_rules();
		}
		if(data_vector[i].get_export_rules() > export_rules_tmp){
			export_rules_tmp = data_vector[i].get_export_rules();
		}
		if(data_vector[i].get_peer_rules() > peer_rules_tmp){
			peer_rules_tmp = data_vector[i].get_peer_rules();
		}
	}
	if(network_rules != nullptr){
		*network_rules = network_rules_tmp;
	}
	if(export_rules != nullptr){
		*export_rules = export_rules_tmp;
	}
	if(peer_rules != nullptr){
		*peer_rules = peer_rules_tmp;
	}
}

std::string id_breakdown(id_t_ id_){
	std::string type = "BADTYPE";
	try{
		type =
			convert::type::from(
				get_id_type(id_));
	}catch(...){}
	return " (" + convert::array::id::to_hex(id_) +
		" of type " + type + ") ";
}

std::pair<std::vector<id_t_>, std::vector<id_t_> > data_id_t::get_linked_list(){
	return std::make_pair(
		expand_id_set(linked_list.first),
		expand_id_set(linked_list.second));
}

void data_id_t::set_linked_list(std::pair<std::vector<id_t_>, std::vector<id_t_> > tmp){
	linked_list.first =
		compact_id_set(tmp.first);
	linked_list.second =
		compact_id_set(tmp.second);
}
