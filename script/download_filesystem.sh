#!/bin/bash

echo $(ps aux | grep idf.py | grep monitor | awk '{print $2}')
IDS=$(ps aux | grep idf.py | grep monitor | awk '{print $2}')

for PROCESS in $IDS; do
    kill $PROCESS
done

esptool.py -p /dev/ttyUSB0 -b 230400 read_flash 0x00310000 409600 filesystem_content.bin

rmdir ./embedded_flash

./mkspiffs -u ./embedded_flash filesystem_content.bin

rm filesystem_content.bin
