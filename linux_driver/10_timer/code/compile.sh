#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc timer_app.c -o timer_app
sudo cp -f timer_dev.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f timer_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/