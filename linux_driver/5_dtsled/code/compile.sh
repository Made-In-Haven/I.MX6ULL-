#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc dtsled_app.c -o dtsled_app
sudo cp -f dtsled.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f dtsled_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/