#include "id.h"
#include "id_api.h"
#include "tier/id_tier.h"
#include "tier/memory/id_tier_memory.h"
#include "tier/memory/id_tier_memory_special.h"
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

const data_id_transport_rules_t public_ruleset(
	all_tiers,
	all_intermediaries);

const data_id_transport_rules_t private_ruleset(
	all_tiers,
	{0});

const data_id_transport_rules_t cache_ruleset(
	all_mem_cache,
	{0});

const data_id_transport_rules_t mem_ruleset(
	std::vector<std::pair<uint8_t, uint8_t> >({{0, 0}}),
	{0});

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
	add_data_one_byte_vector(
		&(linked_list.first),
		~0);
	add_data_one_byte_vector(
		&(linked_list.second),
		~0);
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

id_t_ data_id_t::get_id(){
	return id;
}

void data_id_t::set_id(id_t_ id_){
	// del/add keeps cache in check
	ASSERT(get_id_type(id) == get_id_type(id_), P_ERR);
	const bool deletable =
		get_id_type(id) != TYPE_ID_TIER_STATE_T;
	if(deletable){
		mem_del_id(this);
	}
	set_id_uuid(&id, get_id_uuid(id_));
	set_id_hash(&id, get_id_hash(id_));
	if(deletable){
		mem_add_id(this);
	}
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
			 data_id_transport_rules_t transport_rules){
	if(ptr_ == nullptr){
		print("ptr_ is a nullptr", P_ERR);
	}
	data_vector.push_back(
		data_id_ptr_t(
			ptr_,
			size_,
			flags_,
			transport_rules));
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
		expand_id_set(
			linked_list.first), // preserves order
		expand_id_set(
			linked_list.second));
}

void data_id_t::set_linked_list(std::pair<std::vector<id_t_>, std::vector<id_t_> > tmp){
	for(uint64_t i = 0;i < tmp.first.size();i++){
		if(tmp.first[i] == get_id()){
			print("attempted to put myself in my own linked list", P_WARN);
			tmp.first.erase(
				tmp.first.begin()+i);
			i--;
		}
	}
	for(uint64_t i = 0;i < tmp.second.size();i++){
		if(tmp.second[i] == get_id()){
			print("attempted to put myself in my own linked list", P_WARN);
			tmp.second.erase(
				tmp.second.begin()+i);
			i--;
		}
	}
	linked_list.first =
		compact_id_set(
			tmp.first,
			true);
	linked_list.second =
		compact_id_set(
			tmp.second,
			true);
}

// Works by assuming tiers get more liberal the higher
// they go. Still painting with a broad brush, but cache
// and things not covered with this approach are few and
// far between

// TODO: get full-aesthetics and abstract this out to a
// milti-length base system

static void most_liberal_tier(
	std::vector<std::pair<uint8_t, uint8_t> > *tiers,
	std::pair<uint8_t, uint8_t> most){

	for(uint64_t c = 0;c < tiers->size();c++){
		if((*tiers)[c].first > most.first ||
		   ((*tiers)[c].first == most.first &&
		    (*tiers)[c].second > most.second)){
			tiers->erase(
				tiers->begin()+c);
			c--;
		}
	}
}

static void most_liberal_intermediary(
	std::vector<uint8_t> *intermediary,
	uint8_t most){

	for(uint64_t i = 0;i < intermediary->size();i++){
		if((*intermediary)[i] > most){
			intermediary->erase(
				intermediary->begin()+i);
			i--;
		}
	}
}

void data_id_t::set_most_liberal_rules(
	std::pair<uint8_t, uint8_t> tier_rule,
	uint8_t intermediary_rule){
	for(uint64_t i = 0;i < data_vector.size();i++){
		const data_id_transport_rules_t tmp_rules =
			data_vector[i].get_transport_rules();
		most_liberal_tier(
			&(data_vector[i].transport_rules.tier),
			tier_rule);
		most_liberal_intermediary(
			&(data_vector[i].transport_rules.intermediary),
			intermediary_rule);
	}
}

void data_id_t::set_most_liberal_rules(
	data_id_transport_rules_t rules){
	// These should be linked to all_* in id_tier, so we can
	// (somewhat) safely assume the last one is the most liberal
	set_most_liberal_rules(
		rules.tier[rules.tier.size()-1],
		rules.intermediary[rules.intermediary.size()-1]);
}

std::vector<id_t_> data_id_t::all_ids(){
	std::vector<id_t_> retval;
	for(uint64_t i = 0;i < data_vector.size();i++){
		if(data_vector[i].get_flags() == ID_DATA_ID){
			ASSERT(data_vector[i].get_ptr() != nullptr, P_ERR);
			retval.push_back(*reinterpret_cast<id_t_*>(data_vector[i].get_ptr()));
		}
	}
	return retval;
}
