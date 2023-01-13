#include "rw.hpp"

//#include "hls_print.h"

void multiple_readwrite_tbl (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
#		ifdef BLAS1
		half lc_reg[NB_FU],
#		endif
		int ld0_addr[NB_FU], int ld1_addr[NB_FU], int st_ld_addr[NB_FU], int st_wr_addr[NB_FU],
		half ld0[NB_FU], half ld1[NB_FU], half st_rd[NB_FU], half st_wr[NB_FU],
		bool write) {
#	pragma HLS inline
	int i,j;
	for (i=0; i<REG_SIZ; i++) {
#		pragma HLS unroll

		int offset_ld = -1;
		int st_reg = -1;
		for (j=0; j<NB_FU; j++) {
#			pragma HLS unroll
			if (ops[j].r0 == i)
				offset_ld = ld0_addr[j];
			if (ops[j].r1 == i)
				offset_ld = ld1_addr[j];
			if (ops[j].r_dst == i) {
				offset_ld = st_ld_addr[j];
				st_reg = j;
			}
		}
		if (offset_ld>=0) {
			half value = reg_file[i][offset_ld];
			for (j=0; j<NB_FU; j++) {
#				pragma HLS unroll
				if (ops[j].r0 == i)
					ld0[j] = value;
				if (ops[j].r1 == i)
					ld1[j] = value;
				if (ops[j].r_dst == i)
					st_rd[j] = value;
			}
		}
		if (write and st_reg >= 0) {
			half value = st_wr[st_reg];
# 			ifdef BLAS1
			if (st_wr_addr[st_reg] != RED_REG) {
#			endif
				//hls::print("%f\n", (float) value);
				reg_file[i][st_wr_addr[st_reg]] = value;
# 			ifdef BLAS1
			} else  {
				lc_reg[st_reg] = value;
			}
#			endif
		}
	}
}

/*
void multiple_read_tbl (
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

void multiple_write_tbl (
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
*/
