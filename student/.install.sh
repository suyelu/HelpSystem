#!/bin/bash
ConfigDir="/etc/HelpSys"
check_system_info=`uname`
username=`echo $(logname)`

if [[ ${username} == "root" ]];then
    echo "Can't run this using root user!"
    exit
fi

Rv=`which tmux`
if [[ ${Rv}x == 'x' ]];then
    echo "Please Install tmux on your system first!"
    exit
fi

PubKey=`cat ~/.ssh/id_rsa.pub | cut -d " " -f 2`
if [[ -z ${PubKey} ]]; then
    echo "Please check you public key!"
fi


cat ~/.ssh/authorized_keys | grep ${PubKey} >/dev/null

if [[ ! $? -eq 0 ]];then
    cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
fi

if [[ ! -d ${ConfigDir} ]]; then
    mkdir ${ConfigDir}
fi
read -p "请输入你的真实姓名:" Name
sudo cp ./.student.conf.sample ${ConfigDir}/student.conf
sudo sed -i  's/XXX/'''${Name}'''/g' ${ConfigDir}/student.conf
sudo sed -i  's#PWD#'''${HOME}'''#g' ${ConfigDir}/student.conf
chmod a+x ./check_key.sh
if [[ $check_system_info =~ "Darwin" ]]; then
    sudo cp helpme /usr/local/bin
    sudo cp check_key.sh /usr/local/bin/check_key
else
    sudo cp helpme /usr/bin
    sudo cp check_key.sh /usr/bin/check_key
fi

echo "Install OK"
echo "You can change you config in ${ConfigDir}/student.conf."
