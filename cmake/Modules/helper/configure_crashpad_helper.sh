#!/bin/bash -e

#This script exists because cmakes adds extra quotation marks and it is difficult to control.
#Compounded by gn being picky about formatting.

output_dir=$1
args=$2
 
gn gen $output_dir --args="${args}" --script-executable="python3"