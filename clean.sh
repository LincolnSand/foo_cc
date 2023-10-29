#!/bin/bash

rm -rf build
rm -f log.txt
find . -type f ! -name "*.*" -not -path "./.git/*" -not -path "./.vscode/*" -not -path "./subprojects/*" ! -name "LICENSE" | xargs rm
find . -type f -name "*.s" | xargs rm
