#!/bin/bash
ConfigDir="/etc/HelpSys"
check_system_info=`uname`

if [[ ! -d ${ConfigDir} ]]; then
    mkdir ${ConfigDir}
fi
read -p "请输入你的真实姓名:" Name
cp ./.student.conf.sample ${ConfigDir}/student.conf

if [[ $check_system_info =~ "Darwin" ]]; then
    sed -i '' 's/XXX/'''${Name}'''/g' ${ConfigDir}/student.conf
else
    sed -i 's/XXX/'''${Name}'''/g' ${ConfigDir}/student.conf
fi

cp helpme /usr/bin || cp helpme /usr/local/bin
echo "Install OK"
echo "You can change you config in ${ConfigDir}/student.conf."
