#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc input_key_app.c -o input_key_app
sudo cp -f input_key_dev.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f input_key_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/