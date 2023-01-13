#include "ap_int.h"
#include "config.hpp"

constexpr float CUTOFF = 0.1f;

void fu_addmul_axis (
		op_t op,
		half st, half ld0, half ld1,
#		ifdef BLAS1
		half loop_carried_val,
#		endif
		int i, int j, int k,
#		ifdef BLAS1
		int red_idx, int lat_step,
#		endif
		half &a, half &b, half &c);

#if defined(DIV) && defined(SQRT)
void fu_divsqrt (
		op_t op,
		half &st, half ld0, half ld1,
		int i, int j, int k);
#endif
