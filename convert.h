#ifndef CONVERT_H
#define CONVERT_H
#include "vector"
#include "string"
#include "array"
// TODO: implement some builtin functions for VC++ and other compilers
// before opting for the slowest method

// NBO: network byte order

#ifdef __ORDER_LITTLE_ENDIAN__

#ifdef __GNUC__

#define NBO_64 __builtin_bswap64
#define NBO_32 __builtin_bswap32
#define NBO_16 __builtin_bswap16
// seriously doubt that a native function would
// be faster than this, even for 16-bit (?)
#define NBO_8(a) (NBO_16((uint16_t)a) >> 8)
#define NBO_TO_NATIVE_8 __builtin_bswap8
#define NBO_TO_NATIVE_16 __builtin_bswap16
#define NBO_TO_NATIVE_32 __builtin_bswap32
#define NBO_TO_NATIVE_64 __builtin_bswap64

#else

#error "no converting functions for NBO"

#endif

#elif __ORDER_BIG_ENDIAN__
#define NBO_64(a) (a)
#define NBO_32(a) (a)
#define NBO_16(a) (a)
#define NBO_8(a) (a)
#define NBO_TO_NATIVE_8(a) (a)
#define NBO_TO_NATIVE_16(a) (a)
#define NBO_TO_NATIVE_32(a) (a)
#define NBO_TO_NATIVE_64(a) (a)
#endif

// mostly for time, but add the other ones
#define MILLI_PREFIX (0.001)
#define MICRO_PREFIX (0.000001)

#ifndef __ORDER_LITTLE_ENDIAN__
#error "convert CROP_LSB functions to big-endian machines"
#endif

#define CROP_LSB(data, bits_out) (data & ~((~0) << bits_out))

namespace convert{
	namespace nbo{
		std::vector<uint8_t> to(std::vector<uint8_t>);
		std::vector<uint8_t> to(std::string);
		std::vector<uint8_t> from(std::vector<uint8_t>);
		std::vector<uint8_t> from(std::string);
	}
	namespace array{
		namespace type{
			std::array<uint8_t, 32> to(std::string);
			std::string from(std::array<uint8_t, 32>);
		}
	}
};

#endif
