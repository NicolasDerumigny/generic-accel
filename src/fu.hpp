#include "ap_int.h"
#include "config.hpp"

void fu_addmul_axis (
		op_t op,
		half st, half ld0, half ld1,
		//half &loop_carried_val,
		int i, int j, int k,
		half &a, half &b, half &c);

#if defined(DIV) && defined(SQRT)
void fu_divsqrt (
		op_t op,
		half &st, half ld0, half ld1,
		int i, int j, int k);
#endif