#include "ap_utils.h"

#include "core.hpp"
#include "dma.hpp"
#include "fu.hpp"
#include "agu.hpp"
#include "lbg.hpp"
#include "rw.hpp"

#include "hls_print.h"

static inline void increment_idx(int &idx, const int max_val) {
#	pragma HLS inline
	idx++;
	if (idx == max_val)
		idx=0;
}

#ifdef TRGL
inline bool is_triangular(macro_op_t ops[NB_FU]) {
	bool ret = true;
	int i;
	for (i=0; i<NB_FU; i++) {
#		pragma HLS unroll
		op_t opcode = ops[i].opcode;
		ret = ret and (opcode == op::multrmm or opcode == op::multrmv or opcode == op::multrsm or opcode == op::addtrm or opcode == op::noop);
	}
	return ret;
}
#endif

int core(
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
		CU_INTERFACE) {
# 	pragma HLS inline
	// To avoid read/write conflicts (loop carried dependency by read/writes
	// to the same *local* buffer), we manually implement a rolling buffer
	// on temporary i/o vars
	//constexpr int LAT=8;

	int ld0_addr[NB_FU], ld1_addr[NB_FU], st_addr_ld[NB_FU], st_addr[FU_LATENCY][NB_FU];
#	pragma HLS ARRAY_PARTITION variable=ld0_addr dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=ld1_addr dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=st_addr dim=0 complete
	half ld0[NB_FU], ld1[NB_FU], st[NB_FU];
#	pragma HLS ARRAY_PARTITION variable=ld0 dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=ld1 dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=st dim=0 complete

	int id, idx, idx_st_addr=0;
	int i=0, j=0, k=0;

#	ifdef BLAS1
	int red_idx = 0;
	int lat_step = 0;
	half loop_carried_vals[NB_FU];
#	pragma HLS ARRAY_PARTITION variable=loop_carried_vals dim=0 complete
#	endif

	const int bound = lbg(ops);
#	ifdef TRGL
	const bool is_tr = is_triangular(ops);
#	endif

#	if defined(DIV) && defined(SQRT)
	half st_div[FU_LATENCY][NB_FU_DIVSQRT];
#	endif
	for (idx=0; idx<bound+FU_LATENCY; idx++) {
#		pragma HLS loop_flatten off
#		pragma HLS dependence dependent=false type=inter variable=reg_file
#		ifdef BLAS1
#		pragma HLS dependence dependent=false type=inter variable=loop_carried_vals
#		endif
#		if defined(DIV) && defined(SQRT)
#		pragma HLS dependence dependent=false type=inter variable=st_div
#		endif
#		pragma HLS pipeline II=1

		half res[NB_FU];
		if (not cu0_res.empty()
#			ifndef __SYNTHESIS__
				and idx>=FU_LATENCY // Simulate latency
#			endif
		) { // All CU are used with the same rate, so the queue should be filled at the same pace
			CUres(0) >> res[0];
			CUres(1) >> res[1];
			CUres(2) >> res[2];
			res[3] = st_div[idx_st_addr][0];
		}

		for (id=0; id<NB_FU; id++) {
#			pragma HLS unroll
			agu(ops[id].opcode, i, j, k,
#			ifdef BLAS1
				red_idx, lat_step,
#			endif
			ld0_addr[id], ld1_addr[id], st_addr_ld[id]);
		}

		multiple_readwrite_tbl (
				ops,
				reg_file,
#				ifdef BLAS1
				loop_carried_vals,
#				endif
				ld0_addr, ld1_addr, st_addr_ld, st_addr[idx_st_addr],
				ld0, ld1, st, res,
				idx >= FU_LATENCY);

		for (id=0; id<NB_FU; id++) {
#			pragma HLS unroll
			st_addr[idx_st_addr][id] = st_addr_ld[id];
		}

		if (idx < bound) {
			half a[NB_FU], b[NB_FU], c[NB_FU];
			for (id=0; id<NB_FU_ADDMUL; id++) {
#				pragma HLS unroll
				fu_addmul_axis(ops[id].opcode,
					st[id], ld0[id], ld1[id],
#					ifdef BLAS1
					loop_carried_vals[id],
#					endif
					i, j, k,
#					ifdef BLAS1
					red_idx, lat_step,
#					endif
					a[id], b[id], c[id]);
			}
			fu_divsqrt(ops[3].opcode,
					st_div[idx_st_addr][0], ld0[3], ld1[3],
					i, j, k);

#			ifdef __SYNTHESIS__
			CUa(0) << a[0];
			CUa(1) << a[1];
			CUa(2) << a[2];
			CUb(0) << b[0];
			CUb(1) << b[1];
			CUb(2) << b[2];
			CUc(0) << c[0];
			CUc(1) << c[1];
			CUc(2) << c[2];
#			else
			CUres(0) << a[0]*b[0]+c[0];
			CUres(1) << a[1]*b[1]+c[1];
			CUres(2) << a[2]*b[2]+c[2];
#			endif
		}


		increment_idx(idx_st_addr, FU_LATENCY);
		increment_idx(k, N);
		if (k==0) {
			increment_idx(j, N);
			if (j==0)
				i++;
#			ifdef TRGL
			if (is_tr) {
				k = j;
			}
#			endif
		}
#		ifdef BLAS1
		increment_idx(lat_step, FU_LATENCY);
		if (lat_step == 0) {
			red_idx++;
		}
#		endif
	}

	return bound;
}

void compute(ap_uint<8> pgml[MAX_PGM_SIZE*NB_FU*4],
		half reg_file[REG_SIZ][N*N],
		CU_INTERFACE) {
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
			reg_file,
			CU_INTERFACE_NAMES
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
		ap_uint<8> pgm[MAX_PGM_SIZE*NB_FU*4],
		CU_INTERFACE) {
#	pragma HLS INTERFACE mode=s_axilite port=return
#	pragma HLS INTERFACE mode=s_axilite port=start_time
#	pragma HLS INTERFACE mode=ap_none port=start_time register
#	pragma HLS INTERFACE mode=s_axilite port=end_time
#	pragma HLS INTERFACE mode=ap_none port=end_time register
#	pragma HLS INTERFACE mode=ap_none port=counter register=off
#	pragma HLS INTERFACE mode=s_axilite port=pgm
#	pragma HLS INTERFACE mode=m_axi bundle=data port=data_in offset=slave
#	pragma HLS INTERFACE mode=m_axi bundle=data port=data_out offset=slave
#	pragma HLS INTERFACE mode=axis port=cu0_a
#	pragma HLS INTERFACE mode=axis port=cu0_b
#	pragma HLS INTERFACE mode=axis port=cu0_c
#	pragma HLS INTERFACE mode=axis port=cu0_res
#	pragma HLS INTERFACE mode=axis port=cu1_a
#	pragma HLS INTERFACE mode=axis port=cu1_b
#	pragma HLS INTERFACE mode=axis port=cu1_c
#	pragma HLS INTERFACE mode=axis port=cu1_res
#	pragma HLS INTERFACE mode=axis port=cu2_a
#	pragma HLS INTERFACE mode=axis port=cu2_b
#	pragma HLS INTERFACE mode=axis port=cu2_c
#	pragma HLS INTERFACE mode=axis port=cu2_res

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
		compute (pgml, reg_file, CU_INTERFACE_NAMES);
		ap_wait();
		*end_time = *counter;
		ap_wait();
		send_data_burst(data_out, reg_file);
	}
}
