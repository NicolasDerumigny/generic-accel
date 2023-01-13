#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "stdint.h"

constexpr int N = 64;

constexpr int NB_FU_ADDMUL = 3;
constexpr int NB_FU_DIVSQRT = 1;
constexpr int NB_FU = NB_FU_ADDMUL + NB_FU_DIVSQRT;

constexpr int REG_SIZ = 27; // Upper bound for correlation is 5 per pb + 2 cst
constexpr int MAX_PGM_SIZE = 64; // to be large

#define SUBCMV
#define PMUL
#define ABS
#define SQRT
#define ACCSUMV
#define CUTMINV
#define DIV
#define SETM
//#define TRGL
//#define BLAS1

// Internal constants
constexpr int FU_LATENCY = 13;
constexpr int NO_RW = -1;
constexpr int RED_REG = -2;

#define CUa(id) cu ## id ##_a
#define CUb(id) cu ## id ##_b
#define CUc(id) cu ## id ##_c
#define CUres(id) cu ## id ##_res

#define ONECU(id) \
		gu_t &CUa(id), gu_t &CUb(id), gu_t &CUc(id), gu_t &CUres(id)

#define ONECU_NAME(id) \
		CUa(id), CUb(id), CUc(id), CUres(id)

#define CU_INTERFACE \
		ONECU(0), \
		ONECU(1), \
		ONECU(2)

#define CU_INTERFACE_NAMES \
		ONECU_NAME(0), \
		ONECU_NAME(1), \
		ONECU_NAME(2)


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
	noop,
	mulmm,
	mulmv,
	mulsm,
	mulsv,
	muls,
	trm, // Transpose matrix
	addm,
	addv,
	adds,
	subm,
#	ifdef BLAS1
	copyv,
	dotv,
	sasum,
	//isamax,
#	endif
# 	ifdef SUBCMV
	subcmv, // Point-wise substraction with column-wise (line-independant) value
#	endif
	subv,
	subs,
#   ifdef PMUL
	pmulm,  // Point-wise mul: hadamard product
	pmulv,  // Point-wise mul
#	endif
	oprodv,
#   ifdef ABS
	absm,
	absv,
	abss,
# 	endif
#   ifdef SQRT
	sqrtv,
	sqrts ,
#	endif
#	ifdef ACCSUMV
	accsumcm,  // Accumulation of matrix in a vector by column-wise
				   // (line-indepedant) sum of all the elements
#	endif
# 	ifdef CUTMINV
	cutminv,  // Pointwise selection: `if coef < threshold 1 else coef`
#   endif
#	ifdef DIV
	divms,  // Pointwise division of matrices
	divvs,  // Pointwise division of vectors
	divcmv,  // Point-wise division with column-wise (line-independant) value
#	endif
#	ifdef SETM
	set0m,
	setidm,
	setd1,
#	endif
#	ifdef TRGL
	multrmm,
	multrmv,
	multrsm,
	addtrm,
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
