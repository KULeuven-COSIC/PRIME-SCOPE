#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include "configuration.h"             // Experiment-Specific Macros
#include "../../utils/cache_utils.h"   // Cache Snippets

// Evset functions
#include "../../evsets/ps_evset.h"
#include "../../evsets/list/list_traverse.h"
#include "../../evsets/list/list_utils.h"

#include "prime.h"
struct prime_config {
    char name[20];
    int rounds;
  void (*traverse)(uint64_t*);
} conff;

////////////////////////////////////////////////////////////////////////////////
// Buffers

extern uint64_t *shared_mem;
extern uint64_t *evict_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

////////////////////////////////////////////////////////////////////////////////

#include "params.h" // Contains number of test iterations
#define EV_CHECK_MEASUREMENTS 10000
#define TEST_COUNT 10000
#define EV_LLC (LLC_WAYS)

////////////////////////////////////////////////////////////////////////////////

void test_pattern(Elem** evset, int target_index, struct prime_config conff, int threshold, int threshold_llc, FILE *fp) {

  #include "macros.h"

  uint64_t TARGET = (uint64_t) &shared_mem[target_index]; 

  uint64_t evset_array[LLC_WAYS];
  list_to_array(*evset, evset_array);

  int it, r; 
  size_t timeF, timeSW, timeVictim;

  uint16_t PRIME_TIME_V  [PRIME_ITERATIONS];
  uint16_t PRIME_TIME_A1 [PRIME_ITERATIONS];
  uint16_t PRIME_TIME_A2 [PRIME_ITERATIONS];
  int PRIME_TIMESPAN[PRIME_ITERATIONS];

  for (it=0; it<PRIME_ITERATIONS; it++) {

    Elem* EVC = *evset;

    // Perform a PRIME pattern
    asm volatile("lfence\nmfence\n");
    timeF = rdtsc();

    for(r=0;r<conff.rounds;r++)
      conff.traverse(evset_array);

    PRIME_TIMESPAN[it] = rdtsc()-timeF;
    asm volatile("lfence\nmfence\n");

    // Determine whether the scope line (line 0) is in L1
    ACCESS_EVC();
    PRIME_TIME_A1[it] = timeSW;

    // Determine whether it has evicted victim line
    asm volatile("lfence\nmfence\n");
    VICTIM_MEASURE(TARGET);
    PRIME_TIME_V[it] = timeVictim;

    // Determine whether the scope line (line 0) is the eviction candidate in the LLC
    asm volatile("lfence\nmfence\n");
    ACCESS_EVC();
    PRIME_TIME_A2[it] = timeSW;
  }

  int evicted     = 0;
  int evc_correct = 0;
  for (it=0; it < PRIME_ITERATIONS; it++) {
    if (PRIME_TIME_V[it]   > threshold) 
      evicted++;

    if ((PRIME_TIME_A2[it] > threshold) & 
        (PRIME_TIME_A1[it] < threshold_llc)) 
      evc_correct++;
  }

  double EV  = (double) 100*(double)evicted    /(double)PRIME_ITERATIONS;
  double EVC = (double) 100*(double)evc_correct/(double)PRIME_ITERATIONS;

  printf("%s %u | ", conff.name, conff.rounds);

  if      (EV  > 99.7) printf(GREEN "EV : %7.3f%%  "NC   , EV );
  else if (EV  > 90.0) printf(ORANGE"EV : %7.3f%%  "NC   , EV );
  else                 printf(RED   "EV : %7.3f%%  "NC   , EV );

  if      (EVC > 99.4) printf(GREEN "EVC: %7.3f%% "NC"| ", EVC);
  else if (EVC > 90.0) printf(ORANGE"EVC: %7.3f%% "NC"| ", EVC);
  else                 printf(RED   "EVC: %7.3f%% "NC"| ", EVC);

  qsort(PRIME_TIMESPAN, PRIME_ITERATIONS, sizeof(int), compare);

  printf("Duration: %5u - %5u - %5u cycles (1%% - 50%% - 99%%)\n",
    PRIME_TIMESPAN[(int) (0.01*(double)PRIME_ITERATIONS)],
    PRIME_TIMESPAN[(int) (0.50*(double)PRIME_ITERATIONS)],
    PRIME_TIMESPAN[(int) (0.99*(double)PRIME_ITERATIONS)]
  );

  fprintf(fp, "%s %u %07.3f %07.3f %5u %5u %5u\n", 
    conff.name,
    conff.rounds, 
    EV, 
    EVC, 
    PRIME_TIMESPAN[(int) (0.01*(double)PRIME_ITERATIONS)],
    PRIME_TIMESPAN[(int) (0.50*(double)PRIME_ITERATIONS)],
    PRIME_TIMESPAN[(int) (0.99*(double)PRIME_ITERATIONS)]
  ); 
}

