#include <bitset>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <chrono>

#include "core.hpp"
#include "hls_math.h"

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
		PRGM[i/NB_FU][3][0]=(uint8_t) op::noop;\
		PRGM[i/NB_FU][3][1]=0;\
		PRGM[i/NB_FU][3][2]=0;\
		PRGM[i/NB_FU][3][3]=0;\
		i+=4;\
} while (0)

/*
void init_prgm_simple(ap_uint<8> PRGM[MAX_PGM_SIZE][NB_FU][4]) {
	int null = -1;
	init();

	op0(addm, r[3], r[4], r[5]);
	//op1(subm, r[0], r[1], r[2]);
	//op2(noop, null, null, null);
	//op3(noop, null, null, null);

	//op0(mulmm, r[1], r[3], r[0]);
	op1(noop, null, null, null);
	//op2(noop, null, null, null);
	//op3(noop, null, null, null);

	op0(dotv, r[0], r[3], r[1]);
	op1(noop, null, null, null);

	op0(sasum, r[6], r[7], null);
	op1(noop, null, null, null);

	halt();
}*/

void init_prgm(ap_uint<8> PRGM[MAX_PGM_SIZE][NB_FU][4]) {
	init();

	int float_n = r[0];
	int sqrtfloat_n = r[1];
	int data = r[2];
	int corr = r[3];
	int mean = r[4];
	int stddev = r[5];
	int tmp1 = r[6];
	int tmp2 = r[7];
	int tmp3 = r[8];
	int null = -1;

	op0(set0m, mean, null, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(accsumcm, mean, data, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(noop, null, null, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(divvs, mean, mean, float_n);

	op0(set0m, stddev, null, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(subcmv, data, data, mean);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(pmulm, tmp1, data, data);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(accsumcm, stddev, tmp1, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(noop, null, null, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(divms, stddev, stddev, float_n);

	op0(noop, null, null, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(sqrtv, stddev, stddev, null);

	op0(cutminv, stddev, stddev, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(mulsv, stddev, sqrtfloat_n, stddev);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(noop, null, null, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(divcmv, data, data, stddev);

	op0(trm, tmp1, data, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(mulmm, corr, tmp1, data);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

	op0(setd1, corr, corr, null);
	op1(noop, null, null, null);
	op2(noop, null, null, null);
	op3(noop, null, null, null);

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
				//reg_file[access(id, i, j)] = (i==j)?1:0;
				if (id == 0) {
					reg_file[access(id, i, j)] = (i==0 and j==0)?N:0;
				} else if (id == 1) {
					reg_file[access(id, i, j)] = (i==0 and j==0)?hls::sqrt(N):0;
				} else {
					reg_file[access(id, i, j)] = i - j + (float) val/100;
					val++;
				}
				reg_file_out[access(id, i, j)] = 0;
				if (id == 2) {
					corr[i][j] = 100000;
					data[i][j] = reg_file[access(id, i, j)];
				}
			}
		}
	}
	/*for (int i=0; i<N; i++) {
				for (int j=0; j<N; j++) {
					if (j%2)
						reg_file[access(7, i, j)] = (i==0)?1:0;
					else
						reg_file[access(7, i, j)] = (i==0)?-1:0;
				}
	}*/

	std::cout<<"Register File = "<<reg_file<<std::endl;
	std::cout<<"Out Register File = "<<reg_file_out<<std::endl;

	ap_uint<64> counter = 1;
	ap_uint<64> exec_time = 42;
	gu_t stream_a, stream_b, stream_c, stream_res;
	gu_t stream_a1, stream_b1, stream_c1, stream_res1;
	gu_t stream_a2, stream_b2, stream_c2, stream_res2;
	generic_accel(
			(DMA_TYPE*) reg_file,
			(DMA_TYPE*) reg_file_out,
			&counter,
			&exec_time,
			&exec_time,
			(ap_uint<8>*) PRGM,
			stream_a,
			stream_b,
			stream_c,
			stream_res,
			stream_a1,
			stream_b1,
			stream_c1,
			stream_res1,
			stream_a2,
			stream_b2,
			stream_c2,
			stream_res2);

	kernel_correlation (data, corr);

	print_reg_file(reg_file_out);

	print_matrix(corr);

	std::cout<<"Executed for matrix of size ";
	std::cout<<N<<"x"<<N<<std::endl;
	std::cout<<"Compute time:      ";
	std::cout<<exec_time;
	std::cout<<" cycles"<<std::endl;

	//return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
