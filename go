#!/bin/bash

make clean && make usb && sudo lm4flash -v -E -S 0x4000 ./gcc/cardea_app_usb.bin
