#ifndef CORE_HPP
#define CORE_HPP
#include "stdint.h"

constexpr int N = 64;

constexpr int NB_FU_GEN = 1;
constexpr int NB_FU_MUL = 0;
constexpr int NB_FU_ADD = 1;

constexpr int NB_FU = NB_FU_GEN + NB_FU_ADD + NB_FU_MUL;
//constexpr int REG_SIZ = 3*NB_FU; // 3 input per FU
constexpr int REG_SIZ = 16; // Upper bound for correlation
constexpr int MAX_PGM_SIZE = 128;

constexpr float CUTOFF = 0.1f;

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
	trm      = 6,
	addm     = 7,
	addv     = 8,
	adds     = 9,
	subm     = 10,
	subcmv   = 11, // Point-wise substraction with column-wise (line-independant) value
	subv     = 12,
	subs     = 13,
	pmulm    = 14,  // Point-wise mul: hadamard product
	pmulv    = 15,  // Point-wise mul
	oprodv   = 16,
	absm     = 17,
	absv     = 18,
	abss     = 19,
	sqrtv    = 20,
	sqrts    = 21,
	accsumcm = 22,  // Accumulation of matrix in a vector by column-wise
				   // (line-indepedant) sum of all the elements
	cutminv  = 23,  // Pointwise selection: `if coef < threshold 1 else coef`
	divms    = 24,  // Pointwise division of matrices
	divvs    = 25,  // Pointwise division of vectors
	divcmv   = 26,  // Point-wise division with column-wise (line-independant) value
	set0m    = 27,
	setidm   = 28,
	setd1    = 29,
};

using op_t = enum op;

struct s_macro_op_t {
	op_t opcode;
	uint8_t r_dst;
	uint8_t r0;
	uint8_t r1;
};
using macro_op_t = struct s_macro_op_t;

static_assert(sizeof(macro_op_t) == sizeof(uint32_t));

#endif // CORE_HPP
