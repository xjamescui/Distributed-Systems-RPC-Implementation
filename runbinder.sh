#!/bin/bash

make clean binder
nohup ./binder | tee addr.temp &

sleep 2

echo "Getting addr and port"
BINDER_AND_PORT=`tail -n 50 addr.temp | grep BINDER`
echo $BINDER_AND_PORT

echo "updating runserver.sh and runclient.sh"
BINDER_AND_PORT_ARR=(${BINDER_AND_PORT// / })
BINDER_IP=${BINDER_AND_PORT_ARR[1]}
BINDER_PORT=${BINDER_AND_PORT_ARR[3]}
echo $BINDER_IP
echo $BINDER_PORT

git update-index --assume-unchanged runserver.sh
git update-index --assume-unchanged runclient.sh

sed -e "s/BINDER_ADDRESS.*/BINDER_ADDRESS=$BINDER_IP/" -e "s/BINDER_PORT=.*/BINDER_PORT=$BINDER_PORT/" orig_runserver.sh > runserver.sh
sed -e "s/BINDER_ADDRESS.*/BINDER_ADDRESS=$BINDER_IP/" -e "s/BINDER_PORT=.*/BINDER_PORT=$BINDER_PORT/" orig_runclient.sh > runclient.sh

echo "press any key to terminate binder"
read terminate
pkill -u `id -u` binder