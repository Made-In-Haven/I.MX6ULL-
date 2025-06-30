#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc atomicAPP.c -o atomicAPP -lpthread
sudo cp -f atomic_led_dev.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f atomicAPP ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/