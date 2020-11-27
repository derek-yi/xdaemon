#!/bin/sh

## delete src file
rm -rf *.c
rm -rf *.h

## copy file
ln -s ../../driver/misc/drv_i2c.c drv_i2c.c
ln -s ../../driver/misc/drv_i2c.h drv_i2c.h

## compile
arm-linux-gnueabihf-gcc -o i2ctest -DAPP_TEST drv_i2c.c

## delete tmp file
rm -rf *.c
rm -rf *.h
rm -rf *.o
