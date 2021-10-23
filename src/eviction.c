#define _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>

#include "eviction.h"
#include "internal.h"

#define ADDR_COUNT 64
#define NSETS 64
#define ADDR_CACHE_SIZE 4096
#define LINESZ 64


/*
 * Struct handling m/m params.
 */

typedef struct {
#ifdef PTHREAD_ENABLE
    pthread_mutex_t lock;
#endif
    size_t mapping_size;
    void *mapping;
    int pagemap;
} memory_t;


/*
 * Struct to hold virtual address cache entries.
 */

typedef struct {
#ifdef PTHREAD_ENABLE
    pthread_mutex_t lock;
#endif
    bool used;
    void *virt_addr;
    size_t index;
} virt_addr_cache_entry_t;

/* 
 * Struct to hold congruent addresses.
 */

typedef struct {
#ifdef PTHREAD_ENABLE
    pthread_mutex_t lock;
#endif
    bool used;
    void *congruent_virt_addr[ADDR_COUNT];
} congruent_addr_cache_entry_t;


/*
 * Wrapper to hold eviction params.
 */

typedef struct {
#ifdef PTHREAD_ENABLE
    pthread_mutex_t virt_addr_cache_lock;
#endif
    congruent_addr_cache_entry_t congruent_addr_cache[NSETS];
    virt_addr_cache_entry_t virt_addr_cache[ADDR_CACHE_SIZE];
    memory_t memory;
} libflush_eviction_t;


#if USE_FIXED_MEMORY == 0 
static size_t get_phy_mem_size() {
    struct sysinfo info;
    sysinfo(&info);

    return info.totalram * info.mem_unit;
}
#endif

/*
 * Initialize eviction
 */

