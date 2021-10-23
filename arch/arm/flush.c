#include "arm/flush.h"

void arm_v8_flush(void *addr) {
    asm volatile ("DC CIVAC, %0" :: "r"(addr));
    asm volatile ("DSB ISH");
    asm volatile ("ISB");
}
