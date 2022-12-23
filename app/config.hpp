#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "stdint.h"

constexpr int N = 64;

constexpr int NB_FU = 2;

constexpr int REG_SIZ = 27; // Upper bound for correlation is 5 per pb + 2 cst
constexpr int MAX_PGM_SIZE = 64; // to be large

// #define SUBCMV
// #define PMUL
// #define ABS
// #define SQRT
// #define ACCSUMV
// #define CUTMIV
// #define DIV
// #define SETM
#define TRGL

#define ONECU(id) \
		gu_t &cu ## id ## _a, gu_t &cu ## id ##  _b, gu_t &cu ## id ## _c,	gu_t &cu ## id ##  _res

#define ONECU_NAME(id) \
		cu ## id ## _a, cu ## id ##  _b, cu ## id ## _c, cu ## id ##  _res

#define CU_INTERFACE \
		ONECU(0), \
		ONECU(1)

#define CU_INTERFACE_NAMES \
		ONECU_NAME(0), \
		ONECU_NAME(1)

/** Inplace operations are supported, except on:
 * - mulmm
 * - mulmv
 * - trm
 * - oprodv
 *
 * Concurrent writing on the same register by two
 * FU also results in undefined behaviour
 */
enum op : uint8_t {
	noop     = 0,
	mulmm    = 1,
	mulmv    = 2,
	mulsm    = 3,
	mulsv    = 4,
	muls     = 5,
	trm      = 6, // Transpose matrix
	addm     = 7,
	addv     = 8,
	adds     = 9,
	subm     = 10,
# 	ifdef SUBCMV
	subcmv   = 11, // Point-wise substraction with column-wise (line-independant) value
#	endif
	subv     = 12,
	subs     = 13,
#   ifdef PMUL
	pmulm    = 14,  // Point-wise mul: hadamard product
	pmulv    = 15,  // Point-wise mul
#	endif
	oprodv   = 16,
#   ifdef ABS
	absm     = 17,
	absv     = 18,
	abss     = 19,
# 	endif
#   ifdef SQRT
	sqrtv    = 20,
	sqrts    = 21,
#	endif
#	ifdef ACCSUMV
	accsumcm = 22,  // Accumulation of matrix in a vector by column-wise
				   // (line-indepedant) sum of all the elements
#	endif
# 	ifdef CUTMIV
	cutminv  = 23,  // Pointwise selection: `if coef < threshold 1 else coef`
#   endif
#	ifdef DIV
	divms    = 24,  // Pointwise division of matrices
	divvs    = 25,  // Pointwise division of vectors
	divcmv   = 26,  // Point-wise division with column-wise (line-independant) value
#	endif
#	ifdef SETM
	set0m    = 27,
	setidm   = 28,
	setd1    = 29,
#	endif
#	ifdef TRGL
	multrmm  = 30,
	multrmv  = 31,
	multrsm  = 32,
	addtrm   = 33,
#	endif
};

using op_t = enum op;

#ifdef __SYNTHESIS__
# include "ap_int.h"

struct s_macro_op_t {
	op_t opcode;
	ap_uint<5> r_dst;
	ap_uint<5> r0;
	ap_uint<5> r1;
};
#else
struct s_macro_op_t {
	op_t opcode;
	uint8_t r_dst;
	uint8_t r0;
	uint8_t r1;
};
#endif

using macro_op_t = struct s_macro_op_t;

static_assert(sizeof(macro_op_t) == sizeof(uint32_t));
#endif // CONFIG_HPP
