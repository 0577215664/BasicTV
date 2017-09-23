#include "test.h"
#include "../id/id_api.h"
#include "../id/id_transport.h"
// type of choice
#include "../tv/tv_item.h"
// SHA-256 hash for ID spoofing
#include "../encrypt/encrypt.h"
// Linked list
#include "../tv/tv_frame_audio.h"

template <typename T>
static void unload_nuke_reload(T ptr){
	if(true){
		std::vector<uint8_t> data = 
			(*ptr)->id.export_data(
				0);
		delete (*ptr);
		*ptr = new tv_item_t; // change this as test_id_transport chanages
		(*ptr)->id.import_data(
			data);
	}
}

#define SIMPLE_TRANSPORT_TEST(import_func, export_func, subvector_size) \
	if(true){							\
		const std::vector<uint8_t> tmp_correct = std::vector<uint8_t>(rand_data.begin(), rand_data.begin()+subvector_size); \
		export_func(&tmp, tmp_correct);				\
		const std::vector<uint8_t> tmp_output = import_func(&tmp); \
		ASSERT(tmp_output == tmp_correct, P_ERR);		\
		ASSERT(tmp.size() == 0, P_ERR);				\
	}

static void static_size_payload_test(){
	std::vector<uint8_t> tmp;
	uint64_t static_export = true_rand(0, ~static_cast<uint64_t>(0));
	uint64_t static_import = 0;
	export_static_size_payload(
		&tmp,
		reinterpret_cast<uint8_t*>(&static_export),
		sizeof(static_export));
	import_static_size_payload(
		&tmp,
		reinterpret_cast<uint8_t*>(&static_import),
		sizeof(static_import));
	ASSERT(static_import == static_export, P_ERR);
	ASSERT(tmp.size() == 0, P_ERR);
}

static void stringify_rules_test(){
	std::vector<uint8_t> tmp;
	const data_id_transport_rules_t tmp_rules(
		all_tiers,
		all_intermediaries);
	stringify_rules(
		&tmp,
		tmp_rules);
	ASSERT(unstringify_rules(
		       &tmp) == tmp_rules, P_ERR);
	ASSERT(tmp.size() == 0, P_ERR);
}

void test::id_system::transport::core_functions(){
	std::vector<uint8_t> rand_data =
		true_rand_byte_vector(
			65536*2); // three byte size
	std::vector<uint8_t> tmp;
	for(uint64_t b = 0;b < 255;b++){
		SIMPLE_TRANSPORT_TEST(
			import_8bit_size_payload,
			export_8bit_size_payload,
			b);
	}
	for(uint64_t b = 0;b < 1024;b++){
		SIMPLE_TRANSPORT_TEST(
			import_dynamic_size_payload,
			export_dynamic_size_payload,
			b);
	}
	static_size_payload_test();
	stringify_rules_test();
	
}
	
void test::id_system::transport::proper(){
	const std::vector<id_t_> old =
		{production_priv_key_id,
		 production_priv_key_id,
		 production_priv_key_id};
	tv_item_t *item_ptr =
		new tv_item_t;
	
	item_ptr->add_frame_id(
		old);
	id_api::print_id_vector(
		item_ptr->get_frame_id_vector().at(0),
		P_DEBUG);
	unload_nuke_reload(&item_ptr);
	id_api::print_id_vector(
		item_ptr->get_frame_id_vector().at(0),
		P_DEBUG);
	ASSERT(old == item_ptr->get_frame_id_vector().at(0), P_ERR);
	id_api::print_id_vector(old, P_SPAM);
	id_api::print_id_vector(item_ptr->get_frame_id_vector().at(0), P_SPAM);
	delete item_ptr;
	item_ptr = nullptr;
}

void test::id_system::transport::import::random(){
 	tv_item_t *tmp_type_ptr =
		new tv_item_t;
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
	ASSERT(expand_id_set(compact_id_set(id_set, true)) == id_set, P_ERR);
	ASSERT(expand_id_set(compact_id_set(id_set, false)) == id_set, P_ERR);
}

void test::id_system::linked_list(){
	std::vector<id_t_> frame_audio_vector;
	for(uint64_t i = 0;i < 16;i++){
		frame_audio_vector.push_back(
			(new tv_frame_audio_t)->id.get_id());
	}
	for(uint64_t i = 1;i < frame_audio_vector.size();i++){
		print("creating new linked list of depth " + std::to_string(i), P_SPAM);
		id_api::linked_list::link_vector(
			frame_audio_vector,
			i);
		for(uint64_t a = 0;a < frame_audio_vector.size();a++){
			data_id_t *frame_id_ptr =
				PTR_ID(frame_audio_vector[a], );
			const std::pair<std::vector<id_t_>, std::vector<id_t_> > linked_list_ =
				frame_id_ptr->get_linked_list();
			frame_id_ptr->set_linked_list(
				std::make_pair(
					std::vector<id_t_>({}),
					std::vector<id_t_>({})));
			P_V(linked_list_.first.size(), P_VAR);
			P_V(linked_list_.second.size(), P_VAR);
			ASSERT(linked_list_.first.size() <= i, P_ERR);
			ASSERT(linked_list_.second.size() <= i, P_ERR);			
			if(a > i){
				// ASSERT(linked_list_.first.size() == i, P_ERR);
			}
			if(a < 16-i){
				// ASSERT(linked_list_.second.size() == i, P_ERR);
			}
			bool contains_itself_backwards =
				std::find(linked_list_.first.begin(),
					  linked_list_.first.end(),
					  frame_id_ptr->get_id()) != linked_list_.first.end();
			bool contains_itself_forwards =
				std::find(linked_list_.second.begin(),
					  linked_list_.second.end(),
					  frame_id_ptr->get_id()) != linked_list_.second.end();
			ASSERT(contains_itself_backwards == false, P_ERR);
			ASSERT(contains_itself_forwards == false, P_ERR);
		}
	}
	for(uint64_t i = 0;i < 16;i++){
		ID_TIER_DESTROY(frame_audio_vector[i]);
	}
}
