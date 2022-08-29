#!/bin/bash

echo $(ps aux | grep idf.py | grep monitor | awk '{print $2}')
IDS=$(ps aux | grep idf.py | grep monitor | awk '{print $2}')

for PROCESS in $IDS; do
    kill $PROCESS
done

esptool.py -p /dev/ttyUSB0 -b 1000000 read_flash 0x0035b000 0x10000 filesystem_content.bin

rmdir ./embedded_flash

script/mkspiffs -u ./embedded_flash filesystem_content.bin

rm filesystem_content.bin
