# Eviction Set Construction Tool

As shown in the paper, the Prime+Scope-style repeatable observations can be leveraged for a simple, portable and highly efficient construction of eviction sets.

This directory provides easy-to-use functions to construct eviction sets both for inclusive and non-inclusive Last-Level Caches (LLC) on Intel Processors.

## What is Supported?

Note that this directory only provides the library functions for eviction set construction, but not an example application. For checking an application example, you can look into the [`../primescope_demo`](../primescope_demo) directory.

The implemented functions are provided in `./ps_evset.h` file. They differ slightly for inclusive and non-inclusive caches, as explained below.

### Inclusive LLC

In the case of inclusive caches, there are only three functions;

* `ps_evset()` for LLC eviction set contruction.
* `ps_evset_test()` for verifying the contructed set.
* `ps_evset_premap()` for page table initialisation with a simple traverse over the memory allocated for eviction set construction.

### Non-Inclusive LLC

The same structure is kept for non-inclusive caches, while eviction set construction is made possible for different cache levels. These functions are summarised as;

* `ps_evset_llc()` is again for LLC eviction set contruction. Besides, it offers an option to construct eviction set for either LLC or Contention Directory (CD). On the tested CPUs, these two cache structures have the same set and slice mapping, but a different associativity.
* `ps_evset_l1()` and `ps_evset_l2()` are two extra functions for L1 and L2 eviction set construction which, in this repository, are used for access latency calibration for L2, the LLC or RAM.
* `ps_evset_test()` for verifying the contructed set.
* `ps_evset_premap()` for page table initialisation with a simple traverse over the memory allocated for eviction set construction.

## How to Configure and Compile?

Compilation is simply done with `make CONFIG_FILE=<path>/<to>/configuration.h`. This creates object files, and requires a configuration file tweaking the eviction set construction to the target processor. Each application that uses the eviction set constuction may have a different configuration.

To enable this, we exclude the configuration file from this directory but made it a part of the applications. Various examples are available in the [primescope_demo](../primescope_demo/configuration.h), [covert_inclusive](../covert_inclusive/configuration.h) and [primetime](../primetime/primeapp/configuration.h) applications.

To help users understand different configuration options, [this file](../primescope_demo/configuration.h) provides detailed inline comments.

## Helper Thread for Non-inclusive Intel LLCs

The eviction set construction algorithm places (potentially congruent) candidate lines into the LLC and checks if they evict the scope line. The eviction happens if enough congruent lines are placed into LLC (details in the [Prime+Scope paper](https://www.esat.kuleuven.be/cosic/publications/article-3405.pdf)). 

For placing a line in an inclusive LLC, a simple read/write access is enough. However, if the LLC is non-inclusive, the same does not (necessarily) hold. The access introduces it into the L1/2 caches, but may skip the LLC. Without generating contention on the LLC, the eviction set construction algorithm cannot work. 

To force cache lines to take up LLC real estate, we leverage the following observation. Assuming an uncached line, a cache fill installs it in the private L1/L2 caches of the relevant core. When another thread running on another core accesses the same cache line, the line is _copied_ to the LLC. For this reason, eviction set construction algorithm spawns two threads for platforms with non-inclusive LLCs.

## Acknowledgment - Linked List Data Structure

The constructed eviction sets are organized in a linked list structure. The underlying linked list data structure implementation is borrowed from the [https://github.com/cgvwzq/evsets](https://github.com/cgvwzq/evsets) repository, slightly extended, and contained in [`./list`](./list) directory. The corresponding code, list of made modifications and license information are provided in the same directory.
