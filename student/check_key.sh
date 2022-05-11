#!/bin/bash
key_file=$1

Msg=`cat ${key_file} | cut -d " " -f 2 | cut -c 1-80`
sed -i -e '/'''${Msg}'''/d' ~/.ssh/authorized_keys
cat  ${key_file} >> ~/.ssh/authorized_keys 
