#!/bin/bash

# we need to make `foo.c` be a param and then get all occurrences of `foo` from it so we can use the test script for the test suite.
build/foo_cc "$1"
file_without_extension="$(dirname "$1")/$(basename "$1" | cut -f 1 -d .)"
gcc -march=x86-64 "$file_without_extension".s -o "$file_without_extension"
