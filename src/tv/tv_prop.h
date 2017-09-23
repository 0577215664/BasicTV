#ifndef TV_PROP_H
#define TV_PROP_H
#include "../main.h"
#include "../id/id.h"

#define TV_PROP_FORMAT_ONLY (1 << 0)

#define TV_PROP_ENCODER_VAR(x) (x >> 32)
#define TV_PROP_ENCODER_VAL(x) (x & 0x00000000FFFFFFFF)

struct tv_prop_t{
private:
	uint32_t bit_rate = 0;
	uint8_t format = 0;
	uint8_t flags = 0;
	uint32_t snippet_duration_micro_s = 0;
	
	// Interpret as two 32-bit values, seems to be sane
	// NOTE: direct referencing to encoder CTLs are bad practice, since
	// they can change on theri own. A conversion system needs to be in
	// place to have a somewhat nice mapping between internally defined
	// and used CTLs with their library counterparts (lossy/lossless,
	// bitrate, FEC, etc.)
	std::vector<uint64_t> encoder_flags;

	data_id_t *id = nullptr;
public:
	tv_prop_t();
	~tv_prop_t();
	void list_prop_data(data_id_t *id_);
	bool operator==(const tv_prop_t &rhs){
		return bit_rate == rhs.bit_rate &&
		format == rhs.format &&
		snippet_duration_micro_s == rhs.snippet_duration_micro_s &&
		encoder_flags == rhs.encoder_flags;
	}
	bool operator!=(const tv_prop_t &rhs){
		return !(*this == rhs);
	}
	GET_SET_V(bit_rate, uint32_t);
	GET_SET_V(format, uint8_t);
	GET_SET_V(flags, uint8_t);
	GET_SET_V(snippet_duration_micro_s, uint32_t);
	GET_SET_V(encoder_flags, std::vector<uint64_t>);
};

#endif
