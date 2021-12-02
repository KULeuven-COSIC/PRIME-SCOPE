#!/bin/bash

################################################################################
# 
# This script is made to control the execution of PrimeTime. 
# The execution is divided into commands (explained below), which allow 
# creating prime patterns, measure their performance, and filter the 
# well-performing ones. Besides, it iteratively measures the well-performing 
# patterns and filters them again.
# 
# The supported commands are explained below and implemented in 
# `execute_functions.sh`. In addition, how these commands, so the PrimeTime, 
# work is explained in the readme.me file.
# 
# 
# The Supported Commands and Their EXplanations:
# 
#   createPrimes <out_pattern_file> <type> 
# 
#     This operation creates an initial set of access patterns.
#   
#     <out_pattern_file> is the filename to which created patterns will be 
#     written.
#     <type> arguments can be set to either `var0`` or `var1`, respectively
#     for choosing either a sophisticated or simple set.
# 
# 
# execution <in_pattern_file> <out_measurements_file> <in_previous_measurements_file> <number_of_measurements>
# 
#   This operation takes a given pattern file, converts the encoded patterns 
#   into C code, compiles the primeapp with that new code, takes measurements 
#   of how successfull the patterns are in setting the eviction candidate 
#   correctly and produces a measurement output file.
# 
#     <in_pattern_file> is the filename from which patterns to be tested are 
#     read.
#     <out_measurements_file> is the filename to which the measurement results 
#     are written.
#     <in_previous_measurements_file> for indicating how many measurements are 
#     taken.
#     <number_of_measurements> is an optional argument for the file name of 
#     previous measurements. We expect an increase in the success rate at each 
#     execution. Hence, this operand allows checking if measurements meet with 
#     this expectation. If not, the execution is repeated
# 
# 
# filterByEVRate  <in_pattern_file> <in_measurements_file> <out_pattern_file> <threshold_ev_rate>  <duration_slowest>
# filterByEVCRate <in_pattern_file> <in_measurements_file> <out_pattern_file> <threshold_evc_rate> <duration_slowest>
# 
#   These operations analyse the measurement results, and picks the patterns 
#   well performing according to given filtering parameters.
# 
#     <in_pattern_file> is the patterns file used in the previous execution.
#     <in_measurements_file> is thi measurements file created by the previous 
#     execution.
#     <out_pattern_file> is the patterns file, listing the filtered patterns.
#     <threshold_evc_rate> and <threshold_ev_rate> are the thresholds to 
#     determine the success of a pattern.
#     <duration_slowest> another threshold to filter out of slow patterns
# 
# 
# filterBySpeed <in_pattern_file> <in_measurements_file> <out_pattern_file> <count>
# 
#   This command is for picking a given number of fastest patterns
# 
#     <in_pattern_file> is the patterns file used in the previous execution.
#     <in_measurements_file> is thi measurements file created by the previous 
#     execution.
#     <out_pattern_file> is the patterns file, listing the filtered patterns.
#     <count> indicated how many of the fastest patterns are requested.
# 
# 
# sortByEVCr <in_pattern_file> <in_measurements_file> <out_pattern_file> <count>
# 
#   This command sorts the output patterns.
# 
#     <in_pattern_file> is the patterns file used in the previous execution.
#     <in_measurements_file> is thi measurements file created by the previous 
#     execution.
#     <out_pattern_file> is the patterns file, listing the filtered patterns.
#     <count> indicated how many of the fastest patterns are requested.
# 
# 
################################################################################

source ./execute_functions.sh

begin

clean
createPrimes           patterns0   var0

execution              patterns0   measurements0              10000
filterByEVRateAbsolute patterns0   measurements0  patterns1i     98
filterBySpeed          patterns1i  measurements0  patterns1     100

execution              patterns1   measurements1             100000
filterBySpeed          patterns1   measurements1  patterns2      50

sortByEVr              patterns2   measurements1  sorted

end