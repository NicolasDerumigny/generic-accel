#ifndef DMA_HPP
#define DMA_HPP

#include "ap_int.h"
#include "config.hpp"

using DMA_TYPE = ap_uint<64>;
constexpr int DMA_SIZE = REG_SIZ*N*N/4;

void recv_data_burst (DMA_TYPE data_in[DMA_SIZE], half reg_file[REG_SIZ][N*N]);
void send_data_burst (DMA_TYPE data_out[DMA_SIZE], half reg_file[REG_SIZ][N*N]);
void recv_pgm (ap_uint<8> op_loc[MAX_PGM_SIZE*NB_FU*4], ap_uint<8> op_remote[MAX_PGM_SIZE*NB_FU*4]);
#endif
