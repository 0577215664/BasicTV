#include "id.h"
#include "../util.h"
#ifndef ID_API_H
#define ID_API_H
#include <tuple>
#include <unistd.h>
#include <sys/resource.h>
#include <cstdio>

/*
  Redefined and more unified ID API. 
 */

// Function macros for common ID API calls

/*
  Currently only used in stats collection for sockets. If the information
  doesn't exist locally, then it has fallen out of use. This DOES allow for
  re-loading of cached data (since it shouldn't be too slow)
 */

// can refer to id_disk_index_t
#define ID_LOOKUP_FAST 1
// cannot refer to id_disk_index_t, only looks in memory
#define ID_LOOKUP_MEM 2
// pre loaded as a type, no cache
#define ID_LOOKUP_PRE 3

#define ID_LOOKUP_DISK 4

// #define PTR_DATA_PRE(id, type) ((type*)id_api::array::ptr_data(id, #type, ID_LOOKUP_PRE))
// #define PTR_ID_PRE(id, type) (id_api::array::ptr_id(id, #type, ID_LOOKUP_PRE))

// #define PTR_DATA_MEM(id, type) ((type*)id_api::array::ptr_data(id, #type, ID_LOOKUP_MEM))
// #define PTR_ID_MEM(id, type) (id_api::array::ptr_id(id, #type, ID_LOOKUP_MEM))

// #define PTR_DATA_FAST(id, type) ((type*)id_api::array::ptr_data(id, #type, ID_LOOKUP_FAST))
// #define PTR_ID_FAST(id, type) (id_api::array::ptr_id(id, #type, ID_LOOKUP_FAST))

// legacy

#define PTR_DATA_FAST PTR_DATA
#define PTR_ID_FAST PTR_ID
#define PTR_DATA_MEM PTR_DATA
#define PTR_ID_MEM PTR_ID
#define PTR_DATA_PRE PTR_DATA
#define PTR_ID_PRE PTR_ID

#define PTR_DATA(id, type) ((type*)id_api::array::ptr_data(id, #type))
#define PTR_ID(id, type) (id_api::array::ptr_id(id, #type))

#define ID_API_IMPORT_FROM_DISK (1 << 0)
#define ID_API_IMPORT_FROM_NET (1 << 1)

/*
  TODO: drastically simplify and clarify this section

  I feel like a lot of the code declarations here are better subdivided into 
  smaller sections that are only public to the ID code itself, like loading
  data from the disk into memory, and other small stuff
 */

namespace id_api{
	namespace array{
		data_id_t *ptr_id(id_t_ id,
				  std::string type);
		data_id_t *ptr_id(id_t_ id,
				  type_t_ type);
		void *ptr_data(id_t_ id,
				  std::string type);
		void *ptr_data(id_t_ id,
				  type_t_ type);
		void add(data_id_t *ptr);
		void del(id_t_ id); // no type
		id_t_ add_data(std::vector<uint8_t> data_, bool raw = false);
		// used for quick lookups of my own type (encrypt_priv_key_t,
		// net_peer_t, etc.)
		id_t_ fetch_one_from_hash(type_t_ type,
					  std::array<uint8_t, 32> sha_hash);
		// TODO: create a version that throws on more than one
		uint64_t get_id_count();
	}
	namespace cache{
		// get_type_vector_ptr should never be used outside of id_api.cpp	
		void add(id_t_ id,
			 type_t_ type);
		void add(id_t_ id,
			 std::string type);
		void del(id_t_ id,
			 type_t_ type);
		void del(id_t_ id,
			 std::string type);
		std::vector<id_t_> get(type_t_ type);
		std::vector<id_t_> get(std::string type);

		// really two different things
		void hint_increment_id(id_t_ id);
		void hint_decrement_id(id_t_ id);
		void add_data(std::vector<uint8_t> data);
		void load_id(id_t_ id);
	}
	namespace linked_list{
		// next and previous are in the id itself, no interdependency
		void link_vector(std::vector<id_t_> vector);

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
	};
};

extern std::vector<std::string> id_gdb_lookup(const char *hex_id);

#endif
