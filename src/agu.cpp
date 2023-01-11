#include "agu.hpp"
static inline int access (int i, int j) {
#	pragma HLS inline
	return i*N + j;
}

void agu (
		op_t op,
		int i, int j, int k,
#		ifdef BLAS1
		int red_idx, int lat_step,
#		endif
		int &ld0_addr,
		int &ld1_addr,
		int &st_addr
) {
# 	pragma HLS inline
	ld0_addr = NO_RW;
	ld1_addr = NO_RW;
	st_addr = NO_RW;

	switch (op) {
		case op::mulmm: {
			ld0_addr = access(i,j);
			ld1_addr = access(j,k);
			st_addr = access(i,k);
			break;
		}

		case op::mulmv: {
			if (i==0) {
				ld0_addr = access(k,j);
				ld1_addr = access(0,j);
				st_addr = access(j,k);
			}
			break;
		}

		case op::mulsm: {
			if (i==0) {
				ld0_addr = access(0,0);
				ld1_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}

#		ifdef TRGL
		case op::multrmm: {
			if (k>=j) {
				ld0_addr = access(i,j);
				ld1_addr = access(j,k);
				st_addr = access(i,k);
				break;
			}
		}

		case op::multrmv:{
			if (i==0 && k>=j) {
				ld0_addr = access(k,j);
				ld1_addr = access(0,j);
				st_addr = access(j,k);
			}
			break;
		}

		case op::addtrm: {
			if (i==0 and k>=j) {
				ld0_addr = access(j,k);
				ld1_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}

		case op::multrsm: {
			if (i==0 && k>=j) {
				ld0_addr = access(0,0);
				ld1_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}
#		endif

#		ifdef DIV
		case op::divms: {
			if (i==0) {
				ld0_addr = access(j,k);
				ld1_addr = access(0,0);
				st_addr = access(j,k);
			}
			break;
		}
#		endif

		case op::trm: {
			if (i==0) {
				ld0_addr = access(j,k);
				st_addr = access(k,j);
			}
			break;
		}

		case op::addm:
		case op::subm:
# 		ifdef PMUL
		case op::pmulm:
#		endif
		{
			if (i==0) {
				ld0_addr = access(j,k);
				ld1_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}

		case op::addv:
		case op::subv:
# 		ifdef PMUL
		case op::pmulv:
#		endif
		{
			if (i==0 and j==0) {
				ld0_addr = access(0,k);
				ld1_addr = access(0,k);
				st_addr = access(0,k);
			}
			break;
		}


		case op::mulsv: {
			if (i==0 and j==0) {
				ld0_addr = access(0,0);
				ld1_addr = access(0,k);
				st_addr = access(0,k);
			}
			break;
		}

#		ifdef DIV
		case op::divvs:{
			if (i==0 and j==0) {
				ld0_addr = access(0,k);
				ld1_addr = access(0,0);
				st_addr = access(0,k);
			}
			break;
		}
#		endif

		case op::muls:
		case op::adds:
		case op::subs: {
			if (i==0 && j==0 && k==0) {
				ld0_addr = access(0,0);
				ld1_addr = access(0,0);
				st_addr = access(0,0);
			}
			break;
		}

		case op::oprodv: {
			if (i==0) {
				ld0_addr = access(0,j);
				ld1_addr = access(0,k);
				st_addr = access(j,k);
			}
			break;
		}

#		ifdef SUBCMV
		case op::subcmv:
#		endif
#		ifdef DIV
		case op::divcmv:
#		endif
#		if defined(SUBCMV) || defined(DIV)
		{
			if (i==0) {
				ld0_addr = access(j,k);
				ld1_addr = access(0,k);
				st_addr = access(j,k);
			}
			break;
		}
#		endif

#		ifdef ACCSUMV
		case op::accsumcm: {
			if (i==0) {
				ld0_addr = access(j,k);
				st_addr = access(0,k);
			}
			break;
		}
#		endif

#		ifdef ABS
		case op::absm: {
			if (i==0) {
				ld0_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}
#		endif

#		ifdef SETM
		case op::set0m:
		case op::setidm: {
			if (i==0) {
				st_addr = access(j,k);
			}
			break;
		}
#		endif

#		ifdef ABS
		case op::absv:
#		endif
#		ifdef SQRT
		case op::sqrtv:
#		endif
#		ifdef CUTMINV
		case op::cutminv:
#		endif
#		ifdef BLAS1
		case op::copyv:
#		endif
#		if defined(ABS) || defined(SQRT) || defined(CUTMINV) || defined(BLAS1)
		{
			if (i==0 and j==0) {
				ld0_addr = access(0,k);
				st_addr = access(0,k);
			}
			break;
		}
#		endif

#		ifdef ABS
		case op::abss:
#		endif
#		ifdef SQRT
		case op::sqrts:
#		endif
#		if defined(ABS) || defined(SQRT)
		{
			if (i==0 and j==0 and k==0) {
				ld0_addr = access(0,0);
				st_addr = access(0,0);
			}
			break;
		}
#		endif

#		ifdef SETM
		case op::setd1: {
			if (i==0) {
				ld0_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}
#		endif

#		ifdef BLAS1
		case op::dotv: {
			if (lat_step == 0 and red_idx < N) {
				ld0_addr = access(0, red_idx);
				ld1_addr = access(0, red_idx);
				if (red_idx < N-1) {
					st_addr = RED_REG;
				} else {
					st_addr = access(0,0);
				}
			}
			break;
		}

		case op::sasum:
		/*case op::isamax:*/ {
			if (lat_step == 0 and red_idx < N) {
				ld0_addr = access(0, red_idx);
				if (red_idx < N-1) {
					st_addr = RED_REG;
				} else {
					st_addr = access(0,0);
				}
			}
			break;
		}
#		endif

		case op::noop: {
			break;
		}
	}
}
