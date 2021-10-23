#ifndef LIBFLUSH_H
#define LIBFLUSH_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


/* 
 * Anonymous struct to handle libflush session params.
 */

struct libflush_session_t; 

/*
 * Parameters for the session.
 */

struct libflush_session_args_t {
    size_t bind_to_cpu;
    bool performance_register_div64;
};

/*
 * Initialize a libfush session.
 */

bool libflush_init(struct libflush_session_t **, struct libflush_session_args_t *);

/*
 * Terminate the libflush session.
 */

bool libflush_terminate(struct libflush_session_t *);

/*
 * Get time measurement.
 */

uint64_t libflush_get_timing(struct libflush_session_t *);

// Measure start time
uint64_t libflush_get_timing_start(struct libflush_session_t *);

// Measure end time
uint64_t libflush_get_timing_end(struct libflush_session_t *);

/*
 * Reset the timer.
 */

void libflush_reset_timing(struct libflush_session_t *);

/*
 * Flush the given address.
 */

void libflush_flush(struct libflush_session_t *, void *);

/*
 * Measure time taken to flush the given address.
 */

uint64_t libflush_flush_time(struct libflush_session_t *, void *);

/*
 * Evicts the given address.
 * 
 * :param1: session, The used session
 * :param2: address, The address to evict
 */

void libflush_evict(struct libflush_session_t *, void *);

/*
 * Measure time taken to evict.
 *
 * :param1: session, The used session
 * :param2: address, The address to evict
 *
 * :returns: Timing measurement
 */

uint64_t libflush_evict_time(struct libflush_session_t *, void *);

/*
 * Access memory location.
 */

void libflush_access_memory(void *);

/*
 * Reload the given mem address and calculate 
 * access time.
 */

uint64_t libflush_reload_addr(struct libflush_session_t *, void *);

/*
 * Measure time taken to access the given address, then it
 * is flushed to memory
 */

uint64_t libflush_reload_addr_and_flush(struct libflush_session_t *, void *);

// Reload and evict, similar to above
uint64_t libflush_reload_addr_and_evict(struct libflush_session_t *, void *);

/*
 * Call the fence instruction
 */

void libflush_memory_barrier();

/*
 * Prime a given cache set.
 */

void libflush_prime(struct libflush_session_t *, size_t);

/*
 * Probe a given cache set.
 */

uint64_t libflush_probe(struct libflush_session_t *, size_t);

/*
 * Get set index of a given address.
 */

size_t libflush_get_set_index(struct libflush_session_t *, void *);

/*
 * Returns the number of sets.
 */

size_t libflush_get_number_of_sets(struct libflush_session_t *, void *);


/*
 * Get the corresponding physical address of a virtual address.
 */

uintptr_t libflush_get_phy_addr(struct libflush_session_t *, uintptr_t);

/*
 * Get pagemap entry of a virtual address.
 */

uint64_t libflush_get_pagemap_entry(struct libflush_session_t *, uint64_t);

/*
 * Bind process to CPU.
 */

bool libflush_bind_to_cpu(size_t);


#endif
