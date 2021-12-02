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
#include "../../utils/memory_utils.h"
#include "../../utils/misc_utils.h"

uint64_t *shared_mem;
uint64_t *evict_mem;
volatile uint64_t *synchronization;
volatile uint64_t *synchronization_params;

int  attacker();
void victim();

int main(void)
{
  int result;

  #if HUGE_PAGES_AVAILABLE == 1  
    ASSERT(mem_map_shared(&shared_mem, SHARED_MEM_SIZE, HUGE_PAGES_AVAILABLE));
  #else
    ASSERT(mem_map_shared(&shared_mem, 4*KB, HUGE_PAGES_AVAILABLE));
  #endif
  ASSERT(mem_map_private(&evict_mem, EVICT_LLC_SIZE, HUGE_PAGES_AVAILABLE));

  ASSERT(var_map_shared(&synchronization));
  ASSERT(var_map_shared(&synchronization_params));

  *shared_mem = 1;
  *evict_mem = 1;

  printf("\n============================== \n");

  if (fork() == 0) {
    set_core(VICTIM_CORE, "Victim  ");
    victim();
  } 
  else {
    set_core(ATTACKER_CORE, "Attacker");

    result = attacker();

    #if HUGE_PAGES_AVAILABLE == 1  
      ASSERT(mem_unmap(shared_mem, SHARED_MEM_SIZE));
    #else
      ASSERT(mem_unmap(shared_mem, 4*KB));
    #endif
    ASSERT(mem_unmap(evict_mem, EVICT_LLC_SIZE));

    ASSERT(var_unmap(synchronization));
    ASSERT(var_unmap(synchronization_params));
  }
  exit(result);
}
