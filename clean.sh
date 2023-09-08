#!/bin/bash

# TODO: maybe try to automatically exclude *all* folders starting with a "." automatically. But this is very tricky and we really only need to exclude .git/ for now.
find . -type f ! -name "*.*" -not -path "./.git/*" -not -path "./.vscode/*" ! -name "LICENSE" | xargs rm
find . -type f -name "*.s" | xargs rm
find . -type f -name "*.gcda" | xargs rm
find . -type f -name "*.gcno" | xargs rm
find . -type f -name "*.gcov" | xargs rm
find . -type f -name "*.info" | xargs rm
find . -type f -name "*.o" | xargs rm
