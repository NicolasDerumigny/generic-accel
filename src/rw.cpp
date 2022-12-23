#include "rw.hpp"

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
