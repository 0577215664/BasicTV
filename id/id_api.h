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
#define ID_LOOKUP_FAST (1 << 0)

#define PTR_DATA_FAST(id, type) ((type*)id_api::array::ptr_data(id, #type, ID_LOOKUP_FAST))
#define PTR_ID_FAST(id, type) (id_api::array::ptr_id(id, #type, ID_LOOKUP_FAST))

#define PTR_DATA(id, type) ((type*)id_api::array::ptr_data(id, #type))
#define PTR_ID(id, type) (id_api::array::ptr_id(id, #type))

#define ID_API_IMPORT_FROM_DISK (1 << 0)
#define ID_API_IMPORT_FROM_NET (1 << 1)

namespace id_api{
	namespace array{
		data_id_t *ptr_id(id_t_ id,
				  std::string type,
				  uint8_t flags = 0);
		data_id_t *ptr_id(id_t_ id,
				  type_t_ type,
				  uint8_t flags = 0);
		void *ptr_data(id_t_ id,
				  std::string type,
				  uint8_t flags = 0);
		void *ptr_data(id_t_ id,
				  type_t_ type,
				  uint8_t flags = 0);
		void add(data_id_t *ptr);
		void del(id_t_ id); // no type
		id_t_ add_data(std::vector<uint8_t> data_);
		// used for quick lookups of my own type (encrypt_priv_key_t,
		// net_peer_t, etc.)
		id_t_ fetch_one_from_hash(type_t_ type,
					  std::array<uint8_t, 32> sha_hash);
		// TODO: create a version that throws on more than one
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
	}
	namespace linked_list{
		// next and previous are in the id itself, no interdependency
		void link_vector(std::vector<id_t_> vector);
		// perhaps redefine this for standard when that's needed
		uint64_t distance_fast(id_t_ linked_list_id,
				       id_t_ target_id); // only used in stats
		std::vector<id_t_> get_forward_linked_list(id_t_ id);
	};
	namespace sort{
		std::vector<id_t_> fingerprint(std::vector<id_t_> tmp);
		/*
		  Perhaps sort by last access time (when that gets implemented)?
		 */
	};
	namespace disk{
		/*
		  All disks should be abstracted out, lookups and queries into
		  larger tables should be fine for now and forever
		 */
		void save(std::vector<id_t_>);
		void save(id_t_);
		void load(std::vector<id_t_>);
		void load(id_t_);
		std::string get_filename(id_t_);
	};
	namespace import{
		void load_all_of_type(std::string type, uint8_t flags);
		// used for saving, not needed for network (too slow as well)
		uint64_t ver_on_disk(id_t_);
		// used internally, called by id_api::array::ptr_* and others
		void load_from_net(id_t_);
	};
	// metadata from the raw, unencrypted, and decompressed string
	namespace metadata{
		id_t_ get_id_from_data(std::vector<uint8_t> raw_data);
		type_t_ get_type_from_data(std::vector<uint8_t> raw_data);
	};
	namespace bulk_fetch{
		std::vector<uint64_t> mod(std::vector<id_t_> vector);
	};
	std::vector<id_t_> get_all();
	void free_mem();
	void destroy(id_t_ id);
	void destroy_all_data();
};

#endif
