#ifndef ARM_V8_TIMING_H
#define ARM_V8_TIMING_H


#include <stdint.h>
#include "memory.h"


uint64_t arm_v8_get_timing();
void arm_v8_timing_init();
void arm_v8_timing_terminate();
void arm_v8_reset_timing();

#endif
