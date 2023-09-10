#!/bin/bash

meson test -C build
cat build/meson-logs/testlog.txt 
