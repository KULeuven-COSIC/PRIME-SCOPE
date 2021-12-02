# Covert Channel (Prime+Scope style) on the LLC

## Description

This folder implements the covert channel as described in Section 6.3 of the [paper](https://www.esat.kuleuven.be/cosic/publications/article-3405.pdf).

<p align="center" width="100%">
    <img width="500" src="../figures/covert.svg"> 
</p>

The transmitter (TX) and receiver (RX) are implemented in `transmitter.c` and `receiver.c`, respectively.

## Execution
To execute the covert channel PoC:

1. Edit the `configuration.h` file with the cache information and execution parameters (e.g., whether huge pages are available).
2. Define the Prime access pattern in `prime.h`, depending on your target CPU, and change the `PRIME_COVERT(x)` macro. Some pointers:

      - See [this header file](../primescope_demo/prime.h) for the patterns of Table 1 in the paper.
      - Alternatively, run [PrimeTime](../primetime) on your Intel machine to discover a good pattern.

3. Edit the `covert.h` file to set the covert channel parameters (as specified in the figure.)

    Note that these covert channel parameters are finetuned for an Intel i5-7500 (Kaby Lake, 12-way LLC). To make the P+S based covert channel work (and optimized) for your platform, you will very likely have to change these parameters. Also set the clock frequency in order for the capacity calculations to be correct.

4. Compile with `make`, and run with `make run`. This will also invoke the `analyze.py` script that generates the statistics for the covert channel run.
    - (If the program terminates with segmentation fault, one possible cause is that `MESSAGE_LEN` is too large compared to the stack size.
    Check whether the problem persists after `ulimit -s unlimited`.)
