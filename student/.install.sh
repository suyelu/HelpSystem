#!/bin/bash
ConfigDir="/etc/HelpSys"

if [[ ! -d ${ConfigDir} ]]; then
    mkdir ${ConfigDir}
fi
read -p "请输入你的真实姓名:" Name
cp ./.student.conf.sample ${ConfigDir}/student.conf
sed -i 's/XXX/'''${Name}'''/g' ${ConfigDir}/student.conf
cp helpme /usr/bin || cp helpme /usr/local/bin
echo "Install OK"
echo "You can change you config in ${ConfigDir}/student.conf."
