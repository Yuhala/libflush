#define _GNU_SOURCE
#include <sched.h>
#include <stdbool.h>
#include <unistd.h>

bool bind_thread_to_cpu(size_t cpuid) {
    const pid_t pid = getpid();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpuid, &cpuset); 
    const int res = sched_setaffinity(pid, sizeof(cpuset), &cpuset);
    if (res != 0) {
        return false;
    } else {
        return true;
    }
}

