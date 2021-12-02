#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "misc_utils.h"

// Pin thread to specific core
void set_core(int core, char *print_info) {

  // Define your cpu_set bit mask
  cpu_set_t my_set;

  // Initialize it all to 0, i.e. no CPUs selected
  CPU_ZERO(&my_set);

  // Set the bit that represents core
  CPU_SET(core, &my_set);

  // Set affinity of tihs process to the defined mask
  sched_setaffinity(0, sizeof(cpu_set_t), &my_set);

  // Print this thread's CPU
  printf("Core %2d for %s\n", sched_getcpu(), print_info);
}



// Measure time elapsed for experiment (not used for cache timing measurements)
double time_diff_ms(struct timespec begin, struct timespec end)
{
	double timespan;
	if ((end.tv_nsec-begin.tv_nsec)<0) {
		timespan  = (end.tv_sec -begin.tv_sec  - 1				   )*1.0e3 ;
		timespan += (end.tv_nsec-begin.tv_nsec + 1000000000UL)*1.0e-6;
	} else {
		timespan  = (end.tv_sec -begin.tv_sec                )*1.0e3 ;
		timespan += (end.tv_nsec-begin.tv_nsec               )*1.0e-6;
	}
	return timespan;
}

int comp(const void * a, const void * b) {
  return ( *(uint64_t*)a - *(uint64_t*)b );
}

int comp_double(const void * a, const void * b) {
  if (*(double*)a > *(double*)b)
    return 1;
  else if (*(double*)a < *(double*)b)
    return -1;
  else
    return 0;  
}

inline int median(int *array, int len) {
  qsort(array, len, sizeof(int), comp);
  return array[len/2];
}