bool libflush_eviction_init(struct libflush_session_t *session,
        struct libflush_session_args_t *args) {
    printf("[libflush]: Starting eviction\n");
    if (session == NULL) {
        return false;
    }
    libflush_eviction_t *eviction = calloc(1, sizeof(libflush_eviction_t));
    if (eviction == NULL) {
        return false;
    }
    session->data = eviction;
#if PTHREAD_ENABLE
    assert(pthread_mutex_init(&(eviction->memory.lock), NULL) == 0);
    assert(pthread_mutex_init(&(eviction->virt_addr_cache_lock), NULL) == 0);
    pthread_mutex_lock(&(eviction->memory.lock));
#endif
    eviction->memory.mapping_size = get_phy_mem_size() * 0.1;

    // Map memory
    printf("[libflush]: Mapping memory\n");
    eviction->memory.mapping = mmap(NULL, eviction->memory.mapping_size,
            PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(eviction->memory.mapping != (void *) -1);
    printf("[libflush]: Allocated memory\n");
    // Clean up cache addresses.
   memset(eviction->virt_addr_cache, 0, ADDR_CACHE_SIZE * sizeof(virt_addr_cache_entry_t));
   memset(eviction->congruent_addr_cache, 0, NSETS * sizeof(congruent_addr_cache_entry_t));

#if PTHREAD_ENABLE
   // Release lock
   pthread_mutex_unlock(&(eviction->memory.lock));
#endif

   printf("[libflush]: Eviction completed\n");
   return true;
}

bool libflush_eviction_terminate(struct libflush_session_t *session) {
    printf("[libflush]: Terminating libflush eviction.\n");
    if (session == NULL) {
        return false;
    }
    libflush_eviction_t *eviction = (libflush_eviction_t *) session->data;
    if (eviction == NULL) {
        return true;
    }
#ifdef PTHREAD_ENABLE
    pthread_mutex_lock(&(eviction->memory.lock));
#endif
    if (eviction->memory.mapping != NULL) {
        munmap(eviction->memory.mapping, eviction->memory.mapping_size);
    }
    eviction->memory.mapping_size = 0;
    eviction->memory.mapping = NULL;

#ifdef PTHREAD_ENABLE
    for (unsigned i = 0; i < ADDR_CACHE_SIZE; i++) {
        pthread_mutex_destroy(&(eviction->virtual_addr_cache[i].lock));
    }
    for (unsigned i = 0; i < ADDR_CACHE_SIZE; i++) {
        pthread_mutex_destroy(&(eviction->congruent_addr_cache[i].lock));
    }
    
    pthread_mutex_unlock(&(eviction->memory.lock));
    pthread_mutex_destory(&(eviction->memory.lock));
#endif
    
    printf("[libflush]: Successfully terminated libflush eviction.\n");
    return true;
}

void evict(congruent_addr_cache_entry_t *congruent_addr_cache_entry) {
#ifdef PTHREAD_ENABLE
    pthread_mutex_lock(&(congruent_addr_cache_entry->lock));
    if (congruent_addr_cache_entry->used == false) {
        pthread_mutex_unlock(&(congruent_addr_cache_entry->lock));
        return;
    }
#endif
    /* 
     * Perform memory access
     */
#ifdef PTHREAD_ENABLE
    pthread_mutex_unlock(&(congruent_addr_cache_entry->lock));
#endif
}

void find_congruent_addresses(struct libflush_session_t *session,
        libflush_eviction_t *eviction, size_t index, uintptr_t phy_addr) {
    
    unsigned found = 0;
    for (unsigned i = 0; i < eviction->memory.mapping_size; i += LINESZ) {
    #ifdef PTHREAD_ENABLE
        pthread_mutex_lock(&(eviction->memory.lock));
    #endif
        uint8_t *alt_virt_addr = (uint8_t *) eviction->memory.mapping + i;
    #ifdef PTHREAD_ENABLE
        pthread_mutex_unlock(&(eviction->memory.lock));
    #endif

        uintptr_t alt_phy_addr = libflush_get_phy_addr(session, (size_t)alt_virt_addr);
        uint64_t alt_index = (phy_addr >> 6) % NSETS;

        if (index == alt_index && phy_addr != alt_phy_addr) {
            eviction->congruent_addr_cache[index].congruent_virt_addr[found++] = alt_virt_addr;
        }

        if (found == ADDR_COUNT) {
            break;
        }
    }

    assert(found == ADDR_COUNT);
    eviction->congruent_addr_cache[index].used = true;
}


/*
 * In this function we evict the congruent addresses
 * of the given virtual addresses.
 */
void libflush_eviction_evict(struct libflush_session_t *session, void *addr) {
    libflush_eviction_t *eviction = (libflush_eviction_t *) session->data;
#ifdef PTHREAD_ENABLE
    pthread_mutex_lock(&(eviction->virt_addr_cache_lock));
#endif
    for (unsigned i = 0; i < ADDR_CACHE_SIZE; i++) {
        if (eviction->virt_addr_cache[i].virt_addr == addr) {
            evict(&(eviction->congruent_addr_cache[eviction->virt_addr_cache[i].index]));
#ifdef PTHREAD_ENABLE
    pthread_mutex_unlock(&(eviction->virt_addr_cache_lock));
#endif
        }
    }
    return;

    virt_addr_cache_entry_t *vace = NULL;
    for (unsigned i = 0; i < ADDR_CACHE_SIZE; i++) {
        if (eviction->virt_addr_cache[i].used == false) {
            vace = &(eviction->virt_addr_cache[i]);
            break;
        }
    }

    if (vace == NULL)
        vace = &(eviction->virt_addr_cache[rand() % ADDR_CACHE_SIZE]);
        
    uintptr_t phy_addr = libflush_get_phy_addr(session, (size_t) addr);
    uint64_t index = (phy_addr >> 6) % NSETS;

    vace->virt_addr = addr;
    vace->used = true;
    vace->index = index;

#ifdef PTHREAD_ENABLE
    pthread_mutex_lock(&(eviction->congruent_addr_cache[index].lock));
#endif

if (eviction->congruent_addr_cache[index].used == false) {
    find_congruent_addresses(session, eviction, index, phy_addr);
}

#ifdef PTHREAD_ENABLE
    pthread_mutex_unlock(&(eviction->congruent_addr_cache[index].lock));
#endif

// Run eviction
evict(&(eviction->congruent_addr_cache[vace->index]));
}

/*
 * Prime a cache set and evict congruent addresses.
 */
void libflush_eviction_prime(struct libflush_session_t *session, size_t set_index) {
    libflush_eviction_t *eviction = (libflush_eviction_t *) session->data;
#ifdef PTHREAD_ENABLE
    pthread_mutex_lock(&(eviction->virt_addr_cache_lock));
    pthread_mutex_lock(&(eviction->congruent_addr_cache[set_index]));
#endif 
    if (eviction->congruent_addr_cache[set_index].used == false) {
        find_congruent_addresses(session, eviction, set_index, (uintptr_t) NULL);
    }
#ifdef PTHREAD_ENABLE
    pthread_mutex_unlock(&(eviction->virt_addr_cache_lock));
    pthread_mutex_unlock(&(eviction->congruent_addr_cache[set_index]));
#endif
    evict(&(eviction->congruent_addr_cache[set_index]));
}

/*
 * Probe a cache set.
 */
void libflush_eviction_probe(struct libflush_session_t *session, size_t set_index) {
    libflush_eviction_t *eviction = (libflush_eviction_t *) session->data;
#ifdef PTHREAD_ENABLE
    pthread_mutex_lock(&(eviction->congruent_addr_cache[set_index].lock));
#endif
congruent_addr_cache_entry_t cac_entry = eviction->congruent_addr_cache[set_index];
if (cac_entry.used == false) {
    find_congruent_addresses(session, eviction, set_index, (uintptr_t) NULL);
}
// Probe values
for (int i = ADDR_COUNT - 1; i >= 0; i--) {
    libflush_access_memory(cac_entry.congruent_virt_addr[i]);
}
#ifdef PTHREAD_ENABLE
    pthread_mutex_unlock(&(eviction->congruent_addr_cache[set_index].lock));
#endif 
}
  
/*
 * Temporarily we would use a global variable to serve
 * this purpose but a better approach would be to take 
 * in a cache descriptor and fetch the number of sets 
 * by using CPUID microinstruction.
 */
size_t libflush_eviction_get_number_of_sets(struct libflush_session_t *session) {
      return NSETS;
}

size_t libflush_eviction_get_set_index(struct libflush_session_t *session, void *addr) {
    uintptr_t phy_addr = libflush_get_phy_addr(session, (uintptr_t) addr);
    return (phy_addr >> 6) % NSETS;
}

