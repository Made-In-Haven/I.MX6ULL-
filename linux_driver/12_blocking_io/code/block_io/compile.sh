#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc block_io_app.c -o block_io_app
sudo cp -f block_io_dev.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f block_io_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/