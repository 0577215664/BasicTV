#ifndef ID_H
#define ID_H
#include <cstdint>
#include <fstream>
#include <array>
#include <vector>
#include <random>
#include <cstdlib>

#include <algorithm>
#include <functional>

/*
  id_t: ID and pointer system for the networking system
 */
#define GET_CONST_PTR_RAW(data_to_get, type, id_str) type const * get_const_ptr_##data_to_get(){return (type const *)&data_to_get;}

#define GET_RAW(data_to_get, type, id_str) type get_##data_to_get() const {return data_to_get;}GET_CONST_PTR_RAW(data_to_get, type, id_str);


#define SET_RAW(data_to_set, type, id_str) void set_##data_to_set(const type datum){if(data_to_set != datum){id_str;}data_to_set = (type)datum;}	
#define GET_ID_RAW(data_to_get, id_str) id_t_ get_##data_to_get() const {if(data_to_get == ID_BLANK_ID){print(#data_to_get" is a nullptr (getting)", P_WARN);}return data_to_get;}
#define SET_ID_RAW(data_to_set, id_str) void set_##data_to_set(id_t_ datum){if(data_to_set != datum){id_str;}if(datum == ID_BLANK_ID){print(#data_to_set" is a nullptr (setting)", P_WARN);}data_to_set = datum;}
#define GET_SET_ID_RAW(data, id_str) GET_RAW(data, id_t_, id_str);SET_RAW(data, id_t_, id_str)
#define GET_SET_RAW(data, type, id_str) GET_RAW(data, type, id_str);SET_RAW(data, type, id_str)

#define FULL_VECTOR_CONTROL_RAW(data_to_set, type, id_str) ADD_DEL_VECTOR_RAW(data_to_set, type, id_str)
#define ADD_DEL_VECTOR_RAW(data_to_set, type, id_str)				\
	void add_##data_to_set(type datum){id_str;for(uint64_t i = 0;i < data_to_set.size();i++){if(data_to_set[i]==datum){return;}}data_to_set.push_back(datum);} \
	void del_##data_to_set(type datum){id_str;for(uint64_t i = 0;i < data_to_set.size();i++){if(data_to_set[i]==datum){data_to_set.erase(data_to_set.begin()+i);break;}}} \
	type get_elem_##data_to_set(uint64_t pos){return data_to_set.at(pos);} \
	void append_##data_to_set(std::vector<type> datum){id_str;data_to_set.insert(data_to_set.end(), datum.begin(), datum.end());} \
	std::vector<type> pull_erase_until_entry_##data_to_set(type datum){std::vector<type> retval;uint64_t dist;if((dist = std::distance(data_to_set.begin(), std::find(data_to_set.begin(), data_to_set.end(), datum))) != data_to_set.size()){id_str;retval = std::vector<type>(data_to_set.begin(), data_to_set.begin()+dist);data_to_set.erase(data_to_set.begin(), data_to_set.begin()+dist);}return retval;} \
	std::vector<type> pull_erase_until_pos_##data_to_set(uint64_t entry){std::vector<type> retval;uint64_t dist = entry;ASSERT(data_to_set.size() >= dist, P_UNABLE);id_str;retval = std::vector<type>(data_to_set.begin(), data_to_set.begin()+dist);data_to_set.erase(data_to_set.begin(), data_to_set.begin()+dist);return retval;} \
	uint64_t get_size_##data_to_set(){return data_to_set.size();}	\
	uint64_t find_iter_##data_to_set(std::function<bool(const type)> function_){return std::distance(data_to_set.begin(),std::find_if(data_to_set.begin(), data_to_set.end(), function_));}	\
	uint64_t find_##data_to_set(type datum){return std::distance(data_to_set.begin(), std::find(data_to_set.begin(), data_to_set.end(), datum));}\
	uint64_t search_dist_##data_to_set(std::vector<type> datum){return std::distance(data_to_set.begin(), std::search(data_to_set.begin(), data_to_set.end(), datum.begin(), datum.end()));}\

