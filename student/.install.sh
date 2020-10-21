#!/bin/bash
ConfigDir="/etc/HelpSys"
check_system_info=`uname`
username=`whoami`
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
sudo sed -i  's/PWD/'''${HOME}'''/g' ${ConfigDir}/student.conf

if [[ $check_system_info =~ "Darwin" ]]; then
    sudo cp helpme /usr/local/bin
else
    sudo cp helpme /usr/bin
fi

echo "Install OK"
echo "You can change you config in ${ConfigDir}/student.conf."
