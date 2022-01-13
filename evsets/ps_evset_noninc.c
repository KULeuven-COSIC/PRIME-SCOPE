#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "ps_evset.h"
#include "./list/list_utils.h"
#include "../utils/cache_utils.h"

#ifdef LLC_NONINCLUSIVE

////////////////////////////////////////////////////////////////////////////////
// Declaration of functions

int  ps_evset_reduce(Elem **evset, char *victim, int len, int threshold, int is_cd);

void meas_collect_cd (Elem **evset, char *victim, int *time, int test_len);
void meas_collect_llc(Elem **evset, char *victim, int *time, int test_len);
int  meas_eval(int *time, int threshold, int test_len, int test_method);

////////////////////////////////////////////////////////////////////////////////
// Definition of functions

// Construct L1 eviction set, simply based on set index mapping (L1 is virtually indexed)
int
ps_evset_l1(Elem **evset, char *victim, int len, uint64_t* page, int is_huge)
{
	
	//////////////////////////////////////////////////////////////////////////////
	// Create a guess pool

	int index = 0, i=0;
	uint64_t address;
	
	// Expects L2 to have more sets than L1
  assert(L2_PERIOD > L1_PERIOD);
	uint64_t mask = (L2_PERIOD-1) ^ (L1_PERIOD-1);
	
	while (index<len) {
		address = ((uint64_t)page + ((uint64_t)victim & (L1_PERIOD-1)) + i++*L1_PERIOD);
		
		// Check if "address" is beyond the range
		assert((address-(uint64_t)page)<EVICT_L1_SIZE);
		
		if (((uint64_t)page & mask) != (address & mask)) {
			if (index++==0)
				list_set_first(evset, (Elem*)address);
			else
				list_append(evset, (Elem*)address);
		}
	}

	return PS_SUCCESS;
}