// no prefix == standard exportable datatype, refer to id through mod_inc normally

#define GET(a, b) GET_RAW(a, b, id.mod_inc())
#define GET_CONST_PTR(a, b) GET_CONST_PTR_RAW(a, b, id.mod_inc())
#define SET(a, b) GET_SET_RAW(a, b, id.mod_inc())
#define GET_ID(a) GET_ID_RAW(a, id.mod_inc())
#define SET_ID(a) SET_ID_RAW(a, id.mod_inc())
#define GET_SET_ID(a) GET_SET_ID_RAW(a, id.mod_inc())
#define GET_SET(a, b) GET_SET_RAW(a, b, id.mod_inc())
#define FULL_VECTOR_CONTROL(a, b) FULL_VECTOR_CONTROL_RAW(a, b, id.mod_inc())
#define ADD_DEL_VECTOR(a, b) FULL_VECTOR_CONTROL(a, b)

// V == Virtual inheritance (id is stored as a ptr)

#define GET_V(a, b) GET_RAW(a, b, id->mod_inc())
#define GET_CONST_PTR_V(a, b) GET_CONST_PTR_RAW(a, b, id->mod_inc())
#define SET_V(a, b) GET_SET(a, b, id->mod_inc())
#define GET_ID_V(a) GET_ID(a, id->mod_inc())
#define SET_ID_V(a) SET_ID(a, id->mod_inc())
#define GET_SET_ID_V(a) GET_SET_ID_RAW(a, id->mod_inc())
#define GET_SET_V(a, b) GET_SET_RAW(a, b, id->mod_inc())
#define FULL_VECTOR_CONTROL_V(a, b) FULL_VECTOR_CONTROL_RAW(a, b, id->mod_inc())
#define ADD_DEL_VECTOR_V(a, b) FULL_VECTOR_CONTROL_V(a, b)

// S == Simple (no ID to register with)

#define GET_S(a, b) GET_RAW(a, b, )
#define GET_CONST_PTR_S(a, b) GET_CONST_PTR_RAW(a, b, )
#define SET_S(a, b) GET_SET(a, b, )
#define GET_ID_S(a) GET_ID(a, )
#define SET_ID_S(a) SET_ID(a, )
#define GET_SET_ID_S(a) GET_SET_ID_RAW(a, )
#define GET_SET_S(a, b) GET_SET_RAW(a, b, )
#define FULL_VECTOR_CONTROL_S(a, b) FULL_VECTOR_CONTROL_RAW(a, b, )
#define ADD_DEL_VECTOR_S(a, b) FULL_VECTOR_CONTROL_S(a, b)


#define ADD_DATA(x, y) id.add_data_raw((uint8_t*)&x, sizeof(x), y)
#define ADD_DATA_PTR(x, y) id->add_data_raw((uint8_t*)&x, sizeof(x), y)

#define CONTINUE_IF_DIFF_OWNER(id_one, id_two) if(likely(get_id_hash(id_one) != get_id_hash(id_two))){continue;}

typedef std::array<uint8_t, 41> id_t_;
typedef uint64_t uuid_t_;
typedef uint8_t type_t_;
typedef std::array<uint8_t, 32> hash_t_;
typedef uint8_t extra_t_;
typedef uint64_t mod_inc_t_;

const id_t_ blank_id = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
const hash_t_ blank_hash = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

#define ID_BLANK_ID blank_id
#define ID_BLANK_TYPE (0)
#define ID_BLANK_HASH (blank_hash)

#define ID_DATA_ID 1
#define ID_DATA_BYTE_VECTOR 2
#define ID_DATA_EIGHT_BYTE_VECTOR 3
#define ID_DATA_BYTE_VECTOR_VECTOR 4
 
// might want to be larger (?)
#define ID_MAX_LINKED_LIST_SIZE 64

#define ID_EXTRA_ENCRYPT (1 << 0)
#define ID_EXTRA_COMPRESS (1 << 1)

#define ID_PREAMBLE_SIZE (sizeof(extra_t_)+sizeof(id_t_)+sizeof(mod_inc_t_))

