#include "id.h"
#include "../util.h"
#ifndef ID_API_H
#define ID_API_H
#include <tuple>
#include <unistd.h>
#include <sys/resource.h>
#include <cstdio>

#define ASSERT_VALID_ID(id) id_api::assert_valid_id(id)

namespace id_api{
	namespace linked_list{
		// next and previous are in the id itself, no interdependency
		void link_vector(std::vector<id_t_> vector);

		namespace list{
			std::vector<id_t_> by_distance(id_t_ start_id, int64_t pos);
			std::vector<id_t_> by_distance_until_match(id_t_ start_id, int64_t pos, id_t_ target_id);
		};
		int64_t pos_in_linked_list(id_t_ ref_id, id_t_ goal_id, uint64_t max_search_radius);
	};
	namespace sort{
		std::vector<id_t_> fingerprint(std::vector<id_t_> tmp);
		/*
		  Perhaps sort by last access time (when that gets implemented)?
		*/
	};
	// comment out import really soon
	namespace import{
		void load_all_of_type(std::string type, uint8_t flags);
		// used for saving, not needed for network (too slow as well)
		uint64_t ver_on_disk(id_t_);
		// used internally, called by id_api::array::ptr_* and others
		void load_from_net(id_t_);
	};
	namespace bulk_fetch{
		std::vector<uint64_t> mod(std::vector<id_t_> vector);
	};
	std::vector<id_t_> get_all();
	void free_mem();
	void add_data(std::vector<uint8_t> data);
	void destroy(id_t_ id);
	void destroy_all_data();
	void print_id_vector(std::vector<id_t_> vector, uint32_t p_l);
	void assert_valid_id(id_t_ id);
	void assert_valid_id(std::vector<id_t_> id);
	namespace raw{
		// encryption ID is pulled from ID hash
		std::vector<uint8_t> encrypt(std::vector<uint8_t>);
		std::vector<uint8_t> decrypt(std::vector<uint8_t>);
		std::vector<uint8_t> compress(std::vector<uint8_t>);
		std::vector<uint8_t> decompress(std::vector<uint8_t>);
		// only variables that unencrypted IDs can have
		id_t_ fetch_id(std::vector<uint8_t>);
		extra_t_ fetch_extra(std::vector<uint8_t>);
		type_t_ fetch_type(std::vector<uint8_t>);
		mod_inc_t_ fetch_mod_inc(std::vector<uint8_t>);

		std::vector<uint8_t> strip_to_lowest_rules(
			std::vector<uint8_t> data,
			uint8_t network_rules,
			uint8_t export_rules,
			uint8_t peer_rules);
	};
};

extern std::vector<type_t_> encrypt_blacklist;
extern bool encrypt_blacklist_type(type_t_ type);

#endif
