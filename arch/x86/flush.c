#include "x86/flush.h"

/*
 * Here rax is used as a clobber register.
 */

void x86_flush(void* addr) {
  asm volatile ("clflush 0(%0)"
    :
    : "r" (addr)
    : "rax"
  );
}

