#!/bin/bash
ConfigDir="/etc/HelpSys"
User=Helper
if [[ ! -d ${ConfigDir} ]];then
    mkdir ${ConfigDir}
fi
cp ./.master.conf.sample ${ConfigDir}/master.conf
userdel -r ${User}
useradd -m ${User}
cp -ar .ssh /home/${User}
chown -R ${User}:${User} /home/${User}
cp -ar .ssh ${ConfigDir}
cp helper /usr/bin
echo "Install OK"
