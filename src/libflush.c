#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "eviction.h"
#include "internal.h"
#include "libflush.h"
#include "timing.h"
#include "stdlib.h"

#if defined(__ARM__)
#include "arm/flush.h"
#include "arm/memory.h"
#include "arm/timing.h"
#endif

#if defined(__x86__)
#include "x86/flush.h"
#include "x86/memory.h"
#include "x86/timing.h"
#endif


// Initialize libflush session
bool libflush_init(struct libflush_session_t **session, struct libflush_session_args_t *args) {  
    printf("[libflush]: Initialising session\n");
    if (session == NULL) {
        return false;
    } 
    if ((*session = calloc(1, sizeof(struct libflush_session_t))) == NULL) {
        return false;
    }
    if (args) {
        (*session)->performance_register_div64 = args->performance_register_div64;
    }
#if HAVE_PAGEMAP_ACCESS == 1
    printf("[libflush]: Opening pagemap file\n"); 
    (*session)->memory.pagemap = open("/proc/self/pagemap", O_RDONLY);
    if ((*session)->memory.pagemap == -1) {
        free(*session);
        return false;
    }
    printf("[libflush]: Completed opening pagemap file\n");
#endif
   
#if TIME_SOURCE == TIME_SOURCE_PERF
    perf_init(*session, args);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
    thread_counter_init(*session, args);
#endif
   
    // Initialize eviction
    libflush_eviction_init(*session, args);

    // Initialize arch for ARM.
#if defined(__ARM__)
    arm_v8_timing_init(*session, args);
#endif
    return true;
}

/*
 * Terminate libflush session.
 */

bool libflush_terminate(struct libflush_session_t *session) {
    printf("[libflush]: Terminating libflush session\n");
    if (session == NULL) {
        return false;
    }

#if HAVE_PAGEMAP_ACCESS == 1
    printf("[libflush]: Closing pagemap file\n");
    if (session->memory.pagemap >= 0) {
        close(session->memory.pagemap);
    }
    session->memory.pagemap = -1;
    printf("[libflush]: Completed closing pagemap file\n");
#endif

    /* Terminate timer */
#if TIME_SOURCE == TIME_SOURCE_PERF
    perf_terminate(session);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
    thread_counter_terminate(session);
#endif
    
    // Terminate eviction
    libflush_eviction_terminate(session);

#if defined(__ARM__)
    arm_v8_timing_terminate(session);
#endif

    // Clean up 
    free(session);
    printf("[libflush]: Successfully terminated libflush session\n");
    return true;
}

// Fence instruction

void libflush_memory_barrier() {
#if defined(__ARM__) 
    arm_v8_memory_barrier();
#elif defined(__x86__)
    x86_memory_barrier();
#endif
}


// Timing instruction

uint64_t libflush_get_timing(struct libflush_session_t *session) {
    uint64_t result = 0;
    libflush_memory_barrier(); 
#if TIME_SOURCE == TIME_SOURCE_MONOTONIC_CLOCK
    result = get_monotonic_time();
#elif TIME_SOURCE == TIME_SOURCE_PERF
    result = perf_get_timing(session);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
    result = thread_counter_get_timing(session);
#elif TIME_SOURCE == TIME_SOURCE_REGISTER
    #if defined(__ARM__)
        result = arm_v8_get_timing();
    #elif defined(__x86__)
        result = x86_get_timing();
    #endif
#endif
    libflush_memory_barrier();
    return result;
}

uint64_t libflush_get_timing_start(struct libflush_session_t *session) {
    uint64_t result = 0;
#if defined __x86__
    result = x86_get_timing_start();
#else
    result = libflush_get_timing(session);
#endif
    return result;
}

uint64_t libflush_get_timing_end(struct libflush_session_t *session) {
    uint64_t result = 0;
#if defined __x86__
    result = x86_get_timing_end();
#else
    result = libflush_get_timing(session);
#endif
    return result;
}

