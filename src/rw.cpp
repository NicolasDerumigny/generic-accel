#include "rw.hpp"

#include "hls_print.h"

void multiple_readwrite_tbl (
		macro_op_t ops[NB_FU],
		dtype_t reg_file[REG_SIZ][N*N],
#		ifdef BLAS1
		dtype_t lc_reg[NB_FU],
#		endif
		int ld0_addr[NB_FU], int ld1_addr[NB_FU], int st_ld_addr[NB_FU], int st_wr_addr[NB_FU],
		dtype_t ld0[NB_FU], dtype_t ld1[NB_FU], dtype_t st_rd[NB_FU], dtype_t st_wr[NB_FU],
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
			dtype_t value = reg_file[i][offset_ld];
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
			dtype_t value = st_wr[st_reg];
# 			ifdef BLAS1
			if (st_wr_addr[st_reg] != RED_REG) {
#			endif
				//hls::print("%f\n", (float) value);
				reg_file[i][st_wr_addr[st_reg]] = value;
# 			ifdef BLAS1
			} else  {
				//hls::print("PROUT %f\n", (float) value);
				lc_reg[st_reg] = value;
			}
#			endif
		}
	}
}
