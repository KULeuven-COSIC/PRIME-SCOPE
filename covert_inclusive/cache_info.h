#pragma once

#define BLOCK_OFFSET      6 // 64B cache lines

////////////////////////////////////////////////////////////////////////////////
// Per-core L1D
#define L1_WAYS           8
#define L1_INDEX_BITS     (BLOCK_OFFSET + 6) // 64 sets
#define L1_PERIOD         (1 << L1_INDEX_BITS)
#define L1_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L1_INDEX_BITS)-1);index;})

////////////////////////////////////////////////////////////////////////////////
// Per-core L2
#define L2_WAYS           4
#define L2_INDEX_BITS     (BLOCK_OFFSET + 10) // 1024 sets
#define L2_PERIOD         (1 << L2_INDEX_BITS)
#define L2_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L2_INDEX_BITS)-1);index;})

////////////////////////////////////////////////////////////////////////////////
// Shared inclusive LLC

#ifdef LLC_WITH_12_WAY_1024_SET
  #define LLC_SLICES        8
  #define LLC_WAYS          12
  #define LLC_INDEX_BITS    (BLOCK_OFFSET + 10) // 1024 sets
  #define LLC_PERIOD        (1 << LLC_INDEX_BITS)
  #define LLC_SET_INDEX(x)  ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << LLC_INDEX_BITS)-1);index;})
#endif

#ifdef LLC_WITH_16_WAY_1024_SET
  #define LLC_SLICES        8
  #define LLC_WAYS          16
  #define LLC_INDEX_BITS    (BLOCK_OFFSET + 10) // 1024 sets
  #define LLC_PERIOD        (1 << LLC_INDEX_BITS)
  #define LLC_SET_INDEX(x)  ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << LLC_INDEX_BITS)-1);index;})
#endif

#define SMALLPAGE_PERIOD    (1 << 12)