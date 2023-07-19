#!/bin/bash

# we need to make `foo.c` be a param and then get all occurrences of `foo` from it so we can use the test script for the test suite.
/home/link/foo_cc/src/build_script.sh
/home/link/foo_cc/src/main_out "$1"
file_without_extension="$(dirname "$1")/$(basename "$1" | cut -f 1 -d .)"
gcc -march=x86-64 "$file_without_extension".s -o "$file_without_extension"