#define ID_DATA_RULE_UNDEF 0
	
/*
  The only variable that can reasonably change between different variables
  of one type is EXPORT
 */

/*
  set_lowest_global_flag_level can be called in the constructor with default
  values, and the called again to change it back
 */

/*
  Transport rules for IDs, on a whitelist basis

  Wildcards can only be in the minor component, and are 255 (unsigned total
  bit flip on an unsigned 8-bit)
  Intermediary is any combination of:
  NET_INTERFACE_INTERMEDIARY_UNDEFINED (not transporting over network)
  NET_INTERFACE_INTERMEDIARY_NONE (clearnet)
  NET_INTERFACE_INTERMEDIARY_TOR
  NET_INTERFACE_INTERMEDIARY_I2P
 */

typedef std::pair<uint8_t, uint8_t> data_id_transport_rules_tier_t;

struct data_id_transport_rules_t{
public:
	std::vector<std::pair<uint8_t, uint8_t> > tier;
	std::vector<uint8_t> intermediary;

	data_id_transport_rules_t(
		std::vector<std::pair<uint8_t, uint8_t> > tier_,
		std::vector<uint8_t> intermediary_){
		tier = tier_;
		intermediary = intermediary_;
	}
	bool operator==(const data_id_transport_rules_t &rhs){
		return tier == rhs.tier &&
		intermediary == rhs.intermediary;
	}
	bool operator!=(const data_id_transport_rules_t &rhs){
		return !(*this == rhs);
	}
	data_id_transport_rules_t operator=(const data_id_transport_rules_t &other){
		if(this != &other){
			this->tier = other.tier;
			this->intermediary = other.intermediary;
		}
		return *this;
	}
};


// everywhere and everything
extern const data_id_transport_rules_t public_ruleset;
// can't network
extern const data_id_transport_rules_t private_ruleset;
// networkable, but can't save to disk
extern const data_id_transport_rules_t cache_ruleset;
// only exists in memory (exporting would probably make the type useless)
extern const data_id_transport_rules_t mem_ruleset;


// pointer added through add_data

struct data_id_ptr_t{
private:
	void *ptr = nullptr;
	std::vector<uint32_t> length;
	uint8_t flags = 0;
public:
	data_id_transport_rules_t transport_rules;
	data_id_ptr_t(void *ptr_,
		      std::vector<uint32_t> length_,
		      uint8_t flags_,
		      data_id_transport_rules_t transport_rules_) : transport_rules({}, {}){
		ptr = ptr_;
		length = length_;
		flags = flags_;
		transport_rules = transport_rules_;
	}
	~data_id_ptr_t(){}
	void *get_ptr();
	uint32_t get_length();
	std::vector<uint32_t> get_length_vector();
	GET_SET_S(flags, uint8_t);
	GET_SET_S(transport_rules, data_id_transport_rules_t);
};

/*
  TODO: rename add_data functions to more easily see how it is stored and used
  internally (raw pointer vs byte vector, namely)
 */

type_t_ get_id_type(id_t_ id); // tacky

#define ADD_DATA_1D_DEF(datatype, str, fl_ag) void add_data_##str(datatype *ptr_, uint32_t max_size_elem_, data_id_transport_rules_t transport_rules_ = public_ruleset){add_data(ptr_, {max_size_elem_}, fl_ag, transport_rules_);}
#define ADD_DATA_2D_DEF(datatype, str, fl_ag) void add_data_##str(datatype *ptr_, uint32_t max_size_elem_, uint32_t max_size_elem__, data_id_transport_rules_t transport_rules_ = public_ruleset){add_data(ptr_, {max_size_elem_, max_size_elem__}, fl_ag, transport_rules_);}

typedef std::pair<std::vector<id_t_>, std::vector<id_t_> > linked_list_data_t;


/*
  I'm getting rid of std::vector<id_t_> in transit in favor of a compression
  scheme which reduces it to std::vector<uint8_t>:
  1. One less thing that can break
  2. Most cases where IDs are transmitted are when their hashes match, this
     compression scheme uses that pretty nicely
  3. ID vector transmission never worked right in the first place	
 */

