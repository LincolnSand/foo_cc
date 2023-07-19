#!/bin/bash

# TODO: maybe try to automatically exclude *all* folders starting with a "." automatically. But this is very tricky and we really only need to exclude .git/ for now.
find . -type f ! -name "*.*" -not -path "./.git/*" -not -path "./.vscode/*" | xargs rm