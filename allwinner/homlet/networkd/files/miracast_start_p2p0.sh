#!/bin/sh

awcast_config="/usr/local/etc/awcast.cfg"
share_interface=""
p2p_wlan=""
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

function get_p2p_wlan()
{
    local str=""

    str=`sed -n '/^p2p_wlan/p' ${awcast_config}`
    p2p_wlan=${str##*=}

    if [ -z ${p2p_wlan} ]; then
        p2p_wlan=wlan1
    fi

    echo "p2p_wlan=${p2p_wlan}"
}

function kill_udhcpc_wlan0()
{
    local pid=99999

    pid=`ps | grep "udhcpc -i wlan0" | grep -v "grep" | awk 'NR==1{print $1}'`
    if [ ${pid} ]; then
        kill -9 ${pid}
    fi
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
get_p2p_wlan
kill_udhcpc_wlan0
if [ ${share_interface} -eq 0 ]  || [ ${net_key} -eq 1 ]; then
    kill_wlan0_process
    ifconfig ${p2p_wlan} down
    ifconfig ${p2p_wlan} up
    ifconfig ${p2p_wlan} 0.0.0.0
    wpa_supplicant -Dnl80211 -i${p2p_wlan} -c/etc/p2p_supplicant_gc.conf &
else
	wpa_supplicant -Dnl80211 -i${p2p_wlan} -c/etc/wifi/wpa_supplicant_p2p.conf -I/etc/wifi/wpa_supplicant_overlay.conf -O/etc/wifi/sockets &
fi

