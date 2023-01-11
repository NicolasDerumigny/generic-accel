#ifndef CORE_HPP
#define CORE_HPP
#include "ap_int.h"
#include "config.hpp"
#include "dma.hpp"
#include "hls_stream.h"

constexpr float CUTOFF = 0.1f;


using gu_t = hls::stream<half>;

void generic_accel(
		DMA_TYPE data_in[DMA_SIZE],
		DMA_TYPE data_out[DMA_SIZE],
		volatile ap_uint<64> *counter,
		ap_uint<64> *start_time,
		ap_uint<64> *end_time,
		ap_uint<8> pgm[MAX_PGM_SIZE*NB_FU*4],
		CU_INTERFACE);

#endif // CORE_HPP
