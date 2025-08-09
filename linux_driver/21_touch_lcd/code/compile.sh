#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc -march=armv7-a -mfpu=neon -mfloat-abi=hard icm20608_app.c -o icm20608_app
sudo cp -f tsc.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f icm20608_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/