#include "core.hpp"
#include "dma.hpp"
#include "hls_print.h"
#include "hls_math.h"
#include "ap_utils.h"

#define max(a, b) ((a>b)?a:b)

static inline int access (int i, int j) {
#	pragma HLS inline
	return i*N + j;
}

static inline half cast_half(DMA_TYPE val) {
# 	pragma HLS inline
	return *(half*) &val;
}

static inline DMA_TYPE cast_dma_type(half val0, half val1, half val2, half val3) {
# 	pragma HLS inline
	DMA_TYPE cast0 = *(ap_uint<16>*) &val0;
	DMA_TYPE cast1 = *(ap_uint<16>*) &val1;
	DMA_TYPE cast2 = *(ap_uint<16>*) &val2;
	DMA_TYPE cast3 = *(ap_uint<16>*) &val3;

	return (cast0 | (cast1 << 16) | (cast2 << 32) | (cast3 << 48));
}

static inline void recv_data_burst (DMA_TYPE data_in[DMA_SIZE], half reg_file[REG_SIZ][N*N]) {
#	pragma HLS inline off
	int reg_id, i,j, idx;

	// for (reg_id=0; reg_id<REG_SIZ; reg_id++) {
	//	for (i=0; i<N; i++) {
	//		for (j=0; j<N; j+=4) {
	i=0;
	j=0;
	reg_id=0;
	for (idx=0; idx<DMA_SIZE; idx++) {
#		pragma HLS pipeline
#		pragma HLS dependence dependent=false type=inter variable=reg_file
#		pragma HLS dependence dependent=false type=intra variable=reg_file
		const DMA_TYPE val = data_in[idx];
		int addr = access(i,j);
		half *write_dst = reg_file[reg_id] + addr;
		write_dst[0] = cast_half((val >> 0) & 0xffff);
		write_dst[1] = cast_half((val >> 16) & 0xffff);
		write_dst[2] = cast_half((val >> 32) & 0xffff);
		write_dst[3] = cast_half((val >> 48) & 0xffff);

		/*hls::print("INCOMING REC:\n");
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 0] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 1] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 2] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 3] );
				*/
		j+=4;
		if (j==N) {
			j=0;
			i++;
			if (i==N) {
				i=0;
				reg_id++;
			}
		}
	}
}

static inline void send_data_burst (DMA_TYPE data_out[DMA_SIZE], half reg_file[REG_SIZ][N*N]) {
#	pragma HLS inline off
	int reg_id, i,j, idx;

	//for (reg_id=0; reg_id<REG_SIZ; reg_id++) {
	//	for (i=0; i<N; i++) {
	//		for (j=0; j<N/4; j++) {
	i=0;
	j=0;
	reg_id=0;
	for (idx=0; idx<DMA_SIZE; idx++) {
#		pragma HLS pipeline
		int addr = access(i,j);
		/*hls::print("OUTGOING PACKET:\n");
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 0] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 1] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 2] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 3] );*/
		const DMA_TYPE val = cast_dma_type(
				reg_file[reg_id][addr+0],
				reg_file[reg_id][addr+1],
				reg_file[reg_id][addr+2],
				reg_file[reg_id][addr+3]
			);
		data_out[idx] = val;
		j+=4;
		if (j==N) {
			j=0;
			i++;
			if (i==N) {
				i=0;
				reg_id++;
			}
		}
	}
}

static inline void recv_pgm (ap_uint<8> op_loc[MAX_PGM_SIZE*NB_FU*4], ap_uint<8> op_remote[MAX_PGM_SIZE*NB_FU*4]) {
#	pragma HLS inline off
	int idx;
	for (idx=0; idx<MAX_PGM_SIZE*NB_FU*4; idx++) {
#		pragma HLS pipeline
		op_loc[idx] = op_remote[idx];
	}
}

