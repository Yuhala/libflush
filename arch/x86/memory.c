#include "x86/memory.h"


void x86_access_memory(void* pointer) {
  asm volatile ("movq (%0), %%rax\n"
      :
      : "c" (pointer)
      : "rax");
}

/*
 * Use the fence instruction to perform
 * serialization of loads and stores.
 */

void x86_memory_barrier(void) {
  asm volatile ("mfence");
}

void x86_prefetch(void* pointer) {
  asm volatile ("prefetchnta (%0)" :: "r" (pointer));
  asm volatile ("prefetcht2 (%0)" :: "r" (pointer));
}

