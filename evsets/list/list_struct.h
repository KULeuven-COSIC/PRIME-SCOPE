#pragma once

#include <stddef.h> // For size_t

typedef struct elem
{
  struct elem *next;
  struct elem *prev;
  int          set;
  size_t       delta;
  char         pad[32]; // up to 64B
} Elem;