////////////////////////////////////////////////////////////////////////////////
// Basic Memory Operations

#define READ_ACCESS(x)  ({                                        \
  maccess((void*)x);                                              })

#define TIME_READ_ACCESS(x)  ({                                   \
  timeSW = time_mread((void*)x);                             })

#define TIME_READ_BENCHMARK()   ({                                \
  timing[m++][t] = time_mread((void*)TARGET);                     })

#define WRITE_ACCESS(x)  ({                                       \
  memwrite((void*)x);                                             })

#define FLUSH(x)  ({                                              \
  flush((void*)x);                                                }) 

#define ACCESS_EVC()  ({                                          \
  timeSW = time_mread((void*) EVC);                               })

////////////////////////////////////////////////////////////////////////////////
// Victim Memory Operations

#define VICTIM_READ(x)   ({                                       \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 1;                                            \
  while(*synchronization==1); })

#define VICTIM_MEASURE(x)   ({                                    \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 2;                                            \
  while(*synchronization==2);                                      \
  timeVictim = *synchronization_params;                            \
})

////////////////////////////////////////////////////////////////////////////////
// Detect platform
#define PRINT_PLATFORM() ({\
  FILE *fp = fopen("/proc/cpuinfo", "r");\
  assert(fp != NULL);\
  size_t n = 0;\
  char *line = NULL;\
  while (getline(&line, &n, fp) > 0) {\
    if (strstr(line, "model name")) {\
      printf(" %s", line); break;\
    }\
  }\
  free(line); fclose(fp);\
  printf("============================== \n\n");\
})

////////////////////////////////////////////////////////////////////////////////
// Configure thresholds
#define CONFIGURE_THRESHOLDS() ({\
    for (t=0; t<TEST_COUNT; t++) {\
      m=0;\
      FLUSH(TARGET);\
      VICTIM_READ(TARGET);\
      TIME_READ_BENCHMARK(); /*time 0: LLC*/\
      FLUSH(TARGET);\
      TIME_READ_BENCHMARK(); /* time 1: DRAM*/\
      TIME_READ_BENCHMARK(); /* time 2: L1/L2*/\
    }\
    qsort(timing[0], TEST_COUNT, sizeof(int), compare); qsort(timing[1], TEST_COUNT, sizeof(int), compare); qsort(timing[2], TEST_COUNT, sizeof(int), compare);\
    SW_LLC = timing[0][(int) 0.10*TEST_COUNT]; \
    SW_RAM = timing[1][(int) 0.50*TEST_COUNT]; \
    SW_L1 = timing[2][(int) 0.10*TEST_COUNT]; \
    SW_THR = (2*SW_RAM + SW_LLC)/3;\
})

////////////////////////////////////////////////////////////////////////////////
// Miscellaneous
#define ASSERT(x) assert(x != -1)
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)>(b))?(b):(a))
#define FENCE asm volatile ("mfence\n\t lfence\n\t");
#define MFENCE asm volatile ("mfence\n\t");
#define LFENCE asm volatile ("lfence\n\t");

#define BUSY_WAIT() ({                                            \
  for (i = 30000; i>0; i--)                                       \
    asm volatile("nop;");                                         })

// Print Colors
#define BLACK   "\033[0;30m"     
#define DGRAY   "\033[1;30m"
#define LGRAY   "\033[0;37m"     
#define WHITE   "\033[1;37m"
#define RED     "\033[0;31m"     
#define LRED    "\033[1;31m"
#define GREEN   "\033[0;32m"     
#define LGREEN  "\033[1;32m"
#define ORANGE   "\033[0;33m"     
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[0;34m"     
#define LBLUE   "\033[1;34m"
#define PURPLE  "\033[0;35m"     
#define LPURPLE "\033[1;35m"
#define CYAN    "\033[0;36m"     
#define LCYAN   "\033[1;36m"
#define NC      "\033[0m"

// Comparison routine for sorting the tests
int compare(const void * a, const void * b) {
  return ( *(uint64_t*)a - *(uint64_t*)b );
}