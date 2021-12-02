#include <stdio.h>

#include "list_traverse.h"
#include "list_struct.h"

#include "../../utils/cache_utils.h"

inline
void
traverse_list_skylake(Elem *ptr)
{
	while (ptr && ptr->next && ptr->next->next)
	{
		maccess (ptr);
		maccess (ptr->next);
		maccess (ptr->next->next);
		maccess (ptr);
		maccess (ptr->next);
		maccess (ptr->next->next);
		ptr = ptr->next;
	}
}

inline
void
traverse_list_asm_skylake(Elem *ptr)
{
	__asm__ volatile
	(
		"test %%rcx, %%rcx;"	
		"jz out;"
		"loop:"
		"movq (%%rcx), %%rax;" // memread 0
		"test %%rax, %%rax;" // 
		"jz out;"
		"movq (%%rax), %%rax;" // memread 1
		"test %%rax, %%rax;" // 
		"jz out;"
		"movq (%%rax), %%rax;" // memread 2
		"movq (%%rcx), %%rcx;" // memread 0, ptr is now 1
		"movq (%%rcx), %%rax;" // memread 1
		"movq (%%rax), %%rax;" // memread 2
		"test %%rcx, %%rcx;"
		"jnz loop;"
		"out:"
		: // no output
		: "c" (ptr) // ptr in rcx
		: "cc", "memory"
	);
}

inline
void
traverse_list_asm_haswell(Elem *ptr)
{
	__asm__ volatile
	(
		"test %%rcx, %%rcx;"
		"jz out2;"
		"loop2:"
		"movq (%%rcx), %%rax;"
		"test %%rax, %%rax;"
		"jz out2;"
		"movq (%%rax), %%rax;"
		"movq (%%rcx), %%rcx;"
		"movq (%%rcx), %%rax;"
		"test %%rcx, %%rcx;"
		"jnz loop2;"
		"out2:"
		: // no output
		: "c" (ptr)
		: "cc", "memory"
	);
}

inline
void
traverse_list_asm_simple(Elem *ptr)
{
	__asm__ volatile
	(
		"loop3:"
		"test %%rcx, %%rcx;"
		"jz out3;"
		"movq (%%rcx), %%rcx;"
		"jmp loop3;"
		"out3:"
		: // no output
		: "c" (ptr)
		: "cc", "memory"
	);
}

inline
void
traverse_list_haswell(Elem *ptr)
{
	while (ptr && ptr->next)
	{
		maccess (ptr);
		maccess (ptr->next);
		maccess (ptr);
		maccess (ptr->next);
		ptr = ptr->next;
	}
}

inline
void
traverse_list_simple(Elem *ptr)
{
	while (ptr)
	{
		maccess (ptr);
		ptr = ptr->next;
	}
}

inline
void
traverse_list_to_n(Elem *ptr, int n)
{
	while (ptr && n-- > 0)
	{
		maccess (ptr);
		ptr = ptr->next;
	}
}

inline
void
traverse_list_to_n_skylake(Elem *ptr, int n)
{
	while (n > 2 && ptr && ptr->next && ptr->next->next)
	{
		maccess (ptr);	maccess (ptr->next);	maccess (ptr->next->next);
		maccess (ptr);	maccess (ptr->next);	maccess (ptr->next->next);
		ptr = ptr->next;
		n--;
	}

	while (n > 1 && ptr && ptr->next)
	{
		maccess (ptr);	maccess (ptr->next);
		maccess (ptr);	maccess (ptr->next);
		ptr = ptr->next;
		n--;
	}

	while (n > 0 && ptr)
	{
		maccess (ptr);
		ptr = ptr->next;
		n--;
	}
}

inline
void
// Traverse list, interleave with access to first element
// To keep it in private caches
traverse_zigzag_victim(Elem *ptr, void *victim)
{
    while (ptr)
    {
        maccess (ptr);
        maccess (victim);
        ptr = ptr->next;
    }
}