void libflush_reset_timing(struct libflush_session_t *session) {
#if TIME_SOURCE == TIME_SOURCE_PERF
    perf_reset_timing(session);
#elif TIME_SOURCE == TIME_SOURCE_REGISTER
    #if defined(__ARM__)
        arm_v8_reset_timing();
    #endif
#endif

    libflush_memory_barrier();
}

// Perform memory access
inline void libflush_access_memory(void *addr) {
#if defined(__ARM__)
    arm_v8_access_memory(addr);
#elif defined(__x86__)
    x86_access_memory(addr);
#endif
}

// Flushing call

inline void libflush_flush(struct libflush_session_t *session, void *addr) {
#if USE_EVICTION == 1
    libflush_eviction_evict(session, addr);
#elif defined(__ARM__)
    arm_v8_flush(addr);
#elif defined(__x86__)
    x86_flush(addr);
#endif
}

// Flush and time
uint64_t libflush_flush_time(struct libflush_session_t *session, void *addr) {
    uint64_t start = libflush_get_timing(session);
    libflush_flush(session, addr);
    return libflush_get_timing(session) - start;
}

// Evict call
void libflush_evict(struct libflush_session_t *session, void *addr) {
    libflush_eviction_evict(session, addr);
}

// Evict time
uint64_t libflush_evict_time(struct libflush_session_t *session, void *addr) {
    uint64_t start = libflush_get_timing(session);
    libflush_eviction_evict(session, addr);
    return libflush_get_timing(session) - start;
}

// Reload address
uint64_t libflush_reload_addr(struct libflush_session_t *session, void *addr) {
    uint64_t start = libflush_get_timing(session);
    libflush_access_memory(addr);
    return libflush_get_timing(session) - start;
}

// Flush - Reload
uint64_t libflush_reload_addr_and_flush(struct libflush_session_t *session, void *addr) {
    uint64_t start = libflush_get_timing_start(session);
    libflush_access_memory(addr);
    uint64_t delta = libflush_get_timing_end(session) - start;
    libflush_flush(session, addr);
    return delta;
}

//Flush - Evict
uint64_t libflush_reload_addr_and_evict(struct libflush_session_t *session, void *addr) {
    uint64_t start = libflush_get_timing_start(session);
    libflush_access_memory(addr);
    uint64_t delta = libflush_get_timing_end(session) - start;
    libflush_evict(session, addr);
    return delta;
}

// Functions to prime probe and calc access time.
void libflush_prime(struct libflush_session_t *session, size_t index) {
    libflush_eviction_prime(session, index);
}

size_t libflush_get_set_index(struct libflush_session_t *session, void *addr) {
    return libflush_eviction_get_set_index(session, addr);
}

size_t libflush_get_number_of_sets(struct libflush_session_t *session, void *addr) {
    return libflush_eviction_get_number_of_sets(session);
}

uint64_t libflush_probe(struct libflush_session_t *session, size_t index) {
    uint64_t tstart = libflush_get_timing_start(session);
    libflush_eviction_probe(session, index);
    uint64_t delta = libflush_get_timing_end(session) - tstart;
    return delta;
}

#if HAVE_PAGEMAP_ACCESS == 1
static size_t get_frame_number_from_pagemap(size_t value) {
    return value & ((1UL << 55) - 1);
}
#endif
    
uintptr_t libflush_get_phy_addr(struct libflush_session_t *session, uintptr_t virt_addr) {
#if HAVE_PAGEMAP_ACCESS == 1
    libflush_access_memory((void *) virt_addr);
    uint64_t value;
    off_t offset = (virt_addr / 4096) * sizeof(value);
    int got = pread(session->memory.pagemap, &value, sizeof(value), offset);
    assert(got == 8); 
    assert(value & (1ULL << 63)); // Page present flag

    uint64_t frame_num = get_frame_number_from_pagemap(value);
    return (frame_num * 4096) | (virt_addr & (4095));
#else
    return 0;
#endif
}


