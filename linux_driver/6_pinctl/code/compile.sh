#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc chardev_app.c -o led_app
sudo cp -f pinctl_led_dev.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f led_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/