int attacker() {  

  //////////////////////////////////////////////////////////////////////////////
  // Variables

  int i,j,t,m,it;                        // Loop counters, etc.
  int SW_LLC, SW_RAM, SW_THR, SW_L1;     // Threshold config
  int seed = time (NULL); srand (seed);  // Seed PRNG
  int timeSW, timeVictim;                // Time measurements 
  int timing[3][TEST_COUNT];
  
  int attempt;
  #define MAX_REPEAT 64
  
  int target_index = 0;
  uint64_t TARGET  = (uint64_t) &shared_mem[target_index];

  Elem*  evset; 
  Elem** evset_ptr = &evset;      // Eviction set 
  
  //////////////////////////////////////////////////////////////////////////////
  // Macros
  #include "macros.h"

  //////////////////////////////////////////////////////////////////////////////
  // Platform-Specific Stuff
  PRINT_PLATFORM();       // Print Platform
  CONFIGURE_THRESHOLDS(); // Determine Thresholds dynamically
  printf(GREEN"\n [+] Thresholds Configured"NC
              "\n     SW_LLC: %u"
              "\n     SW_RAM: %u"
              "\n     SW_THR: %u\n",
    SW_LLC, SW_RAM, SW_THR);

  //////////////////////////////////////////////////////////////////////////////
  // Construct Eviction Set
  
  printf(GREEN"\n [+] EV Set Construction\n"NC);

  int EV_CHECK[EV_CHECK_MEASUREMENTS];          

  #if PREMAP_PAGES == 1 
  for (i=0;i<EVICT_LLC_SIZE/(8);i+=128){
    evict_mem[i] = 0x1;
  }
  for (i=0;i<EVICT_LLC_SIZE/(8);i+=128){
    evict_mem[i] = 0x0;
  }
  #endif
  
  attempt = 0;
try_again:

  target_index = (rand()%100) * 8;
  TARGET  = (uint64_t) &shared_mem[target_index];
  CONFIGURE_THRESHOLDS(); // Determine Thresholds dynamically

  if (++attempt==MAX_REPEAT)
    goto fail;

  // Construct eviction set
  i = ps_evset(evset_ptr, (char*)TARGET, LLC_WAYS, evict_mem, HUGE_PAGES_AVAILABLE, SW_THR);
  if (i==1) {
    printf("     EvSet Construction: "GREEN"Successful\n"NC);
  }
  else {
    printf("     EvSet Construction: "RED"Fail"NC" %d\n", i);
    goto try_again;
  }

  // Test the eviction set. If fails, re-construct
  if (ps_evset_test(evset_ptr, (char*)TARGET, SW_THR, 100, EVTEST_MEDIAN))
    printf("     EvSet Test: "GREEN"Successful"NC" - List Length: %d/%d\n", list_length(evset), LLC_WAYS);
  else {
    printf("     EvSet Test: "RED"Fail"NC" - Try Again\n"NC);
    goto try_again;
  }
  
  // Test whether eviction set is good enough. If fails, re-construct
  for (it = 0; it < EV_CHECK_MEASUREMENTS; it++) {
    FLUSH(TARGET); 
    READ_ACCESS(TARGET);
    for(i=0; i<3; i++) 
      traverse_list_asm_skylake(*evset_ptr);
    TIME_READ_ACCESS(TARGET); 
    EV_CHECK[it] = timeSW;
  }
  qsort(EV_CHECK,  EV_CHECK_MEASUREMENTS, sizeof(int), compare);

  if (EV_CHECK[(int) (0.95*EV_CHECK_MEASUREMENTS)] < SW_THR) {
    printf("     EvSet Test: "RED"Too weak - Try Again\n");
    goto try_again;
  } 
  else if (list_length(evset) > LLC_WAYS) {
    printf("     EvSet Test: "RED"Too Large - Try Again\n");
    goto try_again;
  }
  printf("     EvSet Test: "GREEN"Success\n"NC);

  //////////////////////////////////////////////////////////////////////////////
  // Prepare
  
  // Warm up TLB
  TIME_READ_ACCESS(TARGET); 
  FLUSH(TARGET); 
  BUSY_WAIT(); 

  //////////////////////////////////////////////////////////////////////////////
  // Experiment Starts Here

  printf(GREEN"\n [+] Experiment Starts\n"NC);

  FILE *file_measurements;

  file_measurements = fopen("../log/output.txt", "w");
  if (file_measurements == NULL)
    printf("Error: fopen\n");

  struct prime_config config;
  #include "primeaccess.h"

  fclose(file_measurements);

  //////////////////////////////////////////////////////////////////////////////
  // Close Up
  *synchronization = -1; 
  sleep(1); 
  return 1;

fail:
  *synchronization = -1; 
  sleep(1); 
  return 0;
}
