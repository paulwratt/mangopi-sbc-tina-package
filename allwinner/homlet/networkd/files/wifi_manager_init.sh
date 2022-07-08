#!/bin/sh

awcast_config="/usr/local/etc/awcast.cfg"
share_interface=""
net_key=""

function get_net_key()
{
    local str=""

    str=`sed -n '/^net_key/p' ${awcast_config}`
    net_key=${str##*=}

    if [ -z ${net_key} ]; then
        net_key=0
    fi

    echo "net_key=${net_key}"
}

function get_share_interface()
{
    local str=""

    str=`sed -n '/^share_interface/p' ${awcast_config}`
    share_interface=${str##*=}

    if [ -z ${share_interface} ]; then
        share_interface=0
    fi

    echo "share_interface=${share_interface}"
}

function kill_wlan0_process()
{
    local pid=99999

    pid=`ps | grep "wpa_supplicant -iwlan0" | grep -v "grep" | awk 'NR==1{print $1}'`
    if [ ${pid} ]; then
        kill -9 ${pid}
    fi
}

get_share_interface
get_net_key
if [ ${share_interface} -eq 0 ]  || [ ${net_key} -eq 1 ]; then
	kill_wlan0_process
	ifconfig wlan0 down
	ifconfig wlan0 up
	ifconfig wlan0 0.0.0.0
	wpa_supplicant -iwlan0 -Dnl80211 -c/etc/wifi/wpa_supplicant.conf -I/etc/wifi/wpa_supplicant_overlay.conf -O/etc/wifi/sockets &
fi
