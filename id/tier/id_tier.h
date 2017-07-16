#ifndef ID_TIER_H
#define ID_TIER_H

#include "../id.h"
#include "../id_api.h"

#define PTR_DATA(id_, type_) ((type_*)id_tier::mem::get_ptr(id_, #type_, 0, 0))
#define PTR_ID(id_, type_) (id_tier::mem::get_id_ptr(id_, #type_, 0, 0))

// Only until I re-implement cache
#define PTR_DATA_FAST PTR_DATA
#define PTR_ID_FAST PTR_ID

#define PTR_DATA_PRE PTR_DATA
#define PTR_ID_PRE PTR_ID

#define ID_TIER_MAJOR_MEM 0
#define ID_TIER_MAJOR_CACHE 1
#define ID_TIER_MAJOR_DISK 2
#define ID_TIER_MAJOR_LIBRARY 3
#define ID_TIER_MAJOR_NETWORK 4

#define ID_TIER_MINOR_CACHE_UNENCRYPTED_UNCOMPRESSED 0
#define ID_TIER_MINOR_CACHE_UNENCRYPTED_COMPRESSED 1
#define ID_TIER_MINOR_CACHE_ENCRYPTED_COMPRESSED 2

#define ID_TIER_MEDIUM_UNDEFINED 0
#define ID_TIER_MEDIUM_MEM 1
#define ID_TIER_MEDIUM_CACHE 2
#define ID_TIER_MEDIUM_DISK 3


#define ID_TIER_CACHE_GET(type_) (id_tier::lookup::type::from_tier(all_tiers, type_))
// expose more nitty gritties to the user later on
#define ID_TIER_DESTROY(id_) (id_tier::operation::del_id_from_state(id_tier::state_tier::optimal_state_vector_of_tier_vector(all_tiers), {id_}))


#define ID_TIER_INIT_STATE(medium) id_t_ id_tier_##medium##_init_state()
#define ID_TIER_DEL_STATE(medium) void id_tier_##medium##_del_state(id_t_ state_id)
#define ID_TIER_ADD_DATA(medium) void id_tier_##medium##_add_data(id_t_ state_id, std::vector<uint8_t> data)
#define ID_TIER_DEL_ID(medium) void id_tier_##medium##_del_id(id_t_ state_id, id_t_ id)
#define ID_TIER_GET_ID(medium) std::vector<uint8_t> id_tier_##medium##_get_id(id_t_ state_id, id_t_ id)
#define ID_TIER_GET_ID_MOD_INC(medium) mod_inc_t_ id_tier_##medium##_get_id_mod_inc(id_t_ state_id, id_t_ id)

typedef std::pair<id_t_, mod_inc_t_> id_buffer_t;

struct id_tier_state_t{
private:
	uint8_t medium = 0;
	uint8_t tier_major = 0;
	uint8_t tier_minor = 0;	
	uint64_t total_size_bytes = 0;
	uint64_t used_bytes = 0;
	uint64_t free_bytes = 0;
	std::vector<std::pair<id_t_, mod_inc_t_> > id_buffer;
	std::vector<uint8_t> allowed_extra;
	uint64_t last_state_refresh_micro_s = 0;
	uint64_t refresh_interval_micro_s = 0;
	void *payload = nullptr;
public:
	data_id_t id;
	id_tier_state_t();
	~id_tier_state_t();
	bool is_allowed_extra(extra_t_ extra_);
	GET_SET(medium, uint8_t);
	GET_SET(tier_major, uint8_t);
	GET_SET(tier_minor, uint8_t);
	GET_SET(total_size_bytes, uint64_t);
	GET_SET(used_bytes, uint64_t);
	GET_SET(free_bytes, uint64_t);
	void del_id_buffer(id_t_ id);
	ADD_DEL_VECTOR(id_buffer, id_buffer_t);
	GET_SET(id_buffer, std::vector<id_buffer_t>);
	GET_SET(allowed_extra, std::vector<uint8_t>);
	GET_SET(last_state_refresh_micro_s, uint64_t);
	GET_SET(refresh_interval_micro_s, uint64_t);
	GET_SET(payload, void*);
};

