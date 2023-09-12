#!/bin/bash

# ASAN_OPTIONS="disable_coredump=0:unmap_shadow_on_exit=1:abort_on_error=1"
CC=afl-gcc-fast CXX=afl-g++-fast meson build -Db_coverage=true
meson compile -C build/
