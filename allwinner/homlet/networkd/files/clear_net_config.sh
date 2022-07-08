#!/bin/sh

config_file1="/etc/wifi/wpa_supplicant.conf"
config_back1="/etc/wifi/wpa_supplicant_back.conf"

config_file2="/etc/wifi/wpa_supplicant_p2p.conf"
config_back2="/etc/wifi/wpa_supplicant_p2p_back.conf"

awcast_config="/usr/local/etc/awcast.cfg"
share_interface=""
config_file=""
config_back=""

function get_share_interface()
{
    local str=""

    str=`sed -n '/^share_interface/p' ${awcast_config}`
    share_interface=${str##*=}

    if [ -z ${share_interface} ]; then
        share_interface=0
    fi
}

get_share_interface
if [ ${share_interface} -eq 0 ]; then
    config_file=${config_file1}
    config_back=${config_back1}
else
    config_file=${config_file2}
    config_back=${config_back2}
fi
echo "config_file=${config_file}"
echo "config_back=${config_back}"

if [ -f ${config_back} ]; then
	if [ -f ${config_file} ]; then
	    rm ${config_file}
	fi
	cp ${config_back} ${config_file}
else
    echo "${config_back} is not exist, can not clean"
fi
