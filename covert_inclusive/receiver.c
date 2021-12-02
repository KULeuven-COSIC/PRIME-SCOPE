#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Main Config file
#include "configuration.h"

// Cache, time and memory utils
#include "../utils/cache_utils.h"
#include"../utils/memory_utils.h"
#include "../utils/misc_utils.h"

// Evset functions
#include "../evsets/list/list_traverse.h"
#include "../evsets/list/list_utils.h"
#include "../evsets/list/list_traverse.h"
#include "../evsets/ps_evset.h"

// Phenomenal prime snippets
#include "../prime.h"

// Covert Channel
#include "covert.h"

/////////////////////////////////////

void configure_thresholds(
  uint64_t target_addr, int* thrL1, int* thrLLC, int* thrRAM, int* thrDET);

// Buffers
uint64_t shared_mem_iova;
extern volatile uint64_t *shared_mem;
extern uint64_t *evict_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;
////////////////////////////////////////////////////////////////////////////////

void receiver() {  

  //////////////////////////////////////////////////////////////////////////////
  // Variables
  int t, m, i, st_i, k, j, mi, it;            // Loop counters, etc.

  #define TEST_COUNT 1000                    // Threshold calibration
  int timing[2000][TEST_COUNT];               // Threshold calibration
  int SW_LLC, SW_RAM, SW_THR, SW_L1;          // Threshold calibration
  int L2[LLC_WAYS], LLC[LLC_WAYS], RAM[LLC_WAYS];   // Threshold calibration

  #define EV_CHECK_MEASUREMENTS 10000         // EV set verification
  int RAM_OUR2[EV_CHECK_MEASUREMENTS];        // EV set verification

  uint64_t tick_start, tick_next;             // Timestamps
  
  int timeSW;                                 // Time measurement

  //////////////////////////////////////////////////////////////////////////////
  // Macros
  #include "macros.h"

  //////////////////////////////////////////////////////////////////////////////
  // Designate TARGET address (corresponding to the agreed cache set)
  uint64_t TARGET = (uint64_t) &shared_mem[CACHE_SET*8];

  //////////////////////////////////////////////////////////////////////////////
  // Cache Access Thresholds

  int thrL1, thrLLC, thrRAM, thrDET;
  configure_thresholds(TARGET, &thrL1, &thrLLC, &thrRAM, &thrDET);

  printf("\n [+] Thresholds Configured\n\n");
  printf("\tL1/L2    : %u\n", thrL1   );
  printf("\tLLC      : %u\n", thrLLC  );
  printf("\tRAM      : %u\n", thrRAM  );
  printf("\tTHRESHOLD: %u\n", thrDET  );

  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  // Eviction Set Construction

#if PREMAP_PAGES == 1
  ps_evset_premap(evict_mem); 
#endif

  Elem  *evsetList;
  Elem **evsetList_ptr = &evsetList;

  *evsetList_ptr=NULL;

repeat_evset:
  if (PS_SUCCESS != ps_evset( evsetList_ptr,
                              (char*)TARGET,
                              LLC_WAYS,
                              evict_mem,
                              HUGE_PAGES_AVAILABLE,
                              thrDET))
    goto repeat_evset;

  printf("\n [+] Eviction set constructed successfully.\n");

  //printf("\tEviction set addresses are: "); print_list(evsetList);

  // Convert the eviction set link-list to an array
  uint64_t evset[LLC_WAYS]; list_to_array(evsetList, evset);


  /////////////////////////////////////
  // Clean up and prepare
  READ_ACCESS(TARGET); FLUSH(TARGET); BUSY_WAIT(); // warm up TLB 
  uint8_t received[MESSAGE_LEN];
  for (i=0; i < MESSAGE_LEN; i++){ received[i] = SYMBOLS; }

  /////////////////////////////////////////////////////////////////////////////
  //                   ACTUAL EXPERIMENT STARTS HERE                         //
  /////////////////////////////////////////////////////////////////////////////

  
  // Handshake, receiver telling it's ready
    // Starting in T minus WINDOW_SYNCH
  *synchronization = 0;
  tick_start = rdtscp64(); tick_next = tick_start;
  *synchronization_params = tick_start; *synchronization = 10;

  // Warm up the cache set right before we start
  WINDOW_NEXT(WINDOW_SYNCH-150*PREPARE);

  for (it=0;it<300;it++){ PRIME_COVERT(); }

  WINDOW_NEXT(150*PREPARE);

  j = SYMBOLS; // Initial value important for first iteration

  for (mi=0; mi<MESSAGE_LEN; mi++) {

    ///////////////////////////
    // ODD WINDOW IS FOR PRIME
      // Prepare EV set
    while(rdtscp64() < tick_next + PREPARE_TRIGGER){ // Be sure to stay within time budget
      PRIME_COVERT();
    }
    WINDOW_NEXT(PREPARE+OFFSET+(SYMBOLS-j)*SLOT);
    ///////////////////////////

    ///////////////////////////
    // EVEN WINDOW IS FOR TRANSMISSION
      // Prime+Scope on evset
      // Transmitter does the access in one of 2^BITS SLOTs,
      //  hence encoding BITS bits in each DATA window.
    for (j=0;j<SYMBOLS;j++){
      SCOPE();
      //printf("HERE\n");
      if (timeSW > PS_THRESH){received[mi] = j; break;}
      WINDOW_NEXT(SLOT);
    }
    ///////////////////////////

  }

  // Write received symbols to file
  FILE *fp; 
  fp = fopen ("./log/cov_A.log", "w");
  for (i=0; i<MESSAGE_LEN; i++) {
    fprintf(fp, "%02u ", received[i]);
  }
  fprintf (fp, "\n"); 
  fclose (fp);

  printf("\n [+] Raw bandwidth: %.2f Mbps [based on hardcoded FREQ = %.0f MHz]\n", (double) ((double) (BITS*(FREQ/MBPS))/(WINDOW_TRANSMIT+PREPARE)), (double) FREQ/MBPS);
  printf("\tBits per window: %u \n", BITS);
  printf("\tWindow for PREPARE: %u cycles \n", PREPARE);
  printf("\tWindow for DATA: %u cycles \n", WINDOW_TRANSMIT);
  printf("\n===========================================\n");

  // Shut Down
terminate: 
  *synchronization = -1; 
  sleep(1); 

}

void configure_thresholds(
  uint64_t target_addr, int* thrL1, int* thrLLC, int* thrRAM, int* thrDET) {

  #define THRESHOLD_TEST_COUNT 1000

  int timing[10][THRESHOLD_TEST_COUNT];
  int access_time;

  #include "macros.h"

  for (int t=0; t<THRESHOLD_TEST_COUNT; t++) {
    FLUSH             (target_addr);
    TX_READ_ACCESS(target_addr);
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
