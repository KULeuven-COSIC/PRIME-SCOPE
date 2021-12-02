#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "covert.h"
#include "../utils/cache_utils.h"

uint64_t *shared_mem;
uint64_t *evict_mem;
extern volatile uint64_t *synchronization;
extern volatile uint64_t *synchronization_params;

void transmitter() {

  //////////////////////////////////////////////////////////////////////////////
  // Prepare variables for test cache access times

  int i,j,mi;
  int seed = time (NULL); srand (seed);
  uint64_t tick_start, tick_next;
  uint8_t symbol;
  uint8_t message[MESSAGE_LEN];

  // Generate random symbols
  for(i=0;i<MESSAGE_LEN;i++){
    message[i] = rand() & (SYMBOLS-1); 
  }

  // TARGET address
  uint64_t TARGET = (uint64_t) &shared_mem[CACHE_SET*8];
  memread((void*) TARGET); flush((void*) TARGET);

  *synchronization = 2; // Indicate that preparation on this side is done

  //////////////////////////////////////////////////////////////////////////////

  while(1) {

    if (*synchronization == -1) {
      break;
    }
    else if (*synchronization == 1) {
      // Implements "TX_READ_ACCESS" functionality in macros.h
      memread((void*)*synchronization_params); 
      asm volatile("lfence\nmfence\n");
      *synchronization = 0;
    }
    else if (*synchronization == 10) {

      // Handshake
      tick_start = *synchronization_params; 
      tick_next = tick_start;
      *synchronization = 0;
      WINDOW_NEXT(WINDOW_SYNCH);

      for (mi=0; mi<MESSAGE_LEN; mi++) {

        ///////////////////////////
        // ODD WINDOW IS FOR PRIME
          // Do nothing, just unpack the message
        symbol = message[mi];
        WINDOW_NEXT(PREPARE);
        ///////////////////////////

        ///////////////////////////
        // EVEN WINDOW IS FOR TRANSMISSION
        for (i=0;i<SYMBOLS;i++){
            if (i == symbol) {

              #if TRANSMITTER_FENCE == 1
                asm volatile("lfence\n");
              #endif

              memread((void*) TARGET);

              #if TRANSMITTER_FLUSH == 1
                asm volatile("mfence\n");
                flush_nofence((void*) TARGET);
              #endif
            }
          WINDOW_NEXT(SLOT);
        }
        WINDOW_NEXT(PREPARE_LEFTOVER);
      }
    }
  }

  // Write transmitted symbols to file
  FILE * fp; fp = fopen ("./log/cov_V.log","w");
  for (i=0; i<MESSAGE_LEN; i++) {
    fprintf (fp, "%02u ", message[i]);
  }
  fprintf (fp, "\n");
  fclose (fp);

}