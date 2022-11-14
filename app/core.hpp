#ifndef CORE_HPP
#define CORE_HPP
#include "stdint.h"

constexpr int N = 64;

constexpr int NB_FU_GEN = 1;
constexpr int NB_FU_MUL = 0;
constexpr int NB_FU_ADD = 1;

constexpr int NB_FU = NB_FU_GEN + NB_FU_ADD + NB_FU_MUL;
constexpr int REG_SIZ = 3*NB_FU; // 3 input per FU
constexpr int MAX_PGM_SIZE = 16;

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
	noop = 0,
	mulmm   = 1,
	mulmv   = 2,
	mulsm   = 3,
	mulsv   = 4,
	muls    = 5,
	trm     = 6,
	addm    = 7,
	addv    = 8,
	adds    = 9,
	subm    = 10,
	subv    = 11,
	subs    = 12,
	pmulm   = 13,  // Point-wise mul: hadamard product
	pmulv   = 14,  // Point-wise mul
	oprodv  = 15,
	absm    = 16,
	absv    = 17,
	abss    = 18,
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
