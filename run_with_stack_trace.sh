#!/bin/bash


build/foo_cc "$1" | tee log.txt
python3 stack_trace_demangler.py log.txt 
