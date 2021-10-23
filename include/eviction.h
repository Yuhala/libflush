#ifndef EVICTION_H
#define EVICTION_H

#include "libflush.h"
#include <stdbool.h>

/*
 * Init eviction process.
 */

bool libflush_eviction_init(struct libflush_session_t *, struct libflush_session_args_t *);


/*
 * Terminate eviction process.
 */

bool libflush_eviction_terminate(struct libflush_session_t *);


/*
 * Evict an address.
 */

void libflush_eviction_evict(struct libflush_session_t *, void *);

/*
 * Prime a cache set.
 */

void libflush_eviction_prime(struct libflush_session_t *, size_t);

/*
 * Probe a cache set.
 */

void libflush_eviction_probe(struct libflush_session_t *, size_t);

/*
 * Get number of cache sets.
 */

size_t libflush_eviction_get_number_of_sets(struct libflush_session_t *);

/*
 * Get pagemap entry.
 */

uint64_t libflush_get_pagemap_entry(struct libflush_session_t *, uint64_t);

size_t libflush_eviction_get_set_index(struct libflush_session_t *, void *); 
 

#endif
