#pragma once

#ifdef LLC_INCLUSIVE 

  #define BLOCK_OFFSET      6 // 64B cache lines

  //////////////////////////////////////////////////////////////////////////////
  // Per-core L1D
  #define L1_WAYS           8
  #define L1_INDEX_BITS     (BLOCK_OFFSET + 6) // 64 sets
  #define L1_PERIOD         (1 << L1_INDEX_BITS)
  #define L1_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L1_INDEX_BITS)-1);index;})

  //////////////////////////////////////////////////////////////////////////////
  // Per-core L2
  #define L2_WAYS           4
  #define L2_INDEX_BITS     (BLOCK_OFFSET + 10) // 1024 sets
  #define L2_PERIOD         (1 << L2_INDEX_BITS)
  #define L2_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L2_INDEX_BITS)-1);index;})

  //////////////////////////////////////////////////////////////////////////////
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

#endif // LLC_INCLUSIVE


#ifdef LLC_NONINCLUSIVE 
  // The following is tested on both Intel Xeon Silver 4208 and Platinum 8280

  #define BLOCK_OFFSET      6 // 64B cache lines

  //////////////////////////////////////////////////////////////////////////////
  // Per-core L1D
  #define L1_WAYS           8
  #define L1_INDEX_BITS     (BLOCK_OFFSET + 6) // 64 sets
  #define L1_PERIOD         (1 << L1_INDEX_BITS)
  #define L1_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L1_INDEX_BITS)-1);index;})

  //////////////////////////////////////////////////////////////////////////////
  // Per-core L2
  #define L2_WAYS           16
  #define L2_INDEX_BITS     (BLOCK_OFFSET + 10) // 1024 sets
  #define L2_PERIOD         (1 << L2_INDEX_BITS)
  #define L2_SET_INDEX(x)   ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << L2_INDEX_BITS)-1);index;})

  //////////////////////////////////////////////////////////////////////////////
  // Shared non-inclusive LLC

  #ifdef LLC_WITH_11_WAY_2048_SET_8_SLICE
    #define LLC_SLICES        8 
    #define LLC_WAYS          11
    #define LLC_INDEX_BITS    (BLOCK_OFFSET + 11) // 2048 sets per slice
    #define LLC_PERIOD        (1 << LLC_INDEX_BITS)
    #define LLC_SET_INDEX(x)  ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << LLC_INDEX_BITS)-1);index;})
    #define CD_WAYS           12
  #endif

  #ifdef LLC_WITH_11_WAY_2048_SET_28_SLICE
    #define LLC_SLICES        28 
    #define LLC_WAYS          11
    #define LLC_INDEX_BITS    (BLOCK_OFFSET + 11) // 2048 sets per slice
    #define LLC_PERIOD        (1 << LLC_INDEX_BITS)
    #define LLC_SET_INDEX(x)  ({uint64_t index = ( x & ~((1 << BLOCK_OFFSET)-1)) & ((1 << LLC_INDEX_BITS)-1);index;})
    #define CD_WAYS           12
  #endif

  #define SMALLPAGE_PERIOD    (1 << 12)

#endif // LLC_NONINCLUSIVE