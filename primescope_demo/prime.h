#pragma once

#include <stdint.h>
#include "configuration.h"
#include "../utils/cache_utils.h"
#include "../evsets/list/list_utils.h"

////////////////////////////
// PRIME PATTERNS (Table 1)
////////////////////////////
void traverse_Intel_Core_i7_9700K   (uint64_t* arr); // 2018 | Coffee Lake  | Inclusive LLC with 12 Ways     | 4 Repeats
void traverse_Intel_Core_i7_7700K   (uint64_t* arr); // 2017 | Kaby Lake    | Inclusive LLC with 16 Ways     | 2 Repeats
void traverse_Intel_Core_i5_7500    (uint64_t* arr); // 2017 | Kaby Lake    | Inclusive LLC with 12 Ways     | 3 Repeats
void traverse_Intel_Core_i7_6700    (uint64_t* arr); // 2015 | Skylake      | Inclusive LLC with 16 Ways     | 3 Repeats
void traverse_Intel_Core_i5_6500    (uint64_t* arr); // 2015 | Skylake      | Inclusive LLC with 12 Ways     | 4 Repeats
void traverse_Intel_Core_i7_4790    (uint64_t* arr); // 2013 | Haswell      | Inclusive LLC with 16 Ways     | 3 Repeats
void traverse_Intel_Core_i5_4590    (uint64_t* arr); // 2013 | Haswell      | Inclusive LLC with 12 Ways     | 2 Repeats
void traverse_Intel_Core_i7_3770    (uint64_t* arr); // 2012 | Ivy Bridge   | Inclusive LLC with 16 Ways     | 3 Repeats
void traverse_Intel_Core_i5_3450    (uint64_t* arr); // 2012 | Ivy Bridge   | Inclusive LLC with 12 Ways     | 2 Repeats
void traverse_Intel_Core_i5_2400    (uint64_t* arr); // 2011 | Sandy Bridge | Inclusive LLC with 12 Ways     | 5 Repeats
void traverse_Intel_Xeon_Silver_4208(Elem*    list); // 2019 | Xeon Silver  | Non-Inclusive LLC with 11 Ways (CD has 12 ways) | 1 Repeat




#ifdef KABYLAKE12
  #define PRIME(x) ({               \
    traverse_Intel_Core_i5_7500(x); \
    traverse_Intel_Core_i5_7500(x); \
    traverse_Intel_Core_i5_7500(x); })
#endif

#ifdef KABYLAKE16
  #define PRIME(x) ({               \
    traverse_Intel_Core_i7_7700K(x); \
    traverse_Intel_Core_i7_7700K(x); })
#endif

#ifdef COFFEELAKE12
  #define PRIME(x) ({               \
    traverse_Intel_Core_i7_9700K(x); \
    traverse_Intel_Core_i7_9700K(x); \
    traverse_Intel_Core_i7_9700K(x); \
    traverse_Intel_Core_i7_9700K(x); })
#endif

#ifdef SKYLAKE16
  #define PRIME(x) ({               \
    traverse_Intel_Core_i7_6700(x); \
    traverse_Intel_Core_i7_6700(x); \
    traverse_Intel_Core_i7_6700(x); })
#endif

#ifdef SKYLAKE12
  #define PRIME(x) ({               \
    traverse_Intel_Core_i5_6500(x); \
    traverse_Intel_Core_i5_6500(x); \
    traverse_Intel_Core_i5_6500(x); \
    traverse_Intel_Core_i5_6500(x); })
#endif

#ifdef HASWELL16
  #define PRIME(x) ({               \
    traverse_Intel_Core_i7_4790(x); \
    traverse_Intel_Core_i7_4790(x); \
    traverse_Intel_Core_i7_4790(x); })
#endif

#ifdef HASWELL12
  #define PRIME(x) ({               \
    traverse_Intel_Core_i5_4590(x); \
    traverse_Intel_Core_i5_4590(x); })
#endif

#ifdef IVYBRIDGE16
  #define PRIME(x) ({               \
    traverse_Intel_Core_i7_3770(x); \
    traverse_Intel_Core_i7_3770(x); \
    traverse_Intel_Core_i7_3770(x); })
#endif

#ifdef IVYBRIDGE12
  #define PRIME(x) ({               \
    traverse_Intel_Core_i5_3450(x); \
    traverse_Intel_Core_i5_3450(x); })
#endif

#ifdef SANDYBRIDGE12
  #define PRIME(x) ({               \
    traverse_Intel_Core_i5_2400(x); \
    traverse_Intel_Core_i5_2400(x); \
    traverse_Intel_Core_i5_2400(x); \
    traverse_Intel_Core_i5_2400(x); \
    traverse_Intel_Core_i5_2400(x); })
#endif
  
#if defined XEONSILVER12 || defined XEONPLATINUM12
  #define PRIME(x) ({                   \
    traverse_Intel_Xeon_Silver_4208(x); })
#endif

#ifdef YOUROWNPLATFORM
  #error Prime function undefined!
#endif
