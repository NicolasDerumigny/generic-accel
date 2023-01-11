#include "lbg.hpp"

#define max(a, b) ((a>b)?a:b)

// loop_bound_generator
int lbg (macro_op_t ops[NB_FU]) {
# 	pragma HLS inline
	int ret = 0;
	for (int i=0; i<NB_FU; i++) {
		switch (ops[i].opcode) {
			case op::noop: {
				break;
			}

			case op::mulmm: {
				ret = max(ret, N*N*N);
				break;
			}

			case op::mulmv:
			case op::mulsm:
			case op::trm:
			case op::addm:
			case op::subm:
#			ifdef SUBCMV
			case op::subcmv:
#			endif
#			ifdef PMUL
			case op::pmulm:
#			endif
			case op::oprodv:
#			ifdef ABS
			case op::absm:
#			endif
#			ifdef ACCSUMV
			case op::accsumcm:
#			endif
#			ifdef DIV
			case op::divms:
			case op::divcmv:
#			endif
#			ifdef SETM
			case op::set0m:
			case op::setidm:
			case op::setd1:
#			endif
			{
				ret = max(ret, N*N);
				break;
			}

			case op::mulsv:
			case op::addv:
			case op::subv:
#			ifdef PMUL
			case op::pmulv:
#			endif
#			ifdef ABS
			case op::absv:
#			endif
#			ifdef SQRT
			case op::sqrtv:
#			endif
#			ifdef CUTMINV
			case op::cutminv:
#			endif
#			ifdef DIV
			case op::divvs:
#			endif
#			ifdef BLAS1
			case op::copyv:
#			endif
			{
				ret = max(ret, N);
				break;
			}

			case op::muls:
			case op::adds:
			case op::subs:
#			ifdef ABS
			case op::abss:
#			endif
#			ifdef SQRT
			case op::sqrts:
#			endif
			{
				ret = max(ret, 1);
				break;
			}

#			ifdef TRGL
			case op::multrmm: {
				ret = max(ret, N*N*(N+1)/2);
				break;
			}
			case op::multrmv:
			case op::multrsm:
			case op::addtrm: {
				ret = max(ret, N*(N+1)/2);
				break;
			}
#			endif

#			ifdef BLAS1
			case op::dotv:
			case op::sasum:
			//case op::isamax:
			{
				ret = max(ret, FU_LATENCY*N);
				break;
			}
#			endif
		}
	}
	return ret;
}
