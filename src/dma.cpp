#include "dma.hpp"

static inline int access (int i, int j) {
#	pragma HLS inline
	return i*N + j;
}

static inline half cast_half(DMA_TYPE val) {
# 	pragma HLS inline
	return *(half*) &val;
}

static inline DMA_TYPE cast_dma_type(half val0, half val1, half val2, half val3) {
# 	pragma HLS inline
	DMA_TYPE cast0 = *(ap_uint<16>*) &val0;
	DMA_TYPE cast1 = *(ap_uint<16>*) &val1;
	DMA_TYPE cast2 = *(ap_uint<16>*) &val2;
	DMA_TYPE cast3 = *(ap_uint<16>*) &val3;

	return (cast0 | (cast1 << 16) | (cast2 << 32) | (cast3 << 48));
}

void recv_data_burst (DMA_TYPE data_in[DMA_SIZE], half reg_file[REG_SIZ][N*N]) {
#	pragma HLS inline off
	int reg_id, i,j, idx;

	// for (reg_id=0; reg_id<REG_SIZ; reg_id++) {
	//	for (i=0; i<N; i++) {
	//		for (j=0; j<N; j+=4) {
	i=0;
	j=0;
	reg_id=0;
	for (idx=0; idx<DMA_SIZE; idx++) {
#		pragma HLS pipeline
#		pragma HLS dependence dependent=false type=inter variable=reg_file
#		pragma HLS dependence dependent=false type=intra variable=reg_file
		const DMA_TYPE val = data_in[idx];
		int addr = access(i,j);
		half *write_dst = reg_file[reg_id] + addr;
		write_dst[0] = cast_half((val >> 0) & 0xffff);
		write_dst[1] = cast_half((val >> 16) & 0xffff);
		write_dst[2] = cast_half((val >> 32) & 0xffff);
		write_dst[3] = cast_half((val >> 48) & 0xffff);

		/*hls::print("INCOMING REC:\n");
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 0] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 1] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 2] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 3] );
				*/
		j+=4;
		if (j==N) {
			j=0;
			i++;
			if (i==N) {
				i=0;
				reg_id++;
			}
		}
	}
}

void send_data_burst (DMA_TYPE data_out[DMA_SIZE], half reg_file[REG_SIZ][N*N]) {
#	pragma HLS inline off
	int reg_id, i,j, idx;

	//for (reg_id=0; reg_id<REG_SIZ; reg_id++) {
	//	for (i=0; i<N; i++) {
	//		for (j=0; j<N/4; j++) {
	i=0;
	j=0;
	reg_id=0;
	for (idx=0; idx<DMA_SIZE; idx++) {
#		pragma HLS pipeline
		int addr = access(i,j);
		/*hls::print("OUTGOING PACKET:\n");
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 0] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 1] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 2] );
		hls::print("Rec: %f\n",
				(float) reg_file[reg_id][addr + 3] );*/
		const DMA_TYPE val = cast_dma_type(
				reg_file[reg_id][addr+0],
				reg_file[reg_id][addr+1],
				reg_file[reg_id][addr+2],
				reg_file[reg_id][addr+3]
			);
		data_out[idx] = val;
		j+=4;
		if (j==N) {
			j=0;
			i++;
			if (i==N) {
				i=0;
				reg_id++;
			}
		}
	}
}

void recv_pgm (ap_uint<8> op_loc[MAX_PGM_SIZE*NB_FU*4], ap_uint<8> op_remote[MAX_PGM_SIZE*NB_FU*4]) {
#	pragma HLS inline off
	int idx;
	for (idx=0; idx<MAX_PGM_SIZE*NB_FU*4; idx++) {
#		pragma HLS pipeline
		op_loc[idx] = op_remote[idx];
	}
}