#define GET_SET_ID_VECTOR(data_to_set)					\
	std::vector<id_t_> get_##data_to_set(){return expand_id_set(data_to_set);} \
	void set_##data_to_set(std::vector<id_t_> tmp){data_to_set = compact_id_set(tmp, true);} \
	void set_##data_to_set(std::vector<uint8_t> tmp){data_to_set = tmp;} \
	void add_##data_to_set(id_t_ tmp){data_to_set = add_id_to_set(data_to_set, tmp);} \
	void del_##data_to_set(id_t_ tmp){data_to_set = del_id_from_set(data_to_set, tmp);} \
	uint64_t get_size_##data_to_set(){return size_of_id_set(data_to_set);} \


struct data_id_t{
private:
	// first 8 bytes UUID, last 32-byte SHA-256 hash
	id_t_ id = ID_BLANK_ID;
	void *ptr = nullptr;
	id_t_ encrypt_pub_key_id = ID_BLANK_ID;
	std::vector<data_id_ptr_t> data_vector;
	std::pair<std::vector<uint8_t>, std::vector<uint8_t> > linked_list;

	uint64_t last_access_timestamp_micro_s = 0;
	mod_inc_t_ modification_incrementor = 0;
	
	void init_list_all_data();
	void init_gen_id(type_t_);
	void init_type_cache();
	void add_data(void *ptr_, std::vector<uint32_t> size_, uint8_t flags_, data_id_transport_rules_t transport_rules_);
public:
	data_id_t(void *ptr_, uint8_t type);
	~data_id_t();
	// getters and setters
	// skip check for hash, only used internally
	id_t_ get_id();
	/*
	  SHOULD ONLY BE USED TO BOOTSTRAP
	 */
	void set_id(id_t_ id_);
	std::string get_type();
	uint8_t get_type_byte(){return get_id_type(id);}
	void *get_ptr();
	void mod_inc(){modification_incrementor++;}
	mod_inc_t_ get_mod_inc(){return modification_incrementor;}

	std::pair<std::vector<id_t_>, std::vector<id_t_> > get_linked_list();
	void set_linked_list(std::pair<std::vector<id_t_>, std::vector<id_t_> > tmp);
	
	// TODO: should enforce casting
	ADD_DATA_1D_DEF(std::vector<uint8_t>, one_byte_vector, ID_DATA_BYTE_VECTOR);
	ADD_DATA_2D_DEF(std::vector<std::vector<uint8_t> >, one_byte_vector_vector, ID_DATA_BYTE_VECTOR_VECTOR);
	ADD_DATA_1D_DEF(std::vector<uint64_t>, eight_byte_vector, ID_DATA_EIGHT_BYTE_VECTOR);
	ADD_DATA_1D_DEF(id_t_, id, ID_DATA_ID);
	ADD_DATA_1D_DEF(void, raw, 0);
	// export and import data
	// default on export is unencrypted and uncompressed, but is compressed
	// and encrypted when it is loaded into the cache (so always, currently,
	// but just not handled in this function)
	std::vector<uint8_t> export_data(uint8_t extra);
	void import_data(std::vector<uint8_t> data);
	uint64_t get_last_access_timestamp_micro_s(){return last_access_timestamp_micro_s;}

	// Transporting rules
	void set_most_liberal_rules(
		std::pair<uint8_t, uint8_t> tier_rule,
		uint8_t intermediary_rule);
	void set_most_liberal_rules(
		data_id_transport_rules_t rules);

	std::vector<id_t_> all_ids();
};

typedef uint16_t transport_i_t;
typedef uint32_t transport_size_t;

// namespace id_transport{
// 	std::vector<uint8_t> get_entry(std::vector<uint8_t> data, transport_i_t trans_i);
// 	std::vector<uint8_t> set_entry(std::vector<uint8_t> entry, transport_i_t trans);
// };

