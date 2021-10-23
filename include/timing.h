#ifndef TIMING_H
#define TIMING_H

#include <stdbool.h>
#include <stdint.h>

#include "libflush.h"

#define TIME_SOURCE_REGISTER 1
#define TIME_SOURCE_PERF 2
#define TIME_SOURCE_MONOTONIC_CLOCK 3
#define TIME_SOURCE_THREAD_COUNTER 4

#if TIME_SOURCE == TIME_SOURCE_MONOTONIC_CLOCK
uint64_t get_monotonic_time();
#endif

#if TIME_SOURCE == TIME_SOURCE_PERF
bool perf_init(struct libflush_session_t *, struct libflush_session_args_t *);
bool perf_terminate(struct libflush_session_t *);
uint64_t perf_get_timing(struct libflush_session_t *);
uint64_t perf_get_timing_start(struct libflush_session_t *);
uint64_t perf_get_timing_end(struct libflush_session_t *);
void perf_reset_timing(struct libflush_session_t *);
#endif

#if TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
bool thread_counter_init(struct libflush_session_t *, struct libflush_session_args_t *);
uint64_t thread_counter_get_timing(struct libflush_session_t *);
bool thread_counter_termintate(struct libflush_session_t *);
#endif

#endif
