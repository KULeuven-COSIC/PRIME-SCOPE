#pragma once

#include <stdlib.h>
#include "list_struct.h"
#include "list_traverse.h"

typedef unsigned long long int ul;

int   list_to_array(Elem *ptr, uint64_t *array);

int   list_length(Elem *ptr);
Elem *list_pop(Elem **ptr);
void  list_push(Elem **ptr, Elem *e);
int   list_append(Elem **ptr, Elem *e);
int   list_set_first(Elem **ptr, Elem *e);
void  print_list(Elem *ptr);
