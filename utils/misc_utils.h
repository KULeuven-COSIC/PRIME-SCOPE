#pragma once

#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <time.h>

void set_core(int core, char *print_info);

double time_diff_ms(struct timespec begin, struct timespec end);
int comp(const void * a, const void * b);
int comp_double(const void * a, const void * b);

int median(int *array, int len);