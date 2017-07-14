#include "test.h"

#include "../id/id_api.h"
// type of choice
#include "../net/proto/net_proto_request.h"
// SHA-256 hash for ID spoofing
#include "../encrypt/encrypt.h"

template <typename T>
static void unload_nuke_reload(T ptr){
	if(true){
		std::vector<uint8_t> data = 
			(*ptr)->id.export_data(
				0,
				0,
				ID_DATA_NETWORK_RULE_NEVER,
				ID_DATA_EXPORT_RULE_NEVER,
				ID_DATA_PEER_RULE_NEVER);
		delete (*ptr);
		*ptr = new net_proto_id_request_t; // change this as test_id_transport chanages
		(*ptr)->id.import_data(
			data);
	}
}

void test::id_system::transport::proper(){
	const std::vector<id_t_> old =
		{production_priv_key_id,
		 production_priv_key_id,
		 production_priv_key_id};
	net_proto_id_request_t *id_request_ptr =
		new net_proto_id_request_t;
	id_request_ptr->set_ids(
		old);
	id_api::print_id_vector(
		id_request_ptr->get_ids(),
		P_DEBUG);
	unload_nuke_reload(&id_request_ptr);
	id_api::print_id_vector(
		id_request_ptr->get_ids(),
		P_DEBUG);
	ASSERT(old == id_request_ptr->get_ids(), P_ERR);
	id_api::print_id_vector(old, P_SPAM);
	id_api::print_id_vector(id_request_ptr->get_ids(), P_SPAM);
	delete id_request_ptr;
	id_request_ptr = nullptr;
}

void test::id_system::transport::import::random(){
	net_proto_id_request_t *tmp_type_ptr =
		new net_proto_id_request_t;
	tmp_type_ptr->id.import_data(
		true_rand_byte_vector(
			true_rand(
				0, 65536)));
	delete tmp_type_ptr;
	tmp_type_ptr = nullptr;
				
}

void test::id_system::id_set::proper(){
	std::vector<id_t_> id_set;
	std::array<uint8_t, 32> hash;
	uint32_t hash_count = 1;
	for(uint64_t i = 1;i < 1024;i++){
		if(true_rand(0, 30) == 0){
			hash = encrypt_api::hash::sha256::gen_raw(
				true_rand_byte_vector(64));
			hash_count++;
		}
		id_t_ tmp_id;
		set_id_uuid(&tmp_id, i);
		set_id_hash(&tmp_id, hash);
		set_id_type(&tmp_id, TYPE_NET_PROTO_PEER_T);
		id_set.push_back(tmp_id);
	}
	P_V(hash_count, P_NOTE);
	ASSERT(expand_id_set(compact_id_set(id_set)) == id_set, P_ERR);
}
