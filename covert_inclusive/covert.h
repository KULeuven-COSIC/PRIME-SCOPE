#pragma once

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////


///////////////////////////
// MAIN CHANNEL PARAMETERS
///////////////////////////

/*
  These covert channel parameters are finetuned for an Intel i5-7500 (12-way KABY LAKE).
  To make the P+S based covert channel work (& optimized) for your platform,
  you will very likely have to change these parameters.
*/

#if PLATFORM == KABYLAKE12
  #define FREQ 3400000000     // Frequency of the CPU

  #define CACHE_SET 35        // Select LLC cache set to transmit

  // Prime+Scope Threshold
  #define PS_THRESH 60 

  // Principal Covert Channel Parameters (see Fig. 8)
  #define BITS 4              // Number of bits to encode per window
  #define SLOT 105            // Size of one microSLOT
  #define OFFSET 80           // Offset between microSLOTs of Tx and Rx
  #define PREPARE 1400        // Time for Rx to PRIME the cache

  // Derived Covert Channel Parameters
  #define SYMBOLS (1<<BITS)   // Number of symbols
  #define WINDOW_HELPER (PREPARE+OFFSET-450+SYMBOLS*SLOT) 
  #define PREPARE_TRIGGER (WINDOW_HELPER-j*SLOT) 
  #define WINDOW_TRANSMIT (SYMBOLS*SLOT+OFFSET)
  #define PREPARE_LEFTOVER (OFFSET)
#endif 

// Minor optimizations
#define TRANSMITTER_FENCE 1
#define TRANSMITTER_FLUSH 1

#define WINDOW_SYNCH 100000000
#define MESSAGE_LEN  1<<20
#define MBPS 1000000 

//////////////////////////
// COVERT CHANNEL WINDOWS
//////////////////////////

#define WINDOW_NEXT(x) ({     \
  tick_next += x;             \
  while(rdtscp64() < tick_next);    })
