#!/bin/bash
ConfigDir="/etc/HelpSys"
if [[ ! -d ${ConfigDir} ]];then
    mkdir ${ConfigDir}
fi

cp ./.teacher.conf.sample ${ConfigDir}/teacher.conf

cp dohelp /usr/bin

echo "Install Ok"