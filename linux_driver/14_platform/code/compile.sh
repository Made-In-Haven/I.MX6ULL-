#!/bin/sh
set -v
make clean
make
arm-linux-gnueabihf-gcc async_notification_app.c -o async_notification_app
sudo cp -f platform_led_dev.ko ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/
sudo cp -f async_notification_app ~/linux/imx6ull/nfs/rootfs/lib/modules/4.1.15/