#include "fu.hpp"
#include "hls_math.h"

inline half do_minus(half value) {
#	pragma HLS inline
	ap_uint<16> minus_in = *(ap_uint<16>*) &value;
	minus_in = ( ((~minus_in) & 0x8000) | (minus_in & 0x7FFF));
	return *(half*) &minus_in;
}

#ifdef ABS
inline half do_abs(half value) {
	ap_uint<16> abs = *(ap_uint<16>*) &value;
	abs = abs & 0x7FFF;
	return *(half*) &abs;
}
#endif

void fu_addmul_axis (
		op_t op,
		half st, half ld0, half ld1,
		int i, int j, int k,
		half &a, half &b, half &c) {
#	pragma HLS inline off
# 	pragma HLS pipeline II=1
	a = 0;
	b = 1;
	c = 0;

	switch (op) {
		default: {
			break;
		}

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
			a = (ld0<=CUTOFF)?(half)1.0:ld0;
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
			a = (j==k)?(half)1.0f:ld0;
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
		half &st, half ld0, half ld1,
		int i, int j, int k) {
#	pragma HLS inline off
# 	pragma HLS pipeline II=1
# 	pragma HLS allocation operation instances=hadd limit=1
# 	pragma HLS allocation operation instances=hmul limit=1
	half ld_st = st;
	half add_op0, add_op1;

	switch (op) {
		default: {
			break;
		}

		case op::sqrtv:
		case op::sqrts: {
			st = hls::half_sqrt(ld0);
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
			st = (j==k)?(half)1.0f:ld0;
			break;
		}

		case op::noop: {
			break;
		}
#		endif
	}
}
#endif

/*static void fu_addmul (
		op_t op,
		half &st, half ld0, half ld1,
		int i, int j, int k) {
#	pragma HLS inline off
# 	pragma HLS pipeline II=1
# 	pragma HLS allocation operation instances=hadd limit=1
# 	pragma HLS allocation operation instances=hmul limit=1
	half ld_st = st;
	half add_op0, add_op1;
	half mul_res;

	switch (op) {
		case op::mulmm:
		case op::mulmv:
		case op::mulsm:
		case op::mulsv:
		case op::muls:
		case op::pmulm:
		case op::pmulv:
		case op::oprodv: {
			mul_res = ld0*ld1;
			break;
		}

		default: {
			break;
		}
	}

	switch (op) {
		case op::mulmm:
		case op::mulmv: {
			add_op1 = mul_res;
			if (j==0)
				add_op0 = 0;
			else
				add_op0 = ld_st;
			break;
		}

		case op::trm: {
			st = ld0;
			break;
		}

		case op::addm:
		case op::addv:
		case op::adds: {
			add_op0 = ld0;
			add_op1 = ld1;
			break;
		}

		case op::mulsm:
		case op::mulsv:
		case op::muls:
		case op::pmulm:
		case op::pmulv:
		case op::oprodv: {
			st = mul_res;
			break;
		}

		case op::subm:
		case op::subv:
		case op::subs:
		case op::subcmv: {
			add_op1 = do_minus(ld1);
			add_op0 = ld0;
			break;
		}

		case op::accsumcm: {
			add_op1 = ld0;
			if (j==0)
				add_op0 = 0;
			else
				add_op0 = ld_st;
			break;
		}

		case op::cutminv: {
			st = (ld0<=CUTOFF)?(half)1.0:ld0;
			break;
		}

		case op::set0m: {
			st = 0;
			break;
		}

		case op::setidm: {
			st = (i==j);
			break;
		}

		case op::setd1:{
			st = (j==k)?(half)1.0f:ld0;
			break;
		}

		case op::noop: {
			break;
		}
	}
	switch (op) {
		case op::addm:
		case op::addv:
		case op::adds:
		case op::subm:
		case op::subcmv:
		case op::subv:
		case op::subs:
		case op::mulmm:
		case op::mulmv:
		case op::accsumcm: {
			st = add_op0 + add_op1;
			break;
		}

		default: {
			break;
		}
	}
}



static void fu_mul (
		op_t op,
		half &st, half ld0, half ld1,
		int i, int j, int k) {
#	pragma HLS inline off
# 	pragma HLS pipeline II=1
# 	pragma HLS allocation operation instances=hmul limit=1
	half ld_st = st;
	half add_op0, add_op1;
	half mul_res;

	switch (op) {
		case op::mulsm:
		case op::mulsv:
		case op::muls:
		case op::pmulm:
		case op::pmulv:
		case op::oprodv: {
			mul_res = ld0*ld1;
			break;
		}

		default: {
			break;
		}
	}

	switch (op) {
		case op::trm: {
			st = ld0;
			break;
		}

		case op::mulsm:
		case op::mulsv:
		case op::muls:
		case op::pmulm:
		case op::pmulv:
		case op::oprodv: {
			st = mul_res;
			break;
		}

		default: {
			break;
		}
	}
}

static void fu_add (
		op_t op,
		half &st, half ld0, half ld1,
		int i, int j, int k) {
#	pragma HLS inline off
# 	pragma HLS pipeline II=1
# 	pragma HLS allocation operation instances=hadd limit=1
	half ld_st = st;
	half add_op0, add_op1;

	switch (op) {
		default: {
			break;
		}

		case op::trm: {
			st = ld0;
			break;
		}

		case op::addm:
		case op::addv:
		case op::adds: {
			add_op0 = ld0;
			add_op1 = ld1;
			break;
		}

		case op::subm:
		case op::subv:
		case op::subs:
		case op::subcmv: {
			add_op1 = do_minus(ld1);
			add_op0 = ld0;
			break;
		}

		case op::absm:
		case op::absv:
		case op::abss: {
			st = do_abs(ld0);
			break;
		}

		case op::accsumcm: {
			add_op1 = ld0;
			if (j==0)
				add_op0 = 0;
			else
				add_op0 = ld_st;
			break;
		}

		case op::set0m: {
			st = 0;
			break;
		}

		case op::setidm: {
			st = (i==j);
			break;
		}

		case op::setd1:{
			st = (j==k)?(half)1.0f:ld0;
			break;
		}
	}
	switch (op) {
		case op::addm:
		case op::addv:
		case op::adds:
		case op::subm:
		case op::subcmv:
		case op::subv:
		case op::subs:
		case op::mulmm:
		case op::mulmv:
		case op::accsumcm: {
			st = add_op0 + add_op1;
			break;
		}

		default: {
			break;
		}
	}
}*/
