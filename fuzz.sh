#!/bin/bash

AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 afl-fuzz -i test_programs/ -o out -m none -- build/foo_cc @@
