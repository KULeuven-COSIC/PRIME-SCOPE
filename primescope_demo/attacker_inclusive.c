#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <assert.h>
#define ASSERT(x) assert(x != -1)

// Consider this file only if the target machine has inclusive caches 
// according to configuration.h
#include "configuration.h"

#ifdef LLC_INCLUSIVE 


#include "prime.h"
#include "../utils/colors.h"
#include "../utils/cache_utils.h"
#include "../utils/memory_utils.h"
#include "../utils/misc_utils.h"

// Evset functions
#include "../evsets/list/list_traverse.h"
#include "../evsets/list/list_utils.h"
#include "../evsets/ps_evset.h"

////////////////////////////////////////////////////////////////////////////////
// Memory Allocations
extern volatile uint64_t *shared_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

////////////////////////////////////////////////////////////////////////////////
// Function declarations

void test_eviction_set_creation();
void test_primescope();

void configure_thresholds(
  uint64_t target_addr, int* thrL1, int* thrLLC, int* thrRAM, int* thrDET);

////////////////////////////////////////////////////////////////////////////////

uint64_t *evict_mem;
void attacker_helper();

void attacker(int test_option) {

  ASSERT(mem_map_shared(&evict_mem, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));

  if (fork() == 0) {
    set_core(HELPER_CORE, "Attacker Helper");
    attacker_helper();
    return;
  }

  if (test_option == 0) test_primescope();
  else                  test_eviction_set_creation();

  ASSERT(munmap(evict_mem,  EVICT_LLC_SIZE));

  // Shut Down
  *synchronization = -1;
  sleep(1);
}

////////////////////////////////////////////////////////////////////////////////

void test_primescope() {

  //////////////////////////////////////////////////////////////////////////////
  // Include the function macros
  #include "macros.h"

  //////////////////////////////////////////////////////////////////////////////
  // Pick a random target_addr from shared_mem

  int seed = time(NULL); srand(seed);
  int target_index = (rand()%1000)*8;
  
  uint64_t target_addr = (uint64_t)&shared_mem[target_index];

  //////////////////////////////////////////////////////////////////////////////
  // Cache Access Thresholds

  int thrL1, thrLLC, thrRAM, thrDET;
  configure_thresholds(target_addr, &thrL1, &thrLLC, &thrRAM, &thrDET);

  printf("\nThresholds Configured\n\n");
  printf("\tL1/L2    : %u\n", thrL1   );
  printf("\tLLC      : %u\n", thrLLC  );
  printf("\tRAM      : %u\n", thrRAM  );
  printf("\tTHRESHOLD: %u\n", thrDET  );

  // Only need helper for clean threshold calibration
  KILL_HELPER(); 

  //////////////////////////////////////////////////////////////////////////////
  // Eviction Set Construction

  #define EV_LLC LLC_WAYS

#if PREMAP_PAGES == 1
  ps_evset_premap(evict_mem);
#endif

  Elem  *evsetList;
  Elem **evsetList_ptr = &evsetList;

  *evsetList_ptr=NULL;

repeat_evset:
  if (PS_SUCCESS != ps_evset( evsetList_ptr,
                              (char*)target_addr,
                              EV_LLC,
                              evict_mem,
                              HUGE_PAGES_AVAILABLE,
                              thrDET))
    goto repeat_evset;

  printf("\nEviction set is constructed successfully\n\n");

  printf("\tEviction set addresses are: "); print_list(evsetList);

  //////////////////////////////////////////////////////////////////////////////
  // Prepare for Test

  // Convert the eviction set link-list to an array
  uint64_t evset[EV_LLC]; list_to_array(evsetList, evset);
  
  // Set its first element as the scope_addr
  uint64_t scope_addr = evset[0];

  //////////////////////////////////////////////////////////////////////////////
  // Prime+Scope (Toy Example)

  printf("\nTesting Prime+Scope\n\n");

  int access_time, success=0;

  for (int t=0; t<TEST_LEN; t++) {

    PRIME(evset);
    TIME_READ_ACCESS(scope_addr);
    TIME_READ_ACCESS(scope_addr);

    // Sanity check: the scope_addr is in low-level cache (L1).

    if (access_time<thrLLC)  
          printf(GREEN"\tSuccess at test %d\n"NC, t);
    else  printf(RED  "\tFailure at test %d\n"NC, t);

    ////////////////////////////////////////////////////////////////////////////

    PRIME(evset);
    TIME_READ_ACCESS(scope_addr);    // SCOPE 
    TIME_READ_ACCESS(scope_addr);    // SCOPE
    VICTIM_READ_ACCESS(target_addr); // Cross-core access to monitored set
    TIME_READ_ACCESS(scope_addr);    // SCOPE detects access

    // The victim access to the target address should evict the scope_addr from all cache levels.

    if (access_time>thrLLC) { 
          success++;
          printf(GREEN"\tSuccess at test %d\n\n"NC, t);}
    else  printf(RED  "\tFailure at test %d\n\n"NC, t);
  }

  printf("\tSuccess %d/%d\n", success, TEST_LEN);
}

