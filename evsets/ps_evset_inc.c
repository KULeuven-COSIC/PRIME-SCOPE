#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "ps_evset.h"
#include "list/list_utils.h"

#include "../utils/cache_utils.h"

#ifdef LLC_INCLUSIVE

////////////////////////////////////////////////////////////////////////////////
// Declaration of functions

int ps_evset_reduce(Elem **evset, char *victim, int len, int threshold);

////////////////////////////////////////////////////////////////////////////////
// Definition of functions

int
ps_evset(Elem **evset, char *victim, int len, uint64_t* page, int is_huge, int threshold)
{
  //int is_list_empty = 1; // append works the same for empty list, no special treatment needed
  int list_len = 0;
  int len_requested = len;
  int guess_index=0;
  int counter_attempt = 0;
  int i;
  int counter_guess, try_guesses, time;

  Elem *evset_last = NULL;
  *evset = NULL;
  
  //////////////////////////////////////////////////////////////////////////////
  // Create a guess pool

  uint64_t guess_pool[MAX_POOL_SIZE];
  
  int pool_size = (is_huge) ? MAX_POOL_SIZE_HUGE : MAX_POOL_SIZE_SMALL;

  #if RANDOMIZE_GUESS_POOL == 0
  for (i=0; i<pool_size; i++)  {
    guess_pool[i] = (is_huge) ?
      ((uint64_t)page + ((uint64_t)victim & (LLC_PERIOD-1      )) + i*LLC_PERIOD      ): 
      ((uint64_t)page + ((uint64_t)victim & (SMALLPAGE_PERIOD-1)) + i*SMALLPAGE_PERIOD);
  }
  #else
  // Potential improvement: randomization could be better (e.g., to avoid duplicates)
  for (i=0; i<pool_size; i++)  {
    guess_pool[i] = (is_huge) ?
      ((uint64_t)page + ((uint64_t)victim & (LLC_PERIOD-1      )) + (rand() % MAX_POOL_SIZE_HUGE )*LLC_PERIOD      ): 
      ((uint64_t)page + ((uint64_t)victim & (SMALLPAGE_PERIOD-1)) + (rand() % MAX_POOL_SIZE_SMALL)*SMALLPAGE_PERIOD);
  }
  #endif

  
  //////////////////////////////////////////////////////////////////////////////
  // Start finding eviction set


extend:
  while (list_len<len) {
    counter_guess = 0;
    try_guesses = true;

    // Place TARGET in LLC
    maccess((void*)victim);
    asm volatile("lfence;mfence");

    #if ENABLE_ALREADY_FOUND
    // Place ALREADY_FOUND in LLC
    traverse_zigzag_victim(*evset, (void*)victim); 
    traverse_zigzag_victim(*evset, (void*)victim); 
    #endif

    // Search 
    while (try_guesses) {


      // Access guess
      maccess((void*)guess_pool[guess_index]);
      asm volatile("lfence");

      // Measure TARGET
      time = time_mread((void*)victim);
    
      // If TARGET is evicted
      #if IGNORE_VERY_SLOW == 1
      if (time>threshold-20 && time < 2*threshold) { 
      #else
      if (time>threshold-20) { 
      #endif
        try_guesses = false;
        counter_attempt = 0;

        // Add the guess to linkedlist
        evset_last = (Elem*)guess_pool[guess_index];
        list_len = list_append(evset, evset_last);
        
        // Potential improvement: linked list for easy removal
        for (int i=guess_index; i<pool_size-1; i++){
          guess_pool[i] = guess_pool[i+1];
        }
        pool_size--;
      }
      else {
        guess_index++;
      }
    
      // Wrap around the pool
      if (guess_index>=pool_size-1) {
        guess_index = 0;
        try_guesses = false; // reinstate victim to be sure
        if (++counter_attempt>=MAX_ATTEMPT){ // If too many wrap-arounds, return with fail
          return PS_FAIL_CONSTRUCTION;
        }
      }
    }
  }

    #if ENABLE_EXTENSION 
    // If list of minimal size cannot evict victim, extend it
    if (!ps_evset_test(evset, victim, threshold, 10, EVTEST_MEDIAN)) { 
      if (++len<MAX_EXTENSION)
        goto extend; // Obtain one more address
      return PS_FAIL_EXTENSION;
    }
    #endif

    #if ENABLE_REDUCTION
    // If list needed to be extended, reduce it to minimal
    if (list_len > len_requested){
      if (!ps_evset_reduce(evset, victim, len_requested, threshold)){
        return PS_FAIL_REDUCTION;
      }
    }
    #endif

  return PS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

int
ps_evset_premap(uint64_t* page) {
  int i;
  for (i=0; i<EVICT_LLC_SIZE/(8); i+=128)
    page[i] = 0x1;

  for (i=0; i<EVICT_LLC_SIZE/(8); i+=128)
    page[i] = 0x0;
}

////////////////////////////////////////////////////////////////////////////////

int
ps_evset_reduce(Elem **evset, char *victim, int len, int threshold){
  int list_len = list_length(*evset), i;

  for (i=0; i<list_len; i++) {

    // Pop the first element
    Elem* popped = list_pop(evset);

    // If the reduced list evicts the TARGET, popped element can be removed
    if (ps_evset_test(evset, victim, threshold, 10, EVTEST_MEDIAN)) {
      if (list_length(*evset) == len){ // If the reduced list is minimal, SUCCESS
      return 1; // SUCCESS
    }
  }
    else { // If not, append the popped element to the end of list, and try again
      list_append(evset, popped); 
  }     

  }

  return 0; // FAIL
}

////////////////////////////////////////////////////////////////////////////////

static 
inline 
int 
comp(const void * a, const void * b) {
  return ( *(uint64_t*)a - *(uint64_t*)b );
}

int 
ps_evset_test(Elem **evset, char *victim, int threshold, int test_len, int test_method) {

  // Check, whether the reduced list can evict the TARGET 
    
  int test, time[test_len], time_acc=0;
  int i=0;

  // Place TARGET in LLC
  asm volatile("lfence;mfence");
  maccess((void*)victim);
  asm volatile("lfence;mfence");

  for (test=0; test<test_len; test++) {

    // Potential improvement: could be sped up
    traverse_list_asm_skylake(*evset);
    traverse_list_asm_skylake(*evset);
    traverse_list_asm_skylake(*evset);
    traverse_list_asm_skylake(*evset);
    asm volatile("lfence;mfence");

    // Measure TARGET (and place in LLC for next iteration)
    time[test] = time_mread((void*)victim);
    time_acc += time[test];
  }

  if (test_method == EVTEST_MEAN) {

    return (time_acc/test_len)>threshold;
  
  } else if (test_method == EVTEST_MEDIAN) {

    qsort(time, test_len, sizeof(int), comp);
      
    return time[test_len/2]>threshold;
  
  } else if (test_method == EVTEST_ALLPASS) {

    int all_passed = 1;
    for (test=0; test<test_len; test++)
      if (all_passed && time[test]<threshold) 
        all_passed = 0;
      
    return all_passed;
  }
}

#endif