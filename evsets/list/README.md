# LinkList Data Structure

The majority of code in this directory are borrowed from [https://github.com/cgvwzq/evsets](https://github.com/cgvwzq/evsets) repository, which implements various eviction set construction algorithms explained in "[Theory and Practice of Finding Eviction Sets](https://vwzq.net/papers/evictionsets18.pdf)"

The reused code is protected by the license given in the directory. To fulfill the requirements of this license, applied changes to that code are listed below. Introduced additions are subject to the license provided at the main directory of this repository.

## list_struct.h

A modified version of `public_structs.h` in the `evsets` repository. 

The file keeps only the linked-list data structure definition, ignores all the remaining lines.

## list_traverse.c / .h

A modified version of `cache.c` in the `evsets` repository. 

The file keeps only `list_traverse_*` functions, ignores the rest. Moreover, only the first eight traverse functions are borrowed, the remaining are introduced by this work.

## list_utils.c / .h

A modified version of `list_utils.c` in the `evsets` repository. 

The file keeps a subset of the functions, ignores the rest. The first two functions in the files are introduced by this work, the rest are borrowed.
