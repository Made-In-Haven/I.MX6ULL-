#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc key_app.c -o key_app
sudo cp -f key_int_dev.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f key_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/