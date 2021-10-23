#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>

#include "internal.h"
#include "timing.h"
#include "utils.h"

#if TIME_SOURCE == TIME_SOURCE_MONOTONIC_CLOCK
#include <time.h>

uint64_t get_monotonic_time() {
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
}
#endif

#if TIME_SOURCE == TIME_SOURCE_PERF
#include <linux/perf_event.h>
#include <assert.h>
#include <unistd.h>
#include <stropts.h>
#include <sys/syscall.h>

bool perf_init(struct libflush_session_t *session, struct libflush_session_args_t *args) {
   printf("[libflush]: Initialision of perf\n");
   static struct perf_event_attr attr;
   attr.type = PERF_TYPE_HARDWARE;
   attr.config = PERF_COUNT_HW_CPU_CYCLES;
   attr.size = sizeof(attr);
   attr.exclude_kernel = 1;
   attr.exclude_hv = 1;
   attr.exclude_callchain_kernel = 1;

   session->perf.fd = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
   assert(session->perf.fd >= 0); // Perf events are supported.
   printf("[libflush]: Completed initialisation of perf\n");
   return true;
}

bool perf_terminate(struct libflush_session_t *session) {
    printf("[libflush]: Terminating perf\n");
    close(session->perf.fd);
    printf("[libflush]: Perf has been closed\n");
    return true;
}

uint64_t perf_get_timing(struct libflush_session_t *session) {
    //printf("[libflush]: Perf fetching time\n");
    uint64_t result = 0;
    if (read(session->perf.fd, &result, sizeof(result)) < (ssize_t) sizeof(result)) {
        return 0;
    }
    return result;
}

void perf_reset_timing(struct libflush_session_t *session) {
    printf("[libflush]: Perf resetting time\n");
    ioctl(session->perf.fd, PERF_EVENT_IOC_RESET, 0);
}
#endif

#if TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
#include <pthread.h>
#include "libflush.h"


static void *thread_counter_func(void *);

bool thread_counter_init(struct libflush_session_t *session, struct libflush_session_args_t *args) {
    if (session == NULL) {
        return false;
    }

    session->thread_counter.data.cpu = (args != NULL) ? (ssize_t) args->bind_to_cpu : -1;
    session->thread_counter.data.session = session;

    if (pthread_create(&(session->thread_counter.thread), NULL,
                thread_counter_func, &(session->thread_counter.data)) != 0) {
        return false;
    }

    return true;
}

bool thread_counter_terminate(struct libflush_session_t *session) {
    if (session == NULL)
        return false;
    
    pthread_cancel(session->thread_counter.thread);
    pthread_join(session->thread_counter.thread, NULL);

    return true;
}

static void *thread_counter_func(void *data) {
    thread_data_t *thread_data = (thread_data_t *) data;
    struct libflush_session_t *session = thread_data->session;
    ssize_t cpu = thread_data->cpu;
    
    /*
     * This is to prevent the thread from bouncing.
     * Normally, a scheduler will try to avoid bouncing the thread,
     * but we will pin the thread using sched_setaffinity.
     */

    if (cpu > 0) {
        if (bind_thread_to_cpu(cpu) == false) {
            fprintf(stderr, "Could not bind thread to CPU %zu\n", cpu);
        } else {
            fprintf(stdout, "Thread bound to CPU %zu\n", cpu);
        }
    } 
}

uint64_t thread_counter_get_timing(struct libflush_session_t *session) {
    libflush_memory_barrier(session);
    uint64_t time = session->thread_counter.value;
    libflush_memory_barrier(session);
    return time;
}

#endif

