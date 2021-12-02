#pragma once

#if defined XEONSILVER12
  #define LLC_NONINCLUSIVE
  #define LLC_WITH_11_WAY_2048_SET_8_SLICE

#elif defined XEONPLATINUM12
  #define LLC_NONINCLUSIVE
  #define LLC_WITH_11_WAY_2048_SET_28_SLICE

#else
  #define LLC_INCLUSIVE 
  
  #if defined KABYLAKE16 || \
      defined SKYLAKE16  || \
      defined HASWELL16  || \
      defined IVYBRIDGE16
    #define LLC_WITH_16_WAY_1024_SET 
  #else
    #define LLC_WITH_12_WAY_1024_SET 
  #endif

#endif


////////////////////////////////////////////////////////////////////////////////
// Throw error if there is a multiple platform definitions

#ifdef KABYLAKE12
  #define ALREADY_DEFINED
#endif

#ifdef KABYLAKE16
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef COFFEELAKE12
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef SKYLAKE16
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef SKYLAKE12
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef HASWELL16
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef HASWELL12
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef IVYBRIDGE16
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef IVYBRIDGE12
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef SANDYBRIDGE12
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
  #define ALREADY_DEFINED
#endif

#ifdef YOUROWNPLATFORM
  #ifdef ALREADY_DEFINED
    #error PLATFORM IS ALREADY DEFINED
  #endif
#endif
