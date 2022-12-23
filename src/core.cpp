#include "ap_utils.h"
#include "hls_print.h"

#include "core.hpp"
#include "dma.hpp"
#include "fu.hpp"
#include "agu.hpp"
#include "lbg.hpp"
#include "rw.hpp"

static inline void increment_idx(int &idx, const int max_val) {
#	pragma HLS inline
	idx++;
	if (idx == max_val)
		idx=0;
}

#ifdef TRGL
inline bool is_triangular (macro_op_t ops[NB_FU]) {
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
	constexpr int LAT=9;

	int ld0_addr[NB_FU], ld1_addr[NB_FU], st_addr[LAT][NB_FU];
#	pragma HLS ARRAY_PARTITION variable=ld0_addr dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=ld1_addr dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=st_addr dim=0 complete
	half ld0[NB_FU], ld1[NB_FU], st[NB_FU];
#	pragma HLS ARRAY_PARTITION variable=ld0 dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=ld1 dim=0 complete
#	pragma HLS ARRAY_PARTITION variable=st dim=0 complete

	int idx, idx_st_addr=0;
	int i=0, j=0, k=0;
	const int bound = lbg(ops);
	for (idx=0; idx<bound+LAT; idx++) {
#		pragma HLS loop_flatten off
#		pragma HLS dependence dependent=false type=inter variable=reg_file
#		pragma HLS pipeline II=1

		if (not cu0_res.empty()) { // All CU are used with the same rate, so the queue should be filled at the same pace
			half res[NB_FU];
			cu0_res >> res[0];
			cu1_res >> res[1];
			multiple_write_tbl(ops, reg_file,
					st_addr[idx_st_addr], res);
		}

		if (idx < bound) {
			int id;
			for (id=0; id<NB_FU; id++) {
	#			pragma HLS unroll
				agu(ops[id].opcode, i, j, k, ld0_addr[id], ld1_addr[id], st_addr[idx_st_addr][id]);
			}

			multiple_read_tbl(ops, reg_file,
				ld0_addr, ld1_addr, st_addr[idx_st_addr], ld0, ld1, st);


			half a0, b0, c0;
			half a1, b1, c1;
			fu_addmul_axis(ops[0].opcode,
				st[0], ld0[0], ld1[0],
				i, j, k,
				a0, b0, c0);
			fu_addmul_axis(ops[1].opcode,
				st[1], ld0[1], ld1[1],
				i, j, k,
				a1, b1, c1);

			cu0_a << a0;
			cu0_b << b0;
			cu0_c << c0;
			cu1_a << a1;
			cu1_b << b1;
			cu1_c << c1;
		}

		increment_idx(idx_st_addr, LAT);
		increment_idx(k, N);
		if (k==0) {
			increment_idx(j, N);
			if (j==0)
				i++;
#			ifdef TRGL
			k = j;
#			endif
		}
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
