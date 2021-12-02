#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "configuration.h" // Experiment-Specific Macros
#include "../../utils/cache_utils.h"   // Cache Snippets

#include "macros.h"

// Buffers
extern uint64_t *shared_mem;
extern uint64_t *evict_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

void victim() {

  int i,j,mi;

  //////////////////////////////////////////////////////////////////////////////

  while(1) {

    if (*synchronization == -1) {
      break;
    }
    else if (*synchronization == 1) {
      /* Implements VICTIM_READ() in macros.h */
      memread((void*)*synchronization_params);
      // End
      asm volatile("lfence\nmfence\n");
      *synchronization = 0;
    }
    else if (*synchronization == 2) {
      /* Implements VICTIM_MEASURE() in macros.h */
      *synchronization_params = time_mread((void*)*synchronization_params); 
      FENCE
      *synchronization = 0;
    }
  }

}