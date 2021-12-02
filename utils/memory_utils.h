#pragma once

////////////////////////////////////////////////////////////////////////////////
// SIZES

#include "memory_sizes.h"

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0x40000
#endif
#ifndef MAP_HUGE_SHIFT
#define MAP_HUGE_SHIFT 26
#endif

#define MAP_2M_HUGEPAGE (0x15 << MAP_HUGE_SHIFT) /* 2 ^ 0x15 = 2M */
#define MAP_1G_HUGEPAGE (0x1e << MAP_HUGE_SHIFT) /* 2 ^ 0x1e = 1G */

////////////////////////////////////////////////////////////////////////////////
// DEFINITIONS FOR SHARED MEMORIES

#define SPROTECTION (PROT_READ   | PROT_WRITE     )
#define SFLAGS_4K   (MAP_SHARED  | MAP_ANONYMOUS  )
#define SFLAGS_HG   (SFLAGS_4K   | MAP_HUGETLB    )
#define SFLAGS_2M   (SFLAGS_HG   | MAP_2M_HUGEPAGE)
#define SFLAGS_1G   (SFLAGS_HG   | MAP_1G_HUGEPAGE)

#define PFLAGS_4K   (MAP_PRIVATE | MAP_ANONYMOUS  )
#define PFLAGS_HG   (PFLAGS_4K   | MAP_HUGETLB    )
#define PFLAGS_2M   (PFLAGS_HG   | MAP_2M_HUGEPAGE)
#define PFLAGS_1G   (PFLAGS_HG   | MAP_1G_HUGEPAGE)

int mem_map_private(uint64_t** addr, int len, int withhuge);
int mem_map_shared (uint64_t** addr, int len, int withhuge);
int mem_unmap      (uint64_t*  addr, int len);

int var_map_shared (volatile uint64_t** addr);
int var_unmap      (volatile uint64_t*  addr);

////////////////////////////////////////////////////////////////////////////////
// DEFINITIONS FOR EVICTION MEMORIES

#define EPROTECTION (PROT_READ | PROT_WRITE)
#define EFLAGS_4K (MAP_SHARED | MAP_ANONYMOUS)
#define EFLAGS_2M (MAP_2M_HUGEPAGE | EFLAGS_4K  | MAP_HUGETLB)
#define EFLAGS_1G (MAP_1G_HUGEPAGE | EFLAGS_4K  | MAP_HUGETLB)