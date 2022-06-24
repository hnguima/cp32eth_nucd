#!/bin/bash

server_files=( $( ls -LR ./http_server ) )

## comprime os arquivos
for i in ${server_files[@]}
do
    if [[ $i == *".html"* ]]
    then
        gzip -9 -c ./http_server/$i > ./flash_partitions/server/$i
    fi
    if [[ $i == *".svg"* ]]
    then
        mkdir -p ./flash_partitions/server/img
        gzip -9 -c ./http_server/img/$i > ./flash_partitions/server/img/$i
    fi
    if [[ $i == *".css"* ]]
    then
        mkdir -p ./flash_partitions/server/style
        gzip -9 -c ./http_server/style/$i > ./flash_partitions/server/style/$i
    fi
    if [[ $i == *".js"* ]]
    then
        mkdir -p ./flash_partitions/server/js
        gzip -9 -c ./http_server/js/$i > ./flash_partitions/server/js/$i
    fi
done
