#!/bin/bash

export BINDER_ADDRESS=129.97.167.43
export BINDER_PORT=10000

make clean server
./server
