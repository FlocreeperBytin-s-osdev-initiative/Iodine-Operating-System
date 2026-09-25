#!/usr/bin/env bash
set -e

echo "=== Building Iodine Operating System ==="
make clean
make all
echo "=== Build Successful! ==="
echo "Artifacts generated in bin/:"
ls -lh bin/
