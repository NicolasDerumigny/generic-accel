#include <bitset>
#include <cstdint>
#include <iostream>
#include <chrono>

#include "core.hpp"

#include <cstdio>

#include "dma.hpp"
#include "hls_math.h"

void generic_accel(
		DMA_TYPE data_in[DMA_SIZE],
		DMA_TYPE data_out[DMA_SIZE],
		volatile ap_uint<64> *counter,
		ap_uint<64> *start_time,
		ap_uint<64> *end_time,
		ap_uint<8> pgm[MAX_PGM_SIZE*NB_FU*4]);

#define init() int i=0;\
		uint8_t r[512];\
		for (i=0;i<512;i++)r[i]=i;\
		i=0

#define op0(o, rd, ra, rb) do{\
		PRGM[i/NB_FU][0][0]=(uint8_t) op::o;\
		PRGM[i/NB_FU][0][1]=rd;\
		PRGM[i/NB_FU][0][2]=ra;\
		PRGM[i/NB_FU][0][3]=rb;\
		i++;\
} while (0)

#define op1(o, rd, ra, rb) do{\
		PRGM[i/NB_FU][1][0]=(uint8_t) op::o;\
		PRGM[i/NB_FU][1][1]=rd;\
		PRGM[i/NB_FU][1][2]=ra;\
		PRGM[i/NB_FU][1][3]=rb;\
		i++;\
} while (0)

#define op2(o, rd, ra, rb) do{\
		PRGM[i/NB_FU][2][0]=(uint8_t) op::o;\
		PRGM[i/NB_FU][2][1]=rd;\
		PRGM[i/NB_FU][2][2]=ra;\
		PRGM[i/NB_FU][2][3]=rb;\
		i++;\
} while (0)

#define op3(o, rd, ra, rb) do{\
		PRGM[i/NB_FU][3][0]=(uint8_t) op::o;\
		PRGM[i/NB_FU][3][1]=rd;\
		PRGM[i/NB_FU][3][2]=ra;\
		PRGM[i/NB_FU][3][3]=rb;\
		i++;\
} while (0)

#define halt() do {\
		PRGM[i/NB_FU][0][0]=(uint8_t) op::noop;\
		PRGM[i/NB_FU][0][1]=0;\
		PRGM[i/NB_FU][0][2]=0;\
		PRGM[i/NB_FU][0][3]=0;\
		PRGM[i/NB_FU][1][0]=(uint8_t) op::noop;\
		PRGM[i/NB_FU][1][1]=0;\
		PRGM[i/NB_FU][1][2]=0;\
		PRGM[i/NB_FU][1][3]=0;\
		PRGM[i/NB_FU][2][0]=(uint8_t) op::noop;\
		PRGM[i/NB_FU][2][1]=0;\
		PRGM[i/NB_FU][2][2]=0;\
		PRGM[i/NB_FU][2][3]=0;\
		i+=3;\
} while (0)

void init_prgm(ap_uint<8> PRGM[MAX_PGM_SIZE][NB_FU][4]) {
	const int null = -1;
	init();

	op0(addm, r[3], r[4], r[5]);
	op1(noop, null, null, null);
	op2(noop, null, null, null);

	op0(subm, r[0], r[1], r[2]);
	op1(noop, null, null, null);
	op2(noop, null, null, null);

	op1(noop, null, null, null);
	op0(mulmm, r[1], r[3], r[0]);
	op2(noop, null, null, null);

	halt();
}

inline int access(int reg_id, int i, int j) {
	return reg_id*N*N + i*N + j;
}

inline void print_reg_file(half reg_file[REG_SIZ*N*N]) {
	for (int id=0; id<REG_SIZ; ++id) {
		std::cout<<"reg"<<id<<":"<<std::endl;
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				std::cout<<reg_file[access(id, i, j)] << " ";
			}
			std::cout<<std::endl;
		}
	}
}

inline void print_matrix(half mat[N][N]) {
	std::cout<<"Mat:"<<std::endl;
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			std::cout<<mat[i][j] << " ";
		}
		std::cout<<std::endl;
	}
}

static void kernel_correlation(
		half data[N][N],
		half corr[N][N]) {

	half mean[N];
	half stddev[N];
	int i, j, k;

	float eps = 0.1f;

	half float_n = N;

	for (j = 0; j < N; j++) {
		mean[j] = 0.0;
		for (i = 0; i < N; i++)
			mean[j] += data[i][j];
		mean[j] /= float_n;
	}

	for (j = 0; j < N; j++) {
		stddev[j] = 0.0;
		for (i = 0; i < N; i++)
			stddev[j] += (data[i][j] - mean[j]) * (data[i][j] - mean[j]);

		stddev[j] /= float_n;
		stddev[j] = hls::sqrt(stddev[j]);
		/* The following in an inelegant but usual way to handle
           near-zero std. dev. values, which below would cause a zero-
           divide. */
		stddev[j] = (stddev[j] <= eps) ? ((half) 1.0) : stddev[j];
	}

	/* Center and reduce the column vectors. */
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++) {
			data[i][j] -= mean[j];
			data[i][j] /= hls::sqrt(float_n) * stddev[j];
		}

	/* Calculate the m * m correlation matrix. */
	for (i = 0; i < N - 1; i++) {
		corr[i][i] = 1.0;
		for (j = i + 1; j < N; j++) {
			corr[i][j] = 0.0;
			for (k = 0; k < N; k++)
				corr[i][j] += (data[k][i] * data[k][j]);
			corr[j][i] = corr[i][j];
		}
	}
	corr[N - 1][N - 1] = 1.0;
}

int main() {
	std::cout<<"Starting..."<<std::endl;
	// Initialise our data structures
	ap_uint<8> PRGM[MAX_PGM_SIZE][NB_FU][4];
	//init_prgm_simple (PRGM);
	init_prgm(PRGM);

	std::cout<<"Init data..."<<std::endl;
	// Initialise the channels
	half reg_file[REG_SIZ*N*N];
	half data[N][N];
	half corr[N][N];
	half reg_file_out[REG_SIZ*N*N];
	std::cout<<"Allocation done"<<std::endl;
	int val = -64;
	for (int id=0; id<REG_SIZ; ++id) {
		for (int i=0; i<N; i++) {
			for (int j=0; j<N; j++) {
				reg_file[access(id, i, j)] = (i==j)?1:0;
			}
		}
	}

	std::cout<<"Register File = "<<reg_file<<std::endl;
	std::cout<<"Out Register File = "<<reg_file_out<<std::endl;

	ap_uint<64> counter = 1;
	ap_uint<64> exec_time = 42;
	generic_accel(
			(DMA_TYPE*) reg_file,
			(DMA_TYPE*) reg_file_out,
			&counter,
			&exec_time,
			&exec_time,
			(ap_uint<8>*) PRGM);

	//kernel_correlation (data, corr);

	print_reg_file(reg_file_out);

	//print_matrix(corr);

	std::cout<<"Executed for matrix of size ";
	std::cout<<N<<"x"<<N<<std::endl;
	std::cout<<"Compute time:      ";
	std::cout<<exec_time;
	std::cout<<" cycles"<<std::endl;

	//return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
