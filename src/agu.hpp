#include "config.hpp"

void agu (
	op_t op,
	int i, int j, int k,
#	ifdef BLAS1
	int red_idx, int lat_step,
#	endif
	int &ld0_addr,
	int &ld1_addr,
	int &st_addr);
