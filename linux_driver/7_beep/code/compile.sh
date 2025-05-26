#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc beep_app.c -o led_app
sudo cp -f beep_dev.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f led_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/