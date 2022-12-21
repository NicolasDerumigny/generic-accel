#include "ap_int.h"
using DMA_TYPE = ap_uint<64>;
constexpr int DMA_SIZE = REG_SIZ*N*N/4;


#include "hls_stream.h"
using gu_t = hls::stream<half>;
