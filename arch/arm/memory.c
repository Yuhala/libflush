#include "arm/memory.h"

#include <inttypes.h>


void arm_v8_access_memory(void* pointer)
{
  volatile uint32_t value;
  asm volatile ("LDR %0, [%1]\n\t"
      : "=r" (value)
      : "r" (pointer)
      );
}

void arm_v8_memory_barrier() {
    asm volatile ("DSB SY");
    asm volatile ("ISB");
}

void arm_v8_prefetch(void* pointer)
{
  asm volatile ("PRFM PLDL3KEEP, [%x0]" :: "p" (pointer));
  asm volatile ("PRFM PLDL2KEEP, [%x0]" :: "p" (pointer));
  asm volatile ("PRFM PLDL1KEEP, [%x0]" :: "p" (pointer));
}
