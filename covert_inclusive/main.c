#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sched.h>


#include <assert.h>
#define ASSERT(x) assert(x != -1)

#include "configuration.h"

// Utils
#include "../utils/memory_utils.h"
#include "../utils/misc_utils.h"

uint64_t *shared_mem;
uint64_t *evict_mem;
volatile uint64_t *synchronization;
volatile uint64_t *synchronization_params;

void receiver   ();
void transmitter     ();

int main(void)
{
  ASSERT(mem_map_shared(&shared_mem, SHARED_MEM_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(mem_map_private(&evict_mem, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(var_map_shared(&synchronization));
  ASSERT(var_map_shared(&synchronization_params));

  *shared_mem = 1;
  *evict_mem = 1;
  *synchronization = 0;

  printf("\n===========================================");
  printf("\n         Covert Channel on the LLC");
  printf("\n===========================================\n");

  if (fork() == 0) {
    set_core(TRANSMITTER_CORE, "Transmitter");
    transmitter();
  } 
  else {
    set_core(RECEIVER_CORE, "Receiver   ");

    receiver();

    ASSERT(munmap((void*)shared_mem, SHARED_MEM_SIZE));
    ASSERT(munmap((void*)evict_mem, EVICT_LLC_SIZE ));
    ASSERT(var_unmap(synchronization));
    ASSERT(var_unmap(synchronization_params));
  }
  return 0;
}
