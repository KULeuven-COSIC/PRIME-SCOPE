#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include "memory_utils.h"

int mem_map_private(uint64_t** addr, int len, int withhuge) {
  
  // map shared memory to process address space
  if (withhuge)
    *addr = (uint64_t*) mmap (NULL, len, PROT_READ|PROT_WRITE, PFLAGS_HG, 0, 0);
  else
    *addr = (uint64_t*) mmap (NULL, len, PROT_READ|PROT_WRITE, PFLAGS_4K, 0, 0);

  if (*addr == MAP_FAILED) {
    if (errno == ENOMEM) {
      if (len > 2 * MB)
        printf("Could not allocate buffer (no free 1 GiB huge pages)\n");
      if (len > 4 * KB)
        printf("Could not allocate buffer (no free 2 MiB huge pages)\n");
      else
        printf("Could not allocate buffer (out of memory)\n");
    }
    printf("mmap failed: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int mem_map_shared(uint64_t** addr, int len, int withhuge) {
  
  // map shared memory to process address space
  
  // map shared memory to process address space
  if (withhuge)
    *addr = (uint64_t*) mmap (NULL, len, SPROTECTION, SFLAGS_HG, 0, 0);
  else
    *addr = (uint64_t*) mmap (NULL, len, SPROTECTION, SFLAGS_4K, 0, 0);
  
  if (*addr == MAP_FAILED) {
    if (errno == ENOMEM) {
      if (len > 2 * MB)
        printf("Could not allocate buffer (no free 1 GiB huge pages)\n");
      if (len > 4 * KB)
        printf("Could not allocate buffer (no free 2 MiB huge pages)\n");
      else
        printf("Could not allocate buffer (out of memory)\n");
    }
    printf("mmap failed: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int mem_unmap(uint64_t *addr, int len) {
  
  if (len > 2 * MB)
    len = (len + (1 * GB - 1)) & (~(1 * GB - 1));
  else if (len > 4 * KB)
    len = 2 * MB;

  // un-map
  if (munmap((void*)addr, len) == -1) {
    printf("munmap failed\n");
    return -1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

int var_map_shared(volatile uint64_t** addr) {
  
  int len = sizeof *addr;
  
  *addr = (volatile uint64_t*)mmap(NULL, len, SPROTECTION, SFLAGS_4K, 0, 0);
  
  if (*addr == MAP_FAILED) {
    printf("mmap failed: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int var_unmap(volatile uint64_t *addr) {
  
  int len = sizeof *addr;
  
  // un-map
  if (munmap((void*)addr, len) == -1) {
    printf("munmap failed\n");
    return -1;
  }
  return 0;
}
