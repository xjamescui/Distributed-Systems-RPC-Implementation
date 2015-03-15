#!/bin/bash

# exit when error
set -e

echo "==test_helper=="
make helper.o
g++ -g test_helper.c helper.o -o test_helper
./test_helper
# gdb ./test_helper
# valgrind --leak-check=full --track-origins=yes ./test_helper
rm test_helper
make clean
echo "==test_helper passed!=="

echo "==test_database=="
make binder_database.o
g++ test_database.c binder_database.o -o test_database
valgrind --leak-check=full --track-origins=yes ./test_database
rm test_database
make clean
echo "==test_database passed!=="

echo "==test_skeleton_database=="
make
make SkeletonDatabase.o
g++ test_skeleton_database.cc SkeletonDatabase.o server_functions.o server_function_skels.o helper.o -o test_skeleton_database
# valgrind --leak-check=full --track-origins=yes ./test_skeleton_database
./test_skeleton_database
rm test_skeleton_database
make clean
echo "==test_skeleton_database passed!=="

