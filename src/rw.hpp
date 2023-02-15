#include "ap_int.h"
#include "config.hpp"

void multiple_readwrite_tbl (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
#		ifdef BLAS1
		half lc_reg[NB_FU],
#		endif
		const int ld0_addr[NB_FU], const int ld1_addr[NB_FU], const int st_ld_addr[NB_FU], const int st_wr_addr[NB_FU],
		half ld0[NB_FU], half ld1[NB_FU], half st_rd[NB_FU], const half st_wr[NB_FU],
		const bool write);

/*void multiple_read_tbl (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
		int ld0_addr[NB_FU], int ld1_addr[NB_FU], int st_addr[NB_FU],
		half ld0[NB_FU], half ld1[NB_FU], half st[NB_FU]);

void multiple_write_tbl (
		macro_op_t ops[NB_FU],
		half reg_file[REG_SIZ][N*N],
		int st_addr[NB_FU],
		half st[NB_FU]);
		*/