extern std::array<uint8_t, 32> get_id_hash(id_t_ id);
extern void set_id_hash(id_t_ *id, std::array<uint8_t, 32> hash);
extern uint64_t get_id_uuid(id_t_ id);
extern void set_id_uuid(id_t_ *id, uint64_t uuid);
extern type_t_ get_id_type(id_t_ id);
extern void set_id_type(id_t_ *id, type_t_ type);

/*
  Currently unused type byte

  I want to move away from the 32-byte type ID (only really used for
  ease of debugging and creating new types), but since the speed of
  types created will be going down from here on out, I can spend the
  extra time to add them here

  This also allows me to add a type byte to the ID for faster sanity
  checking (assuming that is followed)
 */

#define TYPE_IR_REMOTE_T				1
#define TYPE_ENCRYPT_PRIV_KEY_T				2
#define TYPE_ENCRYPT_PUB_KEY_T				3
#define TYPE_CONSOLE_T					4
#define TYPE_WALLET_SET_T				5
#define TYPE_NET_PROTO_SOCKET_T				6
#define TYPE_NET_PROTO_PEER_T				7
#define TYPE_NET_PROTO_CON_REQ_T			8
#define TYPE_NET_PROTO_LINKED_LIST_REQUEST_T		9
#define TYPE_NET_PROTO_ID_REQUEST_T		      	10
#define TYPE_NET_PROTO_TYPE_REQUEST_T			11
#define TYPE_NET_SOCKET_T				12
#define TYPE_NET_PROXY_T				13
#define TYPE_TV_CHANNEL_T				14
#define TYPE_TV_WINDOW_T				15
#define TYPE_TV_DEV_AUDIO_T				16
#define TYPE_TV_DEV_VIDEO_T				17
#define TYPE_TV_FRAME_AUDIO_T				18
#define TYPE_TV_FRAME_VIDEO_T				19
#define TYPE_TV_FRAME_CAPTION_T				20
#define TYPE_INPUT_DEV_STANDARD_T			21
#define TYPE_MATH_NUMBER_SET_T				22
#define TYPE_TV_ITEM_T					23
#define TYPE_NET_INTERFACE_RADIO_ADDRESS_T		24
#define TYPE_NET_INTERFACE_IP_ADDRESS_T			25
#define TYPE_NET_INTERFACE_INTERMEDIARY_T		26
#define TYPE_NET_INTERFACE_HARDWARE_DEV_T		27
#define TYPE_NET_INTERFACE_SOFTWARE_DEV_T		28
#define TYPE_ID_TIER_STATE_T 				29
#define TYPE_TV_SINK_STATE_T				30
#define TYPE_TV_SINK_METADATA_STATE_T			31
#define TYPE_NET_HTTP_T					32
#define TYPE_NET_HTTP_FILE_T				33
#define TYPE_TV_FRAME_NUMERICAL_T			34
#define TYPE_NET_HTTP_FILE_DRIVER_STATE_T		35
#define TYPE_ID_REQUEST_REQUEST_T			36
#define TYPE_ID_REQUEST_RESPONSE_T			37
#define TYPE_ID_TIER_NETWORK_SIMPLE_REQUEST_T		38
#define TYPE_ID_TIER_NETWORK_SIMPLE_RESPONSE_T		39
#define TYPE_ID_TIER_NETWORK_CACHE_T			40

#define TYPE_COUNT 41

#define ID_MAKE_TMP(x)					\
	if(true){					\
		data_id_t *tmp = PTR_ID(x, );		\
		if(tmp != nullptr){			\
			tmp->set_most_liberal_rules(	\
				private_ruleset);	\
		}					\
	}

#define ID_DATA_MAKE_TMP(tmp)			\
	tmp.set_most_liberal_rules(		\
		private_ruleset);		\
			
std::string id_breakdown(id_t_ id_);

#define IS_OWNER(id) (id == get_id_hash(production_priv_key_id)

#endif
#include "../main.h"
#include "../util.h"
#include "tier/id_tier.h"
#include "set/id_set.h"
#include "../net/interface/net_interface_intermediary.h"