struct id_tier_medium_t{
public:
	id_t_ (*init_state)() = nullptr;
	void (*del_state)(id_t_) = nullptr;
	void (*add_data)(id_t_ state_id, std::vector<uint8_t> data) = nullptr;
	void (*del_id)(id_t_ state_id, id_t_ id) = nullptr;
	std::vector<uint8_t> (*get_id)(id_t_ state_id, id_t_ id) = nullptr;
	mod_inc_t_ (*get_id_mod_inc)(id_t_ state_id, id_t_ id) = nullptr;
	id_tier_medium_t(
		id_t_ (*init_state_)(),
		void (*del_state_)(id_t_ state_id),
		void (*add_data_)(id_t_, std::vector<uint8_t>),
		void (*del_id_)(id_t_, id_t_),
		std::vector<uint8_t> (*get_id_)(id_t_, id_t_),
		mod_inc_t_ (*get_id_mod_inc_)(id_t_, id_t_)){
		init_state = init_state_;
		del_state = del_state_;
		add_data = add_data_;
		del_id = del_id_;
		get_id = get_id_;
		get_id_mod_inc = get_id_mod_inc_;
	}
};

namespace id_tier{
	id_tier_medium_t get_medium(
		uint8_t medium_type);
	
	namespace state_tier{
		id_t_ only_state_of_tier(
			uint8_t tier_major,
			uint8_t tier_minor);
		std::vector<id_t_> optimal_state_vector_of_tier(
			uint8_t tier_major,
			uint8_t tier_minor);
		std::vector<id_t_> optimal_state_vector_of_tier_vector(
			std::vector<std::pair<uint8_t, uint8_t> > tier_vector);
	};
	namespace operation{
		void add_data_to_state(
			std::vector<id_t_> state_id,
			std::vector<std::vector<uint8_t> > data);
		void del_id_from_state(
			std::vector<id_t_> state_id,
			std::vector<id_t_> id);
		std::vector<std::vector<uint8_t> > get_data_from_state(
			std::vector<id_t_> state_id,
			std::vector<id_t_> id_vector);
		void shift_data_to_state(
			id_t_ start_state_id,
			id_t_ end_state_id,
			std::vector<id_t_> id_vector);
		std::vector<std::tuple<id_t_, uint8_t, uint8_t> > valid_state_with_id(
			id_t_ id);
		uint8_t fix_extra_flags_for_state(
			id_t_ state_id,
			uint8_t data_extra);
	};
	namespace lookup{
		namespace id_mod_inc{
			std::vector<std::pair<id_t_, mod_inc_t_> > from_tier(
				std::vector<std::pair<uint8_t, uint8_t> > tier_vector);
			std::vector<std::pair<id_t_, mod_inc_t_> > from_state(
				std::vector<id_t_> state_vector);
			std::vector<std::pair<id_t_, mod_inc_t_> > from_state(
				id_tier_state_t* tier_state_ptr);
		};
		namespace ids{
			std::vector<id_t_> from_tier(
				std::vector<std::pair<uint8_t, uint8_t> > tier_vector);
			std::vector<id_t_> from_state(
				std::vector<id_t_> state_vector);
			std::vector<id_t_> from_state(
				id_tier_state_t* tier_state_ptr);
		};
		namespace type{
			std::vector<id_t_> from_tier(
				std::vector<std::pair<uint8_t, uint8_t> > tier_vector,
				uint8_t type);
			std::vector<id_t_> from_tier(
				std::vector<std::pair<uint8_t, uint8_t> > tier_vector,
				std::string type); // ID_TIER_CACHE_GET
			std::vector<id_t_> from_state(
				std::vector<id_t_> state_vector,
				uint8_t type);
			std::vector<id_t_> from_state(
				id_tier_state_t* tier_state_ptr,
				uint8_t type);
		};
	};
	
	std::vector<id_t_> fetch_id_buffer(
		id_t_ state_id);
	
	namespace mem{
		data_id_t *get_id_ptr(
			id_t_ id,
			type_t_ type,
			uint8_t high_major,
			uint8_t high_minor);
		data_id_t *get_id_ptr(
			id_t_ id,
			std::string type,
			uint8_t high_major,
			uint8_t high_minor);
		void *get_ptr(
			id_t_ id,
			std::string type,
			uint8_t high_major,
			uint8_t high_minor);
	};
};

extern void id_tier_init();
extern void id_tier_loop();
extern void id_tier_close();

extern std::vector<id_tier_medium_t> id_tier_mediums;	

extern std::vector<std::pair<uint8_t, uint8_t> > all_tiers;

#include "id_tier_memory.h"
#endif
