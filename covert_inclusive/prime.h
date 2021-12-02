#pragma once

#include <stdint.h>
#include "../utils/cache_utils.h"

////////////////////////////////////////////////////////////////////////////////
// Platform Information

#define KABYLAKE12 0
#define SKYLAKE12 1     // Your platform here

#define PLATFORM KABYLAKE12

#if PLATFORM == KABYLAKE12
  #define PRIME_COVERT()  ({                            \
    traverse_Intel_Core_i5_7500(evset);                   \
  })                                             
#else 
  /* 
    Define the PRIME pattern for your platform here. Some pointers:
      - See ../primescope_demo/prime.h for the patterns of Table 1
      - Run PRIMETIME on your Intel machine to discover a good pattern
   */


#endif

// 2017 | Kaby Lake    | Inclusive LLC with 12 Ways
inline void traverse_Intel_Core_i5_7500 (uint64_t* arr) {
  int i;
  for(i=0; i<9; i+=4) {
    maccess((void *) arr[i+3]);
    maccess((void *) arr[i+2]);
    maccess((void *) arr[  0]);
    maccess((void *) arr[  0]);
    maccess((void *) arr[i+1]);
    maccess((void *) arr[  0]);
    maccess((void *) arr[  0]);
    maccess((void *) arr[i+0]);
    maccess((void *) arr[i+0]);
    maccess((void *) arr[i+1]);
    maccess((void *) arr[i+2]);
    maccess((void *) arr[i+3]);
  }
}