#!/bin/bash
ConfigDir="/etc/HelpSys"
User=Helper
source .master.conf.sample
for i in `seq ${StartPort} $[ ${StartPort} + ${ConSize} ]`;do
    ufw allow ${i}
done

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
