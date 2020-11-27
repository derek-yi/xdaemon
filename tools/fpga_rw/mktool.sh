#!/bin/sh

## compile
arm-linux-gnueabihf-gcc -o devmem devmem.c

## delete tmp file
rm -rf *.o