int
ps_evset_l2(Elem **evset, char *victim, int len, uint64_t* page, int is_huge)
{
  // Potential improvement: implementation for small memory pages
  assert(is_huge);

	//////////////////////////////////////////////////////////////////////////////
	// Create a guess pool

	int index = 0, i=0;
	uint64_t address;
	
	// Expects L2 to have more sets than L1
  assert(LLC_PERIOD > L2_PERIOD);
	uint64_t mask = (LLC_PERIOD-1) ^ (L2_PERIOD-1);
	
	while (index<len) {
		address = ((uint64_t)page + ((uint64_t)victim & (L2_PERIOD-1)) + i++*L2_PERIOD);
		
		// Check if "address" is beyond the range
		assert((address-(uint64_t)page)<EVICT_L2_SIZE);
		
		if (((uint64_t)page & mask) != (address & mask)) {
			if (index++==0)
				list_set_first(evset, (Elem*)address);
			else
				list_append(evset, (Elem*)address);
		}
	}

	return PS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

volatile uint64_t *sync;
volatile uint64_t *sync_params;

void helper_read(uint64_t address);

int
ps_evset_llc(Elem **evset, char *victim, int len, uint64_t* page, int is_huge, int is_cd, int threshold_llc, int threshold_l2,
	volatile uint64_t *syncronization, volatile uint64_t *syncronization_params)
{
	int is_list_empty = 1;
	int list_len = 0;
  int len_requested = len;
	int guess_index = 0;
	int counter_attempt = 0;
	int i;

	int contdir_test = 0;
	int counter_contdir_test = 0;

	Elem *evset_last;

	sync = syncronization;
	sync_params = syncronization_params;

	//////////////////////////////////////////////////////////////////////////////
	// Create a guess pool

	uint64_t guess_pool[MAX_POOL_SIZE];
	
	int pool_size = (is_huge) ? MAX_POOL_SIZE_HUGE : MAX_POOL_SIZE_SMALL;

#if RANDOMIZE_GUESS_POOL == 0
	for (i=0; i<pool_size; i++)	
		guess_pool[i] = (is_huge) ?
			((uint64_t)page + ((uint64_t)victim & (LLC_PERIOD-1			 )) + i*LLC_PERIOD 			): 
			((uint64_t)page + ((uint64_t)victim & (SMALLPAGE_PERIOD-1)) + i*SMALLPAGE_PERIOD);
#else
  // Potential improvement: randomization could be better (e.g., to avoid duplicates)
  for (i=0; i<pool_size; i++)
    guess_pool[i] = (is_huge) ?
      ((uint64_t)page + ((uint64_t)victim & (LLC_PERIOD-1      )) + (rand() % MAX_POOL_SIZE_HUGE )*LLC_PERIOD      ): 
      ((uint64_t)page + ((uint64_t)victim & (SMALLPAGE_PERIOD-1)) + (rand() % MAX_POOL_SIZE_SMALL)*SMALLPAGE_PERIOD);
#endif

	//////////////////////////////////////////////////////////////////////////////
	// Start finding eviction set
	
extend:
	while (list_len<len) {
    int counter_guess = 0;
		// int counter_early_eviction = 0;
		int try_guesses = 1;
		int time;

retry_after_early_eviction:

		// Place TARGET in LLC
		mread((void*)victim);
		helper_read((uint64_t)victim);

#if ENABLE_ALREADY_FOUND
    // Place ALREADY_FOUND in LLC
		#define MIN(a,b) (((a)<(b))?(a):(b))
		Elem *ptr = evset_last;
		i = MIN(LLC_WAYS-1, list_len);
		while (i > 2 && ptr && ptr->next && ptr->next->next) {
			maccess (ptr); 							helper_read((uint64_t)ptr);
			maccess (ptr->next); 				helper_read((uint64_t)ptr->next);
			maccess (ptr->next->next); 	helper_read((uint64_t)ptr->next->next);
			maccess (ptr); 							helper_read((uint64_t)ptr);
			maccess (ptr->next); 				helper_read((uint64_t)ptr->next);
			maccess (ptr->next->next); 	helper_read((uint64_t)ptr->next->next);
			ptr = ptr->next;
			i--;
		}
		while (i > 0 && ptr) {
			maccess (ptr);							helper_read((uint64_t)ptr);
			maccess (ptr);							helper_read((uint64_t)ptr);
			ptr = ptr->next;
			i--;
		}
#endif

#if ENABLE_EARLY_EVICTION_CHECK
#endif

		// Search 
		while (try_guesses) {

			// Place guess in the LLC by performing a shared access
			mread((void*)guess_pool[guess_index]);
			helper_read((uint64_t)guess_pool[guess_index]);

			// Access for page table
			mread((void*)(victim + 222)); // May or may not be necessary
	
      // Measure TARGET
			time = time_mread((void*)victim);
		
			// If TARGET is evicted
			#if IGNORE_VERY_SLOW == 1
      if (time>threshold_llc && time<threshold_llc*2) {
      #else
      if (time>threshold_llc) { 
      #endif
				try_guesses = 0;
				counter_attempt = 0;

				// Add the guess to linkedlist
				evset_last = (Elem*)guess_pool[guess_index];
				if (is_list_empty)
					list_len = list_set_first(evset, evset_last);
				else 
					list_len = list_append(evset, evset_last);
				is_list_empty = 0;

				// Remove the guess from the pool
				// memcpy(	(char*)&guess_pool[guess_index], 
				// 				(char*)&guess_pool[guess_index+1],
				// 				(pool_size-guess_index+1)*sizeof(uint64_t));
				for (i=guess_index; i<pool_size; i++)
					guess_pool[i] = guess_pool[i+1];
				pool_size--;
			}
			else {
				guess_index++;

				// If too many guesses are tried, either start over, or return with fail
				if (++counter_guess>=pool_size) {
					try_guesses = 0;
					
					if (++counter_attempt>=MAX_ATTEMPT)
						return PS_FAIL_CONSTRUCTION;
				}
			}
      
      // Wrap around the pool
			if (guess_index>=pool_size)
				guess_index = 0;
		}
	}

	int threshold = (is_cd) ? threshold_l2 : threshold_llc;

#if ENABLE_EXTENSION
	// Tests whether the found list can evict the victim
	if (!ps_evset_test(evset, victim, threshold, 10, EVTEST_ALLPASS, is_cd)) {
    if (++len<MAX_EXTENSION)
      goto extend;
    return PS_FAIL_EXTENSION;
  }
#endif

#if ENABLE_REDUCTION
  if (list_len > len_requested) {
		if (!ps_evset_reduce(evset, victim, len_requested, threshold, is_cd))
      return PS_FAIL_REDUCTION;
	}
#endif

	return PS_SUCCESS;
}

inline
void 
helper_read(uint64_t address) {
	*sync_params = (volatile uint64_t)address;
  *sync = 1;
  while(*sync==1);
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
ps_evset_reduce(Elem **evset, char *victim, int len, int threshold, int is_cd)
{
	int list_len = list_length(*evset), i;

  for (i=0; i<list_len; i++) {

    // Reduce the list by popping the first element
    Elem* popped = list_pop(evset);

    // If the reduced list evicts the TARGET
    if (ps_evset_test(evset, victim, threshold, 10, EVTEST_MEDIAN, is_cd)) {
      // If the lenght of the reduced list is fine, return PS_SUCCESS
    	if (list_length(*evset) ==  len)
        return 1;
		}
    // If not, append the popped element to the end of list, and try again
    else      
      list_append(evset, popped);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

static 
inline 
int 
comp(const void * a, const void * b) {
  return ( *(uint64_t*)a - *(uint64_t*)b );
}

int
ps_evset_test(Elem **evset, char *victim, int threshold, int test_len, int test_method, int is_cd) {

	int test, ret=0;
	int *time = (int*)malloc(test_len*sizeof(int));

	if (is_cd) 
		meas_collect_cd(evset, victim, time, test_len);
	else
		meas_collect_llc(evset, victim, time, test_len);

	ret = meas_eval(time, threshold, test_len, test_method);

	free(time);
  return ret;
}

void 
meas_collect_llc(Elem **evset, char *victim, int *time, int test_len) {
	
	int test;
	Elem *ptr;
	for (test=0; test<test_len; test++) {

		ptr = (Elem*)evset;
		while (ptr) {
			clflush_f((void*)ptr);
			ptr = ptr->next;
		}

		clflush_f((void*)victim);

		// Place TARGET in LLC
		mread((void*)victim);
		helper_read((uint64_t)victim);

		// Access the EvSet in LLC
		Elem *ptr = (Elem*)evset;
		while (ptr && ptr->next && ptr->next->next) {
			maccess (ptr); 							helper_read((uint64_t)ptr);
			maccess (ptr->next); 				helper_read((uint64_t)ptr->next);
			maccess (ptr->next->next); 	helper_read((uint64_t)ptr->next->next);
			maccess (ptr); 							helper_read((uint64_t)ptr);
			maccess (ptr->next); 				helper_read((uint64_t)ptr->next);
			maccess (ptr->next->next); 	helper_read((uint64_t)ptr->next->next);
			ptr = ptr->next;
		}
		while (ptr) {
			maccess (ptr); 							helper_read((uint64_t)ptr);
			maccess (ptr); 							helper_read((uint64_t)ptr);
			ptr = ptr->next;
		}

		// Access for page table
		mread((void*)(victim + 222));

		// Measure TARGET
		time[test] = time_mread((void*)victim);
	}
}

void
meas_collect_cd(Elem **evset, char *victim, int *time, int test_len) {

	int test, i;
	Elem *ptr;
	for (test=0; test<test_len; test++) {

		ptr = (Elem*)evset;
		while (ptr) {
			clflush_f((void*)ptr);
			ptr = ptr->next;
		}

		clflush_f((void*)victim);

		// Place TARGET in LLC
		mread((void*)victim);

		// Access the EvSet in LLC
		for (i=0; i<25; i++)
			traverse_list_asm_skylake(*evset);

		// Access for page table
		mread((void*)(victim + 222));

		// Measure TARGET
		time[test] = time_mread((void*)victim);
	}
}

int 
meas_eval(int *time, int threshold, int test_len, int test_method) {

	int test;
	if (test_method == EVTEST_MEAN) {

    int timeAcc=0;
    for (test=0; test<test_len; test++)
      timeAcc +=  time[test];

		return ((timeAcc/test_len)>threshold && (timeAcc/test_len)<threshold*2);

  } else if (test_method == EVTEST_MEDIAN) {

    qsort(time, test_len, sizeof(int), comp);
      
    return (time[test_len/2]>threshold && time[test_len/2]<threshold*2);

  } else if (test_method == EVTEST_ALLPASS) {
	
    int all_passed = 1;
    for (test=0; test<test_len; test++)
			if (all_passed && !(time[test]>threshold && time[test]<threshold*2)) 
        all_passed = 0;

    return all_passed;
  }
}

#endif