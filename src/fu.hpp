#include "ap_int.h"
#include "config.hpp"

void fu_addmul_axis (
		op_t op,
		dtype_t st, dtype_t ld0, dtype_t ld1,
#		ifdef BLAS1
		dtype_t loop_carried_val,
#		endif
		int i, int j, int k,
#		ifdef BLAS1
		int red_idx, int lat_step,
#		endif
		dtype_t &a, dtype_t &b, dtype_t &c);

#if defined(DIV) && defined(SQRT)
void fu_divsqrt (
		op_t op,
		dtype_t &st, dtype_t ld0, dtype_t ld1,
		int i, int j, int k);
#endif
