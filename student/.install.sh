#!/bin/bash
ConfigDir="/etc/HelpSys"

if [[ ! -d ${ConfigDir} ]]; then
    mkdir ${ConfigDir}
fi
read -p "请输入你的真实姓名:" Name
sed -i 's/XXX/'''${Name}'''/g' ./.student.conf.sample
cp ./.student.conf.sample ${ConfigDir}/student.conf
cp helpme /usr/bin
echo "Install OK"
echo "You can change you config in ${ConfigDir}/student.conf."
