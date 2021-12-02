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
// PRIME+SCOPE operations

#define SCOPE()  ({                                                \
  timeSW = time_mread_nofence((void*) evsetList);                  \
})

////////////////////////////////////////////////////////////////////////////////
// Memory Operations to be executed by the transmitter thread

#define TX_READ_ACCESS(x)   ({                                \
  *synchronization_params = (volatile uint64_t)x;                  \
  *synchronization = 1;                                            \
  while(*synchronization==1);                                      })

////////////////////////////////////////////////////////////////////////////////
// Extras

#define BUSY_WAIT() ({                                            \
  for (i = 30000; i>0; i--)                                       \
    asm volatile("nop;");                                         })
