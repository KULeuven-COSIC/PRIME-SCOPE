////////////////////////////////////////////////////////////////////////////////
// Basic Memory Operations

#define READ_ACCESS(x)  ({                                        \
  maccess((void*)x);                                              })

#define TIME_READ_ACCESS(x)  ({                                   \
  access_time = time_mread((void*)x);                             })

#define WRITE_ACCESS(x)  ({                                       \
  memwrite((void*)x);                                             })

#define FLUSH(x)  ({                                              \
  flush((void*)x);                                                })
                                     

////////////////////////////////////////////////////////////////////////////////
// Memory Operations to be executed by the helper thread

#define HELPER_READ_ACCESS(x)   ({                                \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 1;                                            \
  while(*synchronization==1);                                      })

#define KILL_HELPER()   ({                                \
  *synchronization = 99;                                            \
  while(*synchronization==99);                                      })

#define HELPER_TIME_ACCESS(x)   ({                                 \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 3;                                            \
  while(*synchronization==3);                                      \
  access_time = *synchronization_params;                           })

////////////////////////////////////////////////////////////////////////////////
// Memory Operations to be executed by the victim thread

#define VICTIM_READ_ACCESS(x)   ({                                \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 11;                                           \
  while(*synchronization==11);                                     })

////////////////////////////////////////////////////////////////////////////////
// Extras

#define BUSY_WAIT() ({                                            \
  for (i = 30000; i>0; i--)                                       \
    asm volatile("nop;");                                         })
