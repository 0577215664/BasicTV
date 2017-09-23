#ifndef MATH_NUMBER_UNITS_H
#define MATH_NUMBER_UNITS_H

// base SI unit or complex SI unit, doesn't matter
#define MATH_NUMBER_USE_SI 1
// use as a GPS coordinate (major and minor change value)
#define MATH_NUMBER_USE_COORD 2
// no formal units, still a number tho
#define MATH_NUMBER_USE_NONE 3
// categorical information (don't use me in numerical analysis)
#define MATH_NUMBER_USE_CAT 4


// SI base units, fit nicely into 3-bits

#define MATH_NUMBER_BASE_BLANK 0
#define MATH_NUMBER_BASE_METER 1
#define MATH_NUMBER_BASE_KILOGRAM 2
#define MATH_NUMBER_BASE_SECOND 3
#define MATH_NUMBER_BASE_AMPERE 4
#define MATH_NUMBER_BASE_KELVIN 5
#define MATH_NUMBER_BASE_MOLE 6
#define MATH_NUMBER_BASE_CANDELA 7
#define MATH_NUMBER_BASE_BYTE 8

#define MATH_NUMBER_PREFIX_MICRO 1

// alternative/imperial units IDs
// only distances for now, will finish later
#define MATH_NUMBER_ALT_INCH 1
#define MATH_NUMBER_ALT_FOOT 2
#define MATH_NUMBER_ALT_YARD 3
#define MATH_NUMBER_ALT_MILE 4
//#define MATH_NUMBER_ALT_KNOT 5

#define MATH_NUMBER_DIM_UNDEF 0
#define MATH_NUMBER_DIM_NUM 1
#define MATH_NUMBER_DIM_CAT 2

#define MATH_NUMBER_OPERATOR_BLANK 0
#define MATH_NUMBER_OPERATOR_MULTIPLY 1
#define MATH_NUMBER_OPERATOR_DIVIDE 2
#define MATH_NUMBER_OPERATOR_EXPONENT 3
#define MATH_NUMBER_OPERATOR_NEGATE 4


#define UNIT(use, base, prefix) (use | (base << 8) | (prefix << 16))

#endif
