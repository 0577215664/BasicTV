#ifndef ID_TIER_H
#define ID_TIER_H

#include "../id.h"
#include "../id_api.h"

#include "benchmark/id_tier_benchmark.h"
#include "storage/id_tier_storage.h"
#include "id_tier_control.h"

#define PTR_DATA(id_, type_) ((type_*)id_tier::mem::get_ptr(id_, #type_, all_tiers))
#define PTR_ID(id_, type_) (id_tier::mem::get_id_ptr(id_, #type_, all_tiers))

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
#define ID_TIER_MEDIUM_NETWORK 4

/*
  Only tiers that should exist are:
  0.0 (in memory)
  1.0 (cache, unencrypted and uncompressed)
  1.1 (cache, unencrypted and compressed)
  1.2 (cache, encrypted and compressed)
  2.0 (all disk)
  4.0 (all network peers)

  Library isn't used right now, and probably should be taken out honestly

  TODO:
  1. Make tiers bidirectional (tier can originate requests as well as receive
     them), and allow only sending IDs it requests (read. network peer)
  2. Store all timing information in the normal tier state, but allow direct
     writing directly from the calling function (GET_HINT_ID can start the
     timer only if it actually does something).
 */

#define ID_TIER_CACHE_GET(type_) (id_tier::lookup::type::from_tier(all_tiers, type_))
// expose more nitty gritties to the user later on
#define ID_TIER_DESTROY(id_) (id_tier::operation::del_id_from_state(id_tier::state_tier::optimal_state_vector_of_tier_vector(all_tiers), {id_}))

// All of these can throw on errors

/*
  Instead of having direct interfaces to the tiers, which isn't a good idea for
  network integration because of latencies and the like, we have direct read and
  write access to the buffers. The id_tier API writes data to the buffers, 
  ID_TIER_LOOP handles filling those.

  Now since there are pretty low latencies for the majority of tiers, ID_TIER_LOOP
  can be called right after tier 0 calls
*/

#define ID_TIER_INIT_STATE(medium) id_t_ id_tier_##medium##_init_state()
#define ID_TIER_DEL_STATE(medium) void id_tier_##medium##_del_state(id_t_ state_id)
#define ID_TIER_LOOP(medium) void id_tier_##medium##_loop(id_t_ state_id)

// shift over to a private context
#define ID_TIER_ADD_DATA(medium) void id_tier_##medium##_add_data(id_t_ state_id, std::vector<uint8_t> data)
#define ID_TIER_DEL_ID(medium) void id_tier_##medium##_del_id(id_t_ state_id, id_t_ id)
#define ID_TIER_GET_ID(medium) std::vector<uint8_t> id_tier_##medium##_get_id(id_t_ state_id, id_t_ id)
#define ID_TIER_GET_HINT_ID(medium) void id_tier_##medium##_get_hint_id(id_t_ state_id, id_t_ id)
#define ID_TIER_UPDATE_CACHE(medium) void id_tier_##medium##_update_cache(id_t_ state_id)

struct id_tier_transport_entry_t{
private:
	id_t_ payload_id;
	std::vector<uint8_t> payload;

public:
	bool operator==(const id_tier_transport_entry_t &rhs){
		return payload_id == rhs.payload_id &&
		payload == rhs.payload;
	}
	bool operator!=(const id_tier_transport_entry_t &rhs){
		return !(*this == rhs);
	}
	
	GET_SET_S(payload_id, id_t_);
	GET_SET_S(payload, std::vector<uint8_t>);
};

#define ID_TIER_OPERATION_UNDEFINED 0
#define ID_TIER_OPERATION_DEL 1

struct id_tier_operation_entry_t{
private:
	std::vector<id_t_> ids;
	uint8_t operation = 0;
public:
	GET_SET_S(ids, std::vector<id_t_>);
	GET_SET_S(operation, uint8_t);
};

