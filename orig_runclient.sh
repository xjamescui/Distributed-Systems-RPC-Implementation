#!/bin/bash

export BINDER_ADDRESS=0
export BINDER_PORT=0

make clean client
./client
