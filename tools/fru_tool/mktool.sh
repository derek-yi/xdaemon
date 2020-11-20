#!/bin/sh

## delete src file
rm -rf *.c
rm -rf *.h

## make link
ln -s ../../service/devm/devm_fru.c devm_fru.c
ln -s ../../service/devm/devm_fru.h devm_fru.h

ln -s ../../driver/misc/drv_i2c.c drv_i2c.c
ln -s ../../driver/misc/drv_i2c.h drv_i2c.h

ln -s ../../common/cJSON.c cJSON.c
ln -s ../../common/cJSON.h cJSON.h

## compile
arm-linux-gnueabihf-gcc -o fru.bin -DFRU_APP devm_fru.c cJSON.c drv_i2c.c

## delete tmp file
rm -rf *.c
rm -rf *.h
rm -rf *.o
