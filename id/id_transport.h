#ifndef ID_TRANSPORT_H
#define ID_TRANSPORT_H
/*
  Any helper functions for importing and exporting are defined
  here.

  All functions convert their payload to NBO
 */

#include "id.h"
#include "../util.h"

#define EXPORT_STATIC(x_, y_) export_static_size_payload(&x_, reinterpret_cast<uint8_t*>(&y_), sizeof(y_))
#define IMPORT_STATIC(x_, y_) import_static_size_payload(&x_, reinterpret_cast<uint8_t*>(&y_), sizeof(y_))

// Size of the payload is static (directly from a variable), no size
// information is exported
extern void export_static_size_payload(
	std::vector<uint8_t> *pusher,
	uint8_t *data,
	uint64_t size);
extern void import_static_size_payload(
	std::vector<uint8_t> *puller,
	uint8_t *data,
	uint64_t size);

// Size can fit in 8-bits, so we just designate one byte for the size
// This is only used for
extern void export_8bit_size_payload(
	std::vector<uint8_t> *pusher,
	std::vector<uint8_t> payload);

extern std::vector<uint8_t> import_8bit_size_payload(
	std::vector<uint8_t> *puller);

// These are primarially used for transporting dynamic sized vectors,
// but are also used for positions in nested vectors

extern std::vector<uint8_t> export_gen_dynamic_size(
	uint64_t size);
	
extern uint64_t import_gen_dynamic_size(
	std::vector<uint8_t> *puller);

// The first variable is the length of the size in bytes, and the second
// variable is the size of the payload
extern void export_dynamic_size_payload(
	std::vector<uint8_t> *pusher,
	std::vector<uint8_t> payload);

extern std::vector<uint8_t> import_dynamic_size_payload(
	std::vector<uint8_t> *puller);


// All following functions rely on the previous ones, they don't read or
// write directly to the pusher/puller vector
extern void stringify_rules(
	std::vector<uint8_t> *pusher, 
	data_id_transport_rules_t rules);

extern data_id_transport_rules_t unstringify_rules(
	std::vector<uint8_t> *puller);

// The following functions take in data_id_ptr_t's and a requested size
// and return a valid pointer that can handle size bytes

extern void export_ptr_from_data_id_ptr(
	std::vector<uint8_t> *pusher,
	data_id_ptr_t *data_id_ptr,
	transport_i_t);

extern void import_ptr_to_data_id_ptr(
	std::vector<uint8_t> *puller,
	std::vector<data_id_ptr_t> *data_id_ptr);

// only extern for strip_to_transportable
extern void nether_add_at_pos(
	std::vector<std::vector<uint8_t> > *nether,
	std::vector<uint8_t> data,
	uint64_t pos);
#endif