// loop_bound_generator
static inline int lbg (macro_op_t ops[NB_FU]) {
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

			case op::multrmm:
			case op::multrmv: {
				ret = max(ret, (N*N + 2*N + 1)/2);
				break;
			}

			case op::mulmv:
			case op::mulsm:
			case op::trm:
			case op::addm:
			case op::subm:
			case op::subcmv:
			case op::pmulm:
			case op::oprodv:
			case op::absm:
			case op::accsumcm:
			//case op::divms:
			//case op::divcmv:
			case op::set0m:
			case op::setidm:
			case op::setd1: {
				ret = max(ret, N*N);
				break;
			}

			case op::mulsv:
			case op::addv:
			case op::subv:
			case op::pmulv:
			case op::absv:
			//case op::sqrtv:
			case op::cutminv: {
			//case op::divvs: {
				ret = max(ret, N);
				break;
			}

			case op::muls:
			case op::adds:
			case op::subs:
			case op::abss: {
			//case op::sqrts: {
				ret = max(ret, 1);
				break;
			}
		}
	}
	return ret;
}

static inline void agu (
		op_t op,
		int i, int j, int k,
		int &ld0_addr,
		int &ld1_addr,
		int &st_addr) {
# 	pragma HLS inline
	ld0_addr = -1;
	ld1_addr = -1;
	st_addr = -1;

	switch (op) {
		case op::mulmm: {
			ld0_addr = access(i,j);
			ld1_addr = access(j,k);
			st_addr = access(i,k);
			break;
		}

		case op::multrmm: {
			if (k>=j) {
				ld0_addr = access(i,j);
				ld1_addr = access(j,k);
				st_addr = access(i,k);
				break;
			}
		}

		case op::mulmv: {
			if (i==0) {
				ld0_addr = access(k,j);
				ld1_addr = access(0,j);
				st_addr = access(j,k);
			}
			break;
		}

		case op::multrmv:{
			if (i==0 && k>=j) {
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

		case op::multrsm: {
			if (i==0 && k>=j) {
				ld0_addr = access(0,0);
				ld1_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}

/*
		case op::divms: {
			if (i==0) {
				ld0_addr = access(j,k);
				ld1_addr = access(0,0);
				st_addr = access(j,k);
			}
			break;
		}
*/

		case op::trm: {
			if (i==0) {
				ld0_addr = access(j,k);
				st_addr = access(k,j);
			}
			break;
		}

		case op::addm:
		case op::subm:
		case op::pmulm: {
			if (i==0) {
				ld0_addr = access(j,k);
				ld1_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}

		case op::addtrm: {
			if (i==0 and k>j) {
				ld0_addr = access(j,k);
				ld1_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}

		case op::addv:
		case op::subv:
		case op::pmulv: {
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

		case op::subcmv: {
//		case op::divcmv: {
			if (i==0) {
				ld0_addr = access(j,k);
				ld1_addr = access(0,k);
				st_addr = access(j,k);
			}
			break;
		}

		case op::accsumcm: {
			if (i==0) {
				ld0_addr = access(j,k);
				st_addr = access(0,k);
			}
			break;
		}

		case op::absm: {
			if (i==0) {
				ld0_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}

		case op::set0m:
		case op::setidm: {
			if (i==0) {
				st_addr = access(j,k);
			}
			break;
		}

		case op::absv:
		//case op::sqrtv:
		case op::cutminv: {
			if (i==0 and j==0) {
				ld0_addr = access(0,k);
				st_addr = access(0,k);
			}
			break;
		}

		//case op::sqrts:
		case op::abss: {
			if (i==0 and j==0 and k==0) {
				ld0_addr = access(0,0);
				st_addr = access(0,0);
			}
			break;
		}

		case op::setd1: {
			if (i==0) {
				ld0_addr = access(j,k);
				st_addr = access(j,k);
			}
			break;
		}

		case op::noop: {
			break;
		}
	}
}

inline half do_minus(half value) {
	ap_uint<16> minus_in = *(ap_uint<16>*) &value;
	minus_in = ( ((~minus_in) & 0x8000) | (minus_in & 0x7FFF));
	return *(half*) &minus_in;
}


inline half do_abs(half value) {
	ap_uint<16> abs = *(ap_uint<16>*) &value;
	abs = abs & 0x7FFF;
	return *(half*) &abs;
}

static void fu_addmul (
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
		case op::oprodv:
		case op::multrmm:
		case op::multrsm:
		case op::multrmv: {
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

		case op::multrmm:
		case op::multrmv: {
			add_op1 = mul_res;
			if (j==k)
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
		case op::addtrm:
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
		case op::oprodv:
		case op::multrsm: {
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

/*	case op::absm:
	case op::absv:
	case op::abss: {
		st = do_abs(ld0);
		break;
	}*/

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

		/*case op::divms:
		case op::divvs:
		case op::divcmv: {
//#pragma HLS bind_op variable=st op=hdiv impl=dsp
			st = ld0/ld1;
			break;
		}*/

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
		case op::adds:
		case op::addtrm: {
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

/*		case op::absm:
		case op::absv:
		case op::abss: {
			st = do_abs(ld0);
			break;
		}*/

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
}

/*static inline void read (
		macro_op_t ops,
		half reg_file[REG_SIZ][N*N],
		int ld0_addr, int ld1_addr, int st_addr,
		half &ld0, half &ld1, half &st) {
#	pragma HLS inline
	ld0 = reg_file[ops.r0][ld0_addr];
	ld1 = reg_file[ops.r1][ld1_addr];
	st  = reg_file[ops.r_dst][st_addr];
}*/

/*static inline void multiple_read (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
		int ld0_addr0, int ld1_addr0, int st_addr0,
		half &ld0_0, half &ld1_0, half &st0,
		int ld0_addr1, int ld1_addr1, int st_addr1,
		half &ld0_1, half &ld1_1, half &st1) {
#	pragma HLS inline
	int i;
	for (i=0; i<REG_SIZ; i++) {
#		pragma HLS unroll
		int offset = -1;
		if (ops[0].r0 == i)
			offset = ld0_addr0;
		if (ops[0].r1 == i)
			offset = ld1_addr0;
		if (ops[0].r_dst == i)
			offset = st_addr0;
		if (ops[1].r0 == i)
			offset = ld0_addr1;
		if (ops[1].r1 == i)
			offset = ld1_addr1;
		if (ops[1].r_dst == i)
			offset = st_addr1;
		if (offset>=0) {
			half value = reg_file[i][offset];
			if (ops[0].r0 == i)
				ld0_0 = value;
			if (ops[0].r1 == i)
				ld1_0 = value;
			if (ops[0].r_dst == i)
				st0 = value;
			if (ops[1].r0 == i)
				ld0_1 = value;
			if (ops[1].r1 == i)
				ld1_1 = value;
			if (ops[1].r_dst == i)
				st1 = value;
		}
	}
}*/

static inline void multiple_read_tbl (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
		int ld0_addr[NB_FU], int ld1_addr[NB_FU], int st_addr[NB_FU],
		half ld0[NB_FU], half ld1[NB_FU], half st[NB_FU]) {
#	pragma HLS inline
	int i,j;
	for (i=0; i<REG_SIZ; i++) {
#		pragma HLS unroll

		int offset = -1;
		for (j=0; j<NB_FU; j++) {
#			pragma HLS unroll
			if (ops[j].r0 == i)
				offset = ld0_addr[j];
			if (ops[j].r1 == i)
				offset = ld1_addr[j];
			if (ops[j].r_dst == i)
				offset = st_addr[j];
		}
		if (offset>=0) {
			half value = reg_file[i][offset];
			for (j=0; j<NB_FU; j++) {
#				pragma HLS unroll
				if (ops[j].r0 == i)
					ld0[j] = value;
				if (ops[j].r1 == i)
					ld1[j] = value;
				if (ops[j].r_dst == i)
					st[j] = value;
			}
		}
	}
}

/*static inline void write (
		macro_op_t ops,
		half reg_file[REG_SIZ][N*N],
		int st_addr,
		half st) {
#	pragma HLS inline
	reg_file[ops.r_dst][st_addr] = st;
}*/

/*static inline void multiple_write (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
		int st_addr0,
		half st0,
		int st_addr1,
		half st1) {
#	pragma HLS inline
	ap_uint<8> match = ops[1].r_dst << 4 | ops[0].r_dst;
	switch (match) {
		default:
		break;

#		define dual_store(match, reg1, reg0) \
		case match:\
			if (st_addr1 >= 0) reg_file[reg1][st_addr1] = st1;\
			if (st_addr0 >= 0) reg_file[reg0][st_addr0] = st0;\
			break;
#		include "store_unit.gen"
	}
}*/

/*static inline void multiple_write (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
		int st_addr0,
		half st0,
		int st_addr1,
		half st1) {
#	pragma HLS inline
	int i;
	for (i=0; i<REG_SIZ; i++) {
#		pragma HLS unroll
		int offset = -1;
		half value = 0;
		if (ops[0].r_dst == i) {
			offset = st_addr0;
			value = st0;
		} else if (ops[1].r_dst == i) {
			offset = st_addr1;
			value = st1;
		}
		if (offset>=0)
			reg_file[i][offset] = value;
	}
}*/

static inline void multiple_write_tbl (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
		int st_addr[NB_FU],
		half st[NB_FU]) {
#	pragma HLS inline
	int i, j;
	for (i=0; i<REG_SIZ; i++) {
#		pragma HLS unroll
		int the_one = -1;
		for (j=0; j<NB_FU; j++) {
#			pragma HLS unroll
			if (ops[j].r_dst == i) {
				the_one = j;
			}
		}
		if (the_one>=0)
			reg_file[i][st_addr[the_one]] = st[the_one];
	}
}

inline bool is_triangular (macro_op_t ops[NB_FU]) {
	bool ret = true;
	int i;
	for (i=0; i<NB_FU; i++) {
#		pragma HLS unroll
		op_t opcode = ops[i].opcode;
		ret = ret and (opcode == op::multrmm or opcode == op::multrmv or opcode == op::multrsm or opcode == op::addtrm);
	}
	return ret;
}

int core(
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N]) {
# 	pragma HLS inline
	// To avoid read/write conflicts (loop carried dependency by read/writes
	// to the same *local* buffer), we manually implement a rolling buffer
	// on temporary i/o vars
	constexpr int LAT=3;

	int ld0_addr[NB_FU], ld1_addr[NB_FU], st_addr[NB_FU];
#	pragma HLS ARRAY_PARTITION variable=ld0_addr dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=ld1_addr dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=st_addr dim=0 complete
	half ld0[NB_FU], ld1[NB_FU], st_g[LAT][NB_FU];
#	pragma HLS ARRAY_PARTITION variable=ld0 dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=ld1 dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=st_g dim=0 complete

	int idx;
	int i=0, j=0, k=0;
	const int bound = lbg(ops);
	const bool trg_gen = is_triangular(ops);
	for (idx=0; idx<bound; idx++) {
#		pragma HLS loop_flatten off
#		pragma HLS dependence dependent=false type=inter variable=reg_file
#		pragma HLS pipeline II=1

		/*int ld0_addr0, ld1_addr0, st_addr0;
		int ld0_addr1, ld1_addr1, st_addr1;
		half ld0_0=0, ld1_0=0, st0=0;
		half ld0_1=0, ld1_1=0, st1=0;*/

		//op opcode0 = ops[0].opcode;
		//op opcode1 = ops[1].opcode;
		//agu (opcode0, i, j, k, ld0_addr0, ld1_addr0, st_addr0);
		//agu (opcode1, i, j, k, ld0_addr1, ld1_addr1, st_addr1);

		int id;
		for (id=0; id<NB_FU; id++) {
#			pragma HLS unroll
			agu(ops[id].opcode, i, j, k, ld0_addr[id], ld1_addr[id], st_addr[id]);
		}

		/*read (ops[0], reg_file, ld0_addr0, ld1_addr0, st_addr0, ld0_0, ld1_0, st0);
		read (ops[1], reg_file, ld0_addr1, ld1_addr1, st_addr1, ld0_1, ld1_1, st1);*/
		/*multiple_read(ops, reg_file,
				ld0_addr0, ld1_addr0, st_addr0, ld0_0, ld1_0, st0,
				ld0_addr1, ld1_addr1, st_addr1, ld0_1, ld1_1, st1);*/
		/*multiple_read(ops, reg_file,
			ld0_addr[0], ld1_addr[0], st_addr[0], ld0[0], ld1[0], st[0],
			ld0_addr[1], ld1_addr[1], st_addr[1], ld0[1], ld1[1], st[1]);*/

		multiple_read_tbl(ops, reg_file,
			ld0_addr, ld1_addr, st_addr, ld0, ld1, st_g[0]);


		for (id=0; id<NB_FU; id++) {
#			pragma HLS unroll
			st_g[1][id] = st_g[0][id];
			if (id<NB_FU_GEN) {
				fu_addmul(ops[id].opcode,
					st_g[1][id], ld0[id], ld1[id],
					i, j, k);
			} else if (id<(NB_FU_GEN + NB_FU_MUL)) {
				fu_mul(ops[id].opcode,
					st_g[1][id], ld0[id], ld1[id],
					i, j, k);
			} else {
				fu_add(ops[id].opcode,
					st_g[1][id], ld0[id], ld1[id],
					i, j, k);
			}
			st_g[2][id] = st_g[1][id];
		}

		/*write (ops[0], reg_file, st_addr0, st0);
		write (ops[1], reg_file, st_addr1, st1);*/
		/*multiple_write(ops, reg_file,
				st_addr0, st0,
				st_addr1, st1);*/
		/*multiple_write(ops, reg_file,
			st_addr[0], st[0],
			st_addr[1], st[1]);*/

		multiple_write_tbl(ops, reg_file,
			st_addr, st_g[2]);

		k++;
		// i = 0 to N
		if (trg_gen) {
			if (k==N) {
				j++;
				if (j==N){
					i++;
					j = 0;
				}
				k = j;
			}
		} else {
			if (k==N) {
				j++;
				k = 0;
				if (j==N){
					i++;
					j = 0;
				}
			}
		}
	}

	return bound;
}

void compute(ap_uint<8> pgml[MAX_PGM_SIZE*NB_FU*4],
		half reg_file[REG_SIZ][N*N]) {
#	pragma HLS inline off
	for (int pc=0; pc<MAX_PGM_SIZE; pc++) {
#		pragma HLS loop_flatten off

		macro_op_t m_ins[NB_FU];
		ap_uint<8> *offset = pgml + NB_FU*4*pc;
		int i=0;
		int idx;
		for (idx=0; idx<NB_FU; idx++) {
			m_ins[idx].opcode = (op_t) (uint8_t) offset[i+0];
			m_ins[idx].r_dst = offset[i+1];
			m_ins[idx].r0 = offset[i+2];
			m_ins[idx].r1 = offset[i+3];
			i+=4;
		}

		int nb_it = core(
			m_ins,
			reg_file
		);

		if (nb_it == 0) // Nothing was done: only NOPs
			break;
	}
}

void generic_accel(
		DMA_TYPE data_in[DMA_SIZE],
		DMA_TYPE data_out[DMA_SIZE],
		volatile ap_uint<64> *counter,
		ap_uint<64> *start_time,
		ap_uint<64> *end_time,
		ap_uint<8> pgm[MAX_PGM_SIZE*NB_FU*4]) {
#	pragma HLS INTERFACE mode=s_axilite port=return
#	pragma HLS INTERFACE mode=s_axilite port=start_time
#	pragma HLS INTERFACE mode=ap_none port=start_time register
#	pragma HLS INTERFACE mode=s_axilite port=end_time
#	pragma HLS INTERFACE mode=ap_none port=end_time register
#	pragma HLS INTERFACE mode=ap_none port=counter register=off
#	pragma HLS INTERFACE mode=s_axilite port=pgm
#	pragma HLS INTERFACE mode=m_axi bundle=data port=data_in offset=slave
#	pragma HLS INTERFACE mode=m_axi bundle=data port=data_out offset=slave

	half reg_file[REG_SIZ][N*N];
#	pragma HLS BIND_STORAGE variable=reg_file type=ram_t2p impl=bram
#	pragma HLS ARRAY_PARTITION variable=reg_file dim=1 complete
#	pragma HLS ARRAY_PARTITION variable=reg_file dim=2 type=cyclic factor=2
	ap_uint<8> pgml[MAX_PGM_SIZE*NB_FU*4];
#	pragma HLS BIND_STORAGE variable=reg_file type=ram_t2p impl=bram

	measure: {
#pragma HLS PROTOCOL mode=fixed
		recv_data_burst(data_in, reg_file);
		recv_pgm(pgml, pgm);
		ap_wait();
		*start_time = *counter;
		ap_wait();
		compute (pgml, reg_file);
		ap_wait();
		*end_time = *counter;
		ap_wait();
		send_data_burst(data_out, reg_file);
	}
}
