#include "id.h"
#include "../util.h"
#ifndef ID_API_H
#define ID_API_H
#include "tuple"
#include "unistd.h"
#include "sys/resource.h"
#include "cstdio"

/*
  Redefined and more unified ID API. 
 */

// Function macros for common ID API calls

#define PTR_DATA(id, type) ((type*)id_api::array::ptr_data(id, #type))
#define PTR_ID(id, type) (id_api::array::ptr_id(id, #type))

namespace id_api{
	namespace array{
		data_id_t *ptr_id(uint64_t id,
				  std::string type);
		data_id_t *ptr_id(uint64_t id,
				  std::array<uint8_t, TYPE_LENGTH> type);
		void *ptr_data(uint64_t id,
				  std::string type);
		void *ptr_data(uint64_t id,
				  std::array<uint8_t, TYPE_LENGTH> type);
		void add(data_id_t *ptr);
		void del(id_t_ id); // no type
		id_t_ add_data(std::vector<uint8_t> data_);
		std::vector<uint64_t> sort_by_rsa_pubkey(std::vector<uint64_t> tmp);
		std::vector<uint64_t> get_forward_linked_list(uint64_t id);
	}
	namespace cache{
		// get_type_vector_ptr should never be used outside of id_api.cpp	
		void add(uint64_t id,
			 std::array<uint8_t, TYPE_LENGTH> type);
		void add(uint64_t id,
			 std::string type);
		void del(uint64_t id,
			 std::array<uint8_t, TYPE_LENGTH> type);
		void del(uint64_t id,
			 std::string type);
		std::vector<uint64_t> get(std::array<uint8_t, TYPE_LENGTH> type);
		std::vector<uint64_t> get(std::string type);
	}
	namespace linked_list{
		// next and previous are in the id itself, no interdependency
		void link_vector(std::vector<uint64_t> vector);
	};
	std::vector<uint64_t> get_all();
	void free_mem();
	void destroy(id_t_ id);
	void destroy_all_data();
};
#endif
