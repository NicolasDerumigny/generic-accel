#include "ap_int.h"
#include "config.hpp"

void multiple_readwrite_tbl (
		macro_op_t ops[NB_FU],
		dtype_t reg_file[REG_SIZ][N*N],
#		ifdef BLAS1
		dtype_t lc_reg[NB_FU],
#		endif
		int ld0_addr[NB_FU], int ld1_addr[NB_FU], int st_ld_addr[NB_FU], int st_wr_addr[NB_FU],
		dtype_t ld0[NB_FU], dtype_t ld1[NB_FU], dtype_t st_rd[NB_FU], dtype_t st_wr[NB_FU],
		bool write);
