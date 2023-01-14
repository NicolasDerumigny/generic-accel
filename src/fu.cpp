#include "fu.hpp"
#include "hls_math.h"

#define max(a, b) ((a>b)?a:b)

inline half do_minus(half value) {
#	pragma HLS inline
	ap_uint<16> minus_in = *(ap_uint<16>*) &value;
	minus_in = ( ((~minus_in) & 0x8000) | (minus_in & 0x7FFF));
	return *(half*) &minus_in;
}

inline double do_minus_double(double value) {
       ap_uint<64> minus_in = *(ap_uint<64>*) &value;
       minus_in = ( ((~minus_in) & 0x8000000000000000) | (minus_in & 0x7FFF000000000000));
       return *(double*) &minus_in;
}

#if defined(ABS) || defined(BLAS1)
inline half do_abs(half value) {
	ap_uint<16> abs = *(ap_uint<16>*) &value;
	abs = abs & 0x7FFF;
	return *(half*) &abs;
}

inline double do_abs_double(double value) {
       ap_uint<64> abs = *(ap_uint<64>*) &value;
       abs = abs & 0x7FFF000000000000;
       return *(double*) &abs;
}
#endif

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
		dtype_t &a, dtype_t &b, dtype_t &c) {
#	pragma HLS inline off
# 	pragma HLS pipeline II=1
	a = 0;
	b = 1;
	c = 0;

	switch (op) {
		default: {
			break;
		}

#		ifdef BLAS1
		case op::copyv: {
			a = ld0;
			break;
		}
		case op::sasum: {
			if (red_idx == 0 and lat_step == 0) {
				a = do_abs(ld0);
			}
			else {
				a = do_abs(ld0);
				c = loop_carried_val;
			}
			break;
		}
		/*case op::isamax: {
			if (red_idx == 0 and lat_step == 0) {
				a = ld0;
			}
			else {
				a = max(ld0, loop_carried_val);
			}
			break;
		}*/

		case op::dotv: {
			a = ld0;
			b = ld1;
			if (not (red_idx == 0 and lat_step == 0)) {
				c = loop_carried_val;
			}
			break;
		}
#		endif

		case op::mulmm:
		case op::mulmv:
#		ifdef TRGL
		case op::multrmm:
		case op::multrsm:
		case op::multrmv:
#		endif
		{
			a = ld0;
			b = ld1;
			if (j==0)
				c = 0;
			else
				c = st;
			break;
		}

		case op::trm: {
			a = ld0;
			break;
		}

		case op::addm:
		case op::addv:
		case op::adds:
#		ifdef TRGL
		case op::addtrm:
#		endif
		{
			a = ld0;
			c = ld1;
			break;
		}

		case op::mulsm:
		case op::mulsv:
		case op::muls:
#		ifdef PMUL
		case op::pmulm:
		case op::pmulv:
#		endif
		case op::oprodv: {
			a = ld0;
			b = ld1;
			break;
		}

		case op::subm:
		case op::subv:
		case op::subs:
#		ifdef SUBCMV
		case op::subcmv:
#		endif
		{
			a = ld0;
			c = do_minus(ld1);
			break;
		}

#		ifdef ACCSUMV
		case op::accsumcm: {
			if (j==0)
				a = 0;
			else
				a = st;
			c = ld0;
			break;
		}
#		endif

#		ifdef CUTMINV
		case op::cutminv: {
			a = (ld0<=CUTOFF)?(dtype_t)1.0:ld0;
			break;
		}
#		endif

#		ifdef SETM
		case op::set0m: {
			a = 0;
			break;
		}

		case op::setidm: {
			a = (i==j);
			break;
		}

		case op::setd1:{
			a = (j==k)?(dtype_t)1.0f:ld0;
			break;
		}
#		endif

		case op::noop: {
			break;
		}
	}
}

#if defined(DIV) && defined(SQRT)
void fu_divsqrt (
		op_t op,
		dtype_t &st, dtype_t ld0, dtype_t ld1,
		int i, int j, int k) {
#	pragma HLS inline off
# 	pragma HLS pipeline II=1
# 	pragma HLS allocation operation instances=hadd limit=1
# 	pragma HLS allocation operation instances=hmul limit=1
	dtype_t ld_st = st;
	dtype_t add_op0, add_op1;

	switch (op) {
		default: {
			break;
		}

		case op::sqrtv:
		case op::sqrts: {
			st = hls::sqrt(ld0);
			break;
		}


		case op::divms:
		case op::divvs:
		case op::divcmv: {
			st = ld0/ld1;
			break;
		}

#		ifdef SETM
		case op::set0m: {
			st = 0;
			break;
		}

		case op::setidm: {
			st = (i==j);
			break;
		}

		case op::setd1:{
			st = (j==k)?(dtype_t)1.0f:ld0;
			break;
		}

		case op::noop: {
			break;
		}
#		endif
	}
}
#endif
