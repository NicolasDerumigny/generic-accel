#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>

#include "arm.h"
#include "lib/cma.hpp"

#include "core.hpp"
#include "drivers/xgeneric_accel.h"

using half = __fp16;

constexpr uintptr_t MAP_ADDR = 0xA0000000;

alignas(uint32_t) uint8_t PRGM[MAX_PGM_SIZE][NB_FU][4];

#define init()                                                                           \
    int i = 0;                                                                           \
    uint8_t r[512];                                                                      \
    for (i = 0; i < 512; i++)                                                            \
        r[i] = i;                                                                        \
    i = 0

#define op0(o, rd, ra, rb)                                                               \
    do {                                                                                 \
        PRGM[i / 2][0][0] = (uint8_t)op::o;                                              \
        PRGM[i / 2][0][1] = rd;                                                          \
        PRGM[i / 2][0][2] = ra;                                                          \
        PRGM[i / 2][0][3] = rb;                                                          \
        i++;                                                                             \
    } while (0)

#define op1(o, rd, ra, rb)                                                               \
    do {                                                                                 \
        PRGM[i / 2][1][0] = (uint8_t)op::o;                                              \
        PRGM[i / 2][1][1] = rd;                                                          \
        PRGM[i / 2][1][2] = ra;                                                          \
        PRGM[i / 2][1][3] = rb;                                                          \
        i++;                                                                             \
    } while (0)

#define halt()                                                                           \
    do {                                                                                 \
        PRGM[i / 2][0][0] = (uint8_t)op::noop;                                           \
        PRGM[i / 2][0][1] = 0;                                                           \
        PRGM[i / 2][0][2] = 0;                                                           \
        PRGM[i / 2][0][3] = 0;                                                           \
        PRGM[i / 2][1][0] = (uint8_t)op::noop;                                           \
        PRGM[i / 2][1][1] = 0;                                                           \
        PRGM[i / 2][1][2] = 0;                                                           \
        PRGM[i / 2][1][3] = 0;                                                           \
        i += 2;                                                                          \
    } while (0)

void init_prgm() {
    init();

#include "prgm.inc"
    // op0(msub, r0, r1, r2);
    // op1(madd, r3, r4, r5);
    // op1(mm, r1, r3, r0);
    // op0(noop, r4, r5, r5);
    // halt();
}

inline int access(int reg_id, int i, int j) { return reg_id * N * N + i * N + j; }

inline void print_reg_file(CMA<half> &reg_file) {
    for (int id = 0; id < REG_SIZ; ++id) {
        std::cout << "reg" << id << ":" << std::endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                std::cout << reg_file[access(id, i, j)] << " ";
            }
            std::cout << std::endl;
        }
    }
}

int main() {
    std::cout << "Starting..." << std::endl;
    // Initialise our data structures
    init_prgm();
    XGeneric_accel generic_accel_obj = {MAP_ADDR, 0};
    XGeneric_accel *generic_accel = &generic_accel_obj;
    int ret = XGeneric_accel_Initialize(generic_accel, "fabric");
    switch (ret) {
    case XST_DEVICE_NOT_FOUND:
        std::cout << "Error: device not found" << std::endl;
        return (EXIT_FAILURE);
    case XST_OPEN_DEVICE_FAILED:
        std::cout << "Error: opening device failed" << std::endl;
        return (EXIT_FAILURE);
    case XST_SUCCESS:
        break;
    default:
        std::cout << "Error: unknown error" << std::endl;
        return (EXIT_FAILURE);
    }

    std::cout << "Init data..." << std::endl;
    // Initialise the channels
    CMA<half> reg_file(REG_SIZ * N * N);
    CMA<half> reg_file_out(REG_SIZ * N * N);
    std::cout << "Allocation done" << std::endl;
    for (int id = 0; id < REG_SIZ; ++id) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                reg_file[access(id, i, j)] = (i == j) ? 1 : 0;
                reg_file_out[access(id, i, j)] = 0;
            }
        }
    }

    std::cout << "Register File = " << reg_file << std::endl;
    std::cout << "Out Register File = " << reg_file_out << std::endl;

    u32 *pgrm_ptr = (u32 *)PRGM;
    XGeneric_accel_Write_pgm_Words(generic_accel, 0, pgrm_ptr,
                                   sizeof(PRGM) / sizeof(u32));
    XGeneric_accel_Set_data_in(generic_accel, reg_file.phy_addr());
    XGeneric_accel_Set_data_out(generic_accel, reg_file_out.phy_addr());

    // Start the computation
    auto t_before = std::chrono::steady_clock::now();
    XGeneric_accel_Start(generic_accel);
    // Wait for the completion of the transfer
    while (!XGeneric_accel_IsDone(generic_accel))
        _pause();
    auto t_after = std::chrono::steady_clock::now();
    int time_elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t_after - t_before).count();

    uint64_t nb_cycles_before = XGeneric_accel_Get_start_time(generic_accel);
    uint64_t nb_cycles_after = XGeneric_accel_Get_end_time(generic_accel);
    uint64_t nb_cycles = nb_cycles_after - nb_cycles_before;

    print_reg_file(reg_file_out);

    std::cout << "Executed for matrix of size ";
    std::cout << N << "x" << N << std::endl;
    std::cout << "Total time elapsed:    ";
    std::cout << time_elapsed_us << " µs" << std::endl;
    std::cout << "Compute time:      ";
    std::cout << nb_cycles;
    std::cout << " cycles" << std::endl;

    XGeneric_accel_Release(generic_accel);

    return EXIT_SUCCESS;
}
