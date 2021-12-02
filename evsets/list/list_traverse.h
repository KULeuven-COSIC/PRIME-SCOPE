#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "list_struct.h"

void traverse_list_skylake          (Elem *ptr);
void traverse_list_haswell          (Elem *ptr);
void traverse_list_simple           (Elem *ptr);
void traverse_list_asm_skylake      (Elem *ptr);
void traverse_list_asm_haswell      (Elem *ptr);
void traverse_list_asm_simple       (Elem *ptr);
void traverse_list_to_n             (Elem *ptr, int n);
void traverse_list_to_n_skylake     (Elem *ptr, int n);
void traverse_naive                 (Elem *ptr);
void traverse_flush                 (Elem *ptr);
void traverse_zigzag_victim         (Elem *ptr, void *victim);