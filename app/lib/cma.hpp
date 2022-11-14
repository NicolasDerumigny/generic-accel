#ifndef LIBCMA_H
#define LIBCMA_H

#ifdef __cplusplus
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
extern "C" {
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// kernel buffer pool
#define XLNK_BUFPOOL_SIZE 100

#define XLNK_DRIVER_PATH "/dev/xlnk"

/*
 * Get the virtual address referencing the physical address resulting from
 * mapping /dev/mem.
 * Required to use bare-metal drivers on linux. Return -1 in case of error.
 */
unsigned long cma_mmap(unsigned long phyAddr, uint32_t len);
/*
 * Unmap a previously mapped memory space.
 */
uint32_t cma_munmap(void *buf, uint32_t len);
/*
 * Allocate a physically contiguous chunk of CMA memory and map it into
 * virtual memory space. Return this Virtual pointer. Returns -1 on failure.
 */
void *cma_alloc(uint32_t len, uint32_t cacheable);
/*
 * Return a physical memory address corresponding to a given Virtual address
 * pointer. Returns NULL on failure.
 */
unsigned long cma_get_phy_addr(void *buf);
/*
 * Free a previously allocated CMA memory chunk.
 */
void cma_free(void *buf);
/*
 * Returns the number of available CMA memory pages which can be allocated.
 */
uint32_t cma_pages_available();
/*
 * Extra functions in case user needs to flush or invalidate Cache.
 */
void cma_flush_cache(void *buf, unsigned int phys_addr, int size);
void cma_invalidate_cache(void *buf, unsigned int phys_addr, int size);


#ifdef __cplusplus
} // extern "C"

// Simple wrapper for the C interface
template <typename T>
class CMA {
    void     *_buf;
    uint32_t _size;

public:
    CMA(uint32_t len, bool cacheable = false) {
        _size = len*sizeof(T);
        _buf = cma_alloc(_size, (int) cacheable);
        if (_buf == NULL) {
            std::cerr<<"[ERROR] CMA: Could not allocate ";
            std::cerr<<_size<<" B"<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
    ~CMA() { cma_free(_buf); }

    // Behave like a regular pointer/array
    T operator*=(const CMA<T> rhs) { return (T*) _buf; }
    T& operator[](std::size_t idx) { return ((T*)_buf)[idx]; }

    // Pretty printing the addresses
    friend std::ostream& operator<<(std::ostream& os, CMA<T>& cma) {
        os<<(void*) (cma._buf)<<"/"<<(void *) (cma.phy_addr());
        return os;
    }

    uintptr_t phy_addr()    { return cma_get_phy_addr(_buf); }
    uint32_t  size()        { return _size;}
    void flush_cache()      { cma_flush_cache(_buf, this->phy_addr(), _size); }
    void invalidate_cache() { cma_invalidate_cache(_buf, this->phy_addr(), _size); }
};
#endif

#endif // LIBCMA_H
