#include <bitset>
#include <cstdint>
#include <iostream>
#include <chrono>

#include "core.hpp"

#include <cstdio>

#include "dma.hpp"
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
        PRGM[i/2][0][0]=(uint8_t) op::o;\
        PRGM[i/2][0][1]=rd;\
        PRGM[i/2][0][2]=ra;\
        PRGM[i/2][0][3]=rb;\
        i++;\
    } while (0)

#define op1(o, rd, ra, rb) do{\
        PRGM[i/2][1][0]=(uint8_t) op::o;\
        PRGM[i/2][1][1]=rd;\
        PRGM[i/2][1][2]=ra;\
        PRGM[i/2][1][3]=rb;\
        i++;\
    } while (0)

#define halt() do {\
        PRGM[i/2][0][0]=(uint8_t) op::noop;\
        PRGM[i/2][0][1]=0;\
        PRGM[i/2][0][2]=0;\
        PRGM[i/2][0][3]=0;\
        PRGM[i/2][1][0]=(uint8_t) op::noop;\
        PRGM[i/2][1][1]=0;\
        PRGM[i/2][1][2]=0;\
        PRGM[i/2][1][3]=0;\
        i+=2;\
    } while (0)

void init_prgm(ap_uint<8> PRGM[MAX_PGM_SIZE][NB_FU][4]) {
	init();

    op1(addm, r[3], r[4], r[5]);
    op0(subm, r[0], r[1], r[2]);

    op0(noop, r[4], r[5], r[5]);
    op1(mulmm, r[1], r[3], r[0]);

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

int main() {
    std::cout<<"Starting..."<<std::endl;
    // Initialise our data structures
    ap_uint<8> PRGM[MAX_PGM_SIZE][NB_FU][4];
    init_prgm(PRGM);

    std::cout<<"Init data..."<<std::endl;
    // Initialise the channels
    half reg_file[REG_SIZ*N*N];
    half reg_file_out[REG_SIZ*N*N];
    std::cout<<"Allocation done"<<std::endl;
    for (int id=0; id<REG_SIZ; ++id) {
    	for (int i=0; i<N; i++) {
        	for (int j=0; j<N; j++) {
                reg_file[access(id, i, j)] = (i == j)? 1. : 0;
                reg_file_out[access(id, i, j)] = 0;
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


    print_reg_file(reg_file_out);

    std::cout<<"Executed for matrix of size ";
    std::cout<<N<<"x"<<N<<std::endl;
    std::cout<<"Compute time:      ";
    std::cout<<exec_time;
    std::cout<<" cycles"<<std::endl;

    //return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