void test_eviction_set_creation() {

  #define MAX_RETRY 25

  //////////////////////////////////////////////////////////////////////////////
  // Include the function macros
  #include "macros.h"

  //////////////////////////////////////////////////////////////////////////////
  // Eviction Set Construction

  printf("\nTesting Eviction Set Construction Performance\n\n");

  int attempt_counter;
  struct timespec tstart={0,0}, tend={0,0}; double timespan;

  ////////////////////////////////////////////////////////////////////////////
  // Cache Access Thresholds

  uint64_t target_addr = (uint64_t)&shared_mem[0];
  int thrLLC, thrRAM, thrDET, thrL1;
  configure_thresholds(target_addr, &thrL1, &thrLLC, &thrRAM, &thrDET);

  // Only need helper for clean threshold calibration
  KILL_HELPER(); 

  for (int t=0; t<TEST_LEN; t++) {

    ////////////////////////////////////////////////////////////////////////////
    // Pick a new random target_addr from shared_mem

    int seed = time(NULL); srand(seed);
    int target_index = (rand()%1000)*8;

    target_addr = (uint64_t)&shared_mem[target_index];


    ////////////////////////////////////////////////////////////////////////////
    // Eviction Set Construction

    #define EV_LLC LLC_WAYS

  #if PREMAP_PAGES == 1
    ps_evset_premap(evict_mem);
  #endif

    Elem  *evsetList;
    Elem **evsetList_ptr = &evsetList;

    *evsetList_ptr=NULL;

    attempt_counter = 0;

    clock_gettime(CLOCK_MONOTONIC, &tstart);
  repeat_evset:
    if (PS_SUCCESS != ps_evset( evsetList_ptr,
                                (char*)target_addr,
                                EV_LLC,
                                evict_mem,
                                HUGE_PAGES_AVAILABLE,
                                thrDET)) {
      if (++attempt_counter < MAX_RETRY)
        goto repeat_evset;
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);

    timespan = time_diff_ms(tstart, tend);
    
    if (attempt_counter<MAX_RETRY)
      printf(GREEN"\tSuccess. Constucted with %d retries and in %f ms\n"NC,
        attempt_counter, timespan);
    else
      printf(RED"\tFail. Could not construct with %d retries\n"NC,
        attempt_counter);
  }
}


void configure_thresholds(
  uint64_t target_addr, int* thrL1, int* thrLLC, int* thrRAM, int* thrDET) {

  #define THRESHOLD_TEST_COUNT 1000

  int timing[10][THRESHOLD_TEST_COUNT];
  int access_time;

  #include "macros.h"

  for (int t=0; t<THRESHOLD_TEST_COUNT; t++) {
    FLUSH             (target_addr);
    HELPER_READ_ACCESS(target_addr);
    TIME_READ_ACCESS  (target_addr); timing[0][t] = access_time; // time0: LLC
    FLUSH             (target_addr);
    TIME_READ_ACCESS  (target_addr); timing[1][t] = access_time; // time1: DRAM
    TIME_READ_ACCESS  (target_addr); timing[2][t] = access_time; // time2: L1/L2
  }
  qsort(timing[0], THRESHOLD_TEST_COUNT, sizeof(int), comp);
  qsort(timing[1], THRESHOLD_TEST_COUNT, sizeof(int), comp);
  qsort(timing[2], THRESHOLD_TEST_COUNT, sizeof(int), comp);
  *thrLLC = timing[0][(int)0.10*THRESHOLD_TEST_COUNT];
  *thrRAM = timing[1][(int)0.50*THRESHOLD_TEST_COUNT];
  *thrL1  = timing[2][(int)0.10*THRESHOLD_TEST_COUNT];
  *thrDET = (2*(*thrRAM) + (*thrLLC))/3;
}

#endif