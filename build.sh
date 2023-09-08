#!/bin/bash

meson build -Db_coverage=true
meson compile -C build/
