#!/bin/bash
ConfigDir="/etc/HelpSys"
if [[ ! -d ${ConfigDir} ]];then
    mkdir ${ConfigDir}
fi

Rv=`which tmux`
if [[ ${Rv}x == 'x' ]];then
    echo "Please Install tmux on your system first!"
    exit
fi
check_system_info=`uname`

if [[ $check_system_info =~ "Darwin" ]]; then
    sudo cp dohelp /usr/local/bin
else
    sudo cp dohelp /usr/bin
fi


cp ./.teacher.conf.sample ${ConfigDir}/teacher.conf


echo "Install Ok"
