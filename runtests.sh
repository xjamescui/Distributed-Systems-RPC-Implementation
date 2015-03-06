#!/bin/bash

# exit when error
set -e

echo "==test_helper=="
make helper.o
g++ test_helper.c helper.o -o test_helper
./test_helper
rm test_helper
make clean
echo "==test_helper passed!=="



