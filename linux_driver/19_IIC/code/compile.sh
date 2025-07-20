#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc ap3216c_app.c -o ap3216c_app
sudo cp -f ap3216c.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f ap3216c_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/