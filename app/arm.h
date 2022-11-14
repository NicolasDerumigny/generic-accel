#ifndef ARM_H
#define ARM_H

#define _pause() do{asm volatile ("yield");} while(0)

#endif // ARM_H
