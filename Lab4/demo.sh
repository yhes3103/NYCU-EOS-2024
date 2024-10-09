#!/bin/sh

set -x #prints each command and its arguments to the terminal before executing it
# set -e #Exit immediately if a command exits with a non-zero status

rmmod -f mydev
insmod mydev.ko
mknod /dev/my_device c 255 0

./writer 'Devon' & #run in subshell
./reader 192.168.0.22 8888 /dev/my_device
