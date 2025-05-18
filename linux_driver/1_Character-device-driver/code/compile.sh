#!/bin/sh
make clean
make
arm-linux-gnueabihf-gcc character_device_app.c -o app