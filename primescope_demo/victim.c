#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../utils/cache_utils.h"

extern volatile uint64_t *shared_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

void victim() {

  //////////////////////////////////////////////////////////////////////////////
  // Prepare variables for test cache access times

  #define FENCE asm volatile ("mfence\n\t lfence\n\t");

  //////////////////////////////////////////////////////////////////////////////

  while(1) {

    if (*synchronization == -1) {
      break;
    }
    else if (*synchronization == 11) {
      /* Implements the VICTIM_READ_ACCESS() macro */
      memread((void*)*synchronization_params); 
      FENCE
      *synchronization = 0;
    }
  }

  exit(EXIT_SUCCESS);
}