struct id_tier_state_t{
private:
	uint8_t medium = 0;
	uint8_t tier_major = 255;
	uint8_t tier_minor = 255;	
	
	void *payload = nullptr;
public:
	data_id_t id;
	id_tier_state_control_t control;
	id_tier_state_storage_t storage;
	id_tier_state_benchmark_t benchmark;
	
	// Reading/writing queues
	std::vector<id_tier_transport_entry_t> inbound_transport;
	std::vector<id_tier_transport_entry_t> outbound_transport;

	std::vector<id_tier_operation_entry_t> operations;
	
	
	id_tier_state_t();
	~id_tier_state_t();
	GET_SET(medium, uint8_t);
	GET_SET(tier_major, uint8_t);
	GET_SET(tier_minor, uint8_t);
	
	GET_SET(payload, void*);
};

/*
  id_tier_medium_t calls hang until either a timeout, a valid response, or an
  excpetion are thrown. 

  GET_HINT_ID sends a request for data down an id_tier, saves the response in
  an internal cache, and from the internal cache it dishes the data out

  Because networking is probably the most latent ID tier by a long shot, this
  GET_HINT_ID only does something on that front, the rest of the calls don't
  do anything and let the full latency happen in GET_ID
 */

struct id_tier_medium_t{
public:
	id_t_ (*init_state)() = nullptr;
	void (*del_state)(id_t_) = nullptr;
	void (*loop)(id_t_) = nullptr;
	void (*update_cache)(id_t_) = nullptr;
	id_tier_medium_t(
		id_t_ (*init_state_)(),
		void (*del_state_)(id_t_ state_id),
		void (*loop_)(id_t_ state_id),
		void (*update_cache_)(id_t_ state_id)){
		init_state = init_state_;
		del_state = del_state_;
		loop = loop_;
		update_cache = update_cache_;
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
		namespace smart{
			// specific tiers aren't delegated, just load it
			// to what the program thinks is the best (directly
			// load to tiers to fill requests, redundancy, whatnot)
			std::vector<id_t_> add_data(std::vector<std::vector<uint8_t> > data);		
		};
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
			std::vector<id_t_> *id_vector);
		void shift_data_to_state(
			id_tier_state_t *start_state_ptr,
			id_tier_state_t *end_state_ptr,
			std::vector<id_t_> *id_vector);
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
		// although tier 0 can be listed here without triggering an
		// error, it will always be searched, since it is the only
		// way to return a valid pointer
		data_id_t *get_id_ptr(
			id_t_ id,
			type_t_ type,
			std::vector<std::pair<uint8_t, uint8_t> > tier_vector);
		data_id_t *get_id_ptr(
			id_t_ id,
			std::string type,
			std::vector<std::pair<uint8_t, uint8_t> > tier_vector);
		void *get_ptr(
			id_t_ id,
			std::string type,
			std::vector<std::pair<uint8_t, uint8_t> > tier_vector);
	};
};

extern void id_tier_init();
extern void id_tier_loop();
extern void id_tier_close();

extern std::vector<id_tier_medium_t> id_tier_mediums;
extern std::vector<type_t_> memory_locked;

const std::vector<std::pair<uint8_t, uint8_t> > all_tiers = {
	{ID_TIER_MAJOR_MEM, 0},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_UNENCRYPTED_UNCOMPRESSED},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_UNENCRYPTED_COMPRESSED},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_ENCRYPTED_COMPRESSED},
	{ID_TIER_MAJOR_DISK, 0}
};

const std::vector<std::pair<uint8_t, uint8_t> > all_mem_cache = {
	{ID_TIER_MAJOR_MEM, 0},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_UNENCRYPTED_UNCOMPRESSED},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_UNENCRYPTED_COMPRESSED},
	{ID_TIER_MAJOR_CACHE, ID_TIER_MINOR_CACHE_ENCRYPTED_COMPRESSED}
};

#include "memory/id_tier_memory.h"
#include "id_tier_define.h"
#endif
