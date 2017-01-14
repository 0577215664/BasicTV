#include "../main.h"
#ifndef ID_H
#define ID_H
#include "cstdint"
#include "fstream"
#include "array"
#include "vector"
#include "random"
#include "cstdlib"

#define ADD_DATA(x) (id.add_data(&x, sizeof(x)))
#define ADD_DATA_ARRAY(x, y, z) (id.add_data(&(x[0]), y*z))
#define ADD_DATA_NONET(x) (id.add_data(&x, sizeof(x), ID_DATA_NONET))
#define ADD_DATA_PTR(x) (id->add_data(&x, sizeof(x)))

/*
  id_t: ID and pointer system for the networking system
 */

#define TYPE_LENGTH 32 // standard maximum length for C++ types

/*
  (currently unused) ID system
  First 128-bits are a random number acting as a UUID
  The next 512 bits (rest of it) is the SHA-512 hash of the public key used to
  decrypt this. This is manually searched for, which will reveal the encryption
  scheme used and citing information.

  In order for the requested ID to match the received ID, the public key
  fingerprints have to equal.
  
  Legitimate proof of net_peer_t is taken care of in a TLS style manner (see
  net/proto/net_proto_peer.h for more documentation)
 */
//typedef std::array<uint8_t, 96> id_t_;
typedef uint64_t id_t_; // needs a snazzier name

#define ID_DATA_NOEXPORT (1 << 0)
#define ID_DATA_NOEXP ID_DATA_NOEXPORT
#define ID_DATA_NONET (1 << 1)
#define ID_DATA_ID (1 << 2)
#define ID_DATA_BYTE_VECTOR (1 << 3)
#define ID_DATA_ID_VECTOR (1 << 4)

#define ID_DATA_CACHE ID_DATA_NOEXPORT

// pointer added through add_data
struct data_id_ptr_t{
private:
	void *ptr = nullptr;
	uint32_t length = 0;
	uint8_t flags = 0;
public:
	data_id_ptr_t(void *ptr_,
		      uint32_t length_,
		      uint8_t flags_);
	~data_id_ptr_t();
	void *get_ptr();
	uint32_t get_length();
	uint8_t get_flags();
	void set_flags(uint8_t flags_){flags = flags_;}
};

struct data_id_t{
private:
	// half UUID, half RSA fingerprint (for verification)
	id_t_ id = 0;
       	std::array<uint8_t, TYPE_LENGTH> type = {{0}};
	void *ptr = nullptr;
	id_t_ rsa_cite_id = 0;
	std::vector<std::vector<uint8_t> > rsa_backlog;
	std::vector<data_id_ptr_t> data_vector;
	std::pair<id_t_, id_t_> linked_list = {0,0};
	std::vector<uint8_t> imported_data; // encrypted data if imported
	void init_list_all_data();
	void init_gen_id();
	void init_type_cache();
	// set at get_ptr(), used for selective exporting
	uint64_t last_access_timestamp_micro_s = 0;
public:
	data_id_t(void *ptr_, std::string type_);
	~data_id_t();
	// getters and setters
	uint64_t get_id();
	std::string get_type();
	void *get_ptr();
	void set_rsa_cite_id(uint64_t id_){rsa_cite_id = id_;} // probably should fix this soon
	void set_encrypt_cite_id(uint64_t id_){rsa_cite_id = id_;}
	uint64_t get_rsa_cite_id();
	uint64_t get_encrypt_cite_id(){return get_rsa_cite_id();}
	uint64_t get_data_index_size();
	uint64_t get_next_linked_list();
	uint64_t get_prev_linked_list();
	void set_next_linked_list(uint64_t data);
	void set_prev_linked_list(uint64_t data);
	// pointer list modififers
	/*
	  size of data is referring to the type size and the array size, whereas
	  the size of the ID is referring to just the array size, since the size
	  is assumed with the pointer type (8 bytes, but maybe more later?)
	 */
	void add_data(
		void *ptr_,
		uint32_t size_,
		uint64_t flags = 0);
	void add_data(
		std::vector<uint8_t> *ptr_,
		uint32_t size_,
		uint64_t flags_ = 0);
	void add_data(
		std::vector<id_t_> *ptr_,
		uint32_t size_,
		uint64_t flags = 0);
	// export and import data
	std::vector<uint8_t> export_data(uint8_t flags_);
	void import_data(std::vector<uint8_t> data);
	void rsa_decrypt_backlog();
	bool is_owner();
	std::vector<uint8_t> get_ptr_flags();
	void noexport_all_data();
	void nonet_all_data();
	uint64_t get_last_access_timestamp_micro_s(){return last_access_timestamp_micro_s;}
};
#endif
