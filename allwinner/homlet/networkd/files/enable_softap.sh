#!/bin/sh

awcast_config="/usr/local/etc/awcast.cfg"
hostapd_config="/etc/hostapd.conf"
share_interface=""
need_kill_wpa=""
ap_wlan=""

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

function get_need_kill_wpa()
{
    local str=""
    local net_key=""

    str=`sed -n '/^interface/p' ${hostapd_config}`
    ap_wlan=${str##*=}
    echo "ap_wlan=${ap_wlan}"

    str=`sed -n '/^net_key/p' ${awcast_config}`
    net_key=${str##*=}
    echo "net_key=${net_key}"

    if [ -z ${net_key} ]; then
        need_kill_wpa=0
    else
        if [ ${net_key} -eq 0 ]; then
            need_kill_wpa=0
        else
            if [ -z ${ap_wlan} ]; then
                need_kill_wpa=0
            else
                if [ ${ap_wlan} == "wlan0" ]; then
                    need_kill_wpa=1
                else
                     need_kill_wpa=0
                fi
            fi
        fi
    fi

    echo "need_kill_wpa=${need_kill_wpa}"
}

function kill_wpa_for_softap()
{
    local pid=99999

    pid=`ps | grep "wpa_supplicant -i${ap_wlan}" | grep -v "grep" | awk 'NR==1{print $1}'`
    if [ ${pid} ]; then
        kill -9 ${pid}
    fi
}

function start_dnsmasq()
{
    local pid=99999

    pid=`ps | grep "dnsmasq" | grep -v "grep" | awk 'NR==1{print $1}'`
    if [ ${pid} ]; then
        echo "----------dnsmasq is exist----------"
    else
    	echo "----------dnsmasq start----------"
    	/etc/init.d/dnsmasq start
    fi
}

function start_hostapd()
{
    local pid=99999

    pid=`ps | grep "hostapd" | grep -v "grep" | awk 'NR==1{print $1}'`
    if [ ${pid} ]; then
        echo "----------hostapd is exist----------"
    else
    	echo "----------hostapd start----------"
    	source /usr/bin/softap.sh &
    fi
}

get_share_interface
get_need_kill_wpa
if [ ${need_kill_wpa} -eq 1 ]; then
	kill_wpa_for_softap
	ifconfig ${ap_wlan} up
fi

start_dnsmasq
sleep 1
start_hostapd