#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <getopt.h>
#include <assert.h>
#define ASSERT(x) assert(x != -1)

#include "configuration.h"
#include "../utils/memory_utils.h"
#include "../utils/misc_utils.h"

////////////////////////////////////////////////////////////////////////////////
// Memory Allocations

uint64_t *shared_mem;
volatile uint64_t *synchronization;
volatile uint64_t *synchronization_params;

////////////////////////////////////////////////////////////////////////////////
// Function declarations

void attacker(int test_option);
void victim();

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  //////////////////////////////////////////////////////////////////////////////
  // Process command line arguments

  int option_index=0;
  while (1) {

    static struct option long_options[] = {
      {"primescope" , no_argument, 0, 'p' },
      {"evset"      , no_argument, 0, 'e' },
      {0            , 0          , 0,  0  }};

    if (getopt_long(argc, argv, "p:e", long_options, &option_index) == -1)
      break;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Memory allocations

  // `shared_mem` is for addresses that the attacker and victim will share.
  // `synchronization*` are variables for communication between threads.

  ASSERT(mem_map_shared(&shared_mem, SHARED_MEM_SIZE, HUGE_PAGES_AVAILABLE));
  ASSERT(var_map_shared(&synchronization));
  ASSERT(var_map_shared(&synchronization_params));

  *shared_mem = 1;
  *synchronization = 0;

  //////////////////////////////////////////////////////////////////////////////
  // Start the threads

  if (fork() == 0) {
    set_core(VICTIM_CORE, "Victim");
    victim();
    return 0;
  }
  
  set_core(ATTACKER_CORE, "Attacker"); 
  attacker(option_index);

  //////////////////////////////////////////////////////////////////////////////
  // Memory de-allocations

  ASSERT(munmap(shared_mem, SHARED_MEM_SIZE));
  ASSERT(var_unmap(synchronization));
  ASSERT(var_unmap(synchronization_params));

  return 0;
}