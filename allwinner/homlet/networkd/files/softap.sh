#!/bin/sh

wifi_address="/sys/class/net/wlan0/address"
awcast_config="/usr/local/etc/awcast.cfg"
hostapd_config="/etc/hostapd.conf"
device_name=""
ssid=""
default_name="AWCast"

function write_value_to_file()
{
	local cfgkey=$1
	local cfgval=$2
	local cfgfile=$3

	echo "softap.sh: write $cfgkey=$cfgval to $cfgfile"

	cfgval=$(echo -e "$cfgval" | sed -e 's/^\s\+//g' -e 's/\s\+$//g')
	if [ -f $cfgfile ] && [ -n "$(sed -n "/^$cfgkey\s*=/p" $cfgfile)" ]; then
		sed -i "s|^$cfgkey\s*=\s*.*$|$cfgkey=$cfgval|g" $cfgfile
	else
		echo "$cfgkey=$cfgval" >> $cfgfile
	fi
}

function check_ssid()
{
	local str=""
	local mac=""
	local ssid_mac=""

	if [ ! -f ${wifi_address} ]; then
		echo "softap.sh: ${wifi_address} is not exist"
		return;
	fi

	str=`sed -n '/^device_name/p' ${awcast_config}`
	device_name=${str##*=}

	str=`sed -n '/^ssid/p' ${hostapd_config}`
	ssid=${str##*=}

	if [ "x${device_name}" == "x" ]; then
		mac=`cat /sys/class/net/wlan0/address | awk -F : '{print $4 $5 $6}'`
		ssid_mac="${default_name}-${mac}"
		echo "softap.sh: mac=${mac}, ssid_mac=${ssid_mac}"

		write_value_to_file "ssid" ${ssid_mac} ${hostapd_config}
	else
		if [ ${device_name} != ${ssid} ]; then
			write_value_to_file "ssid" ${device_name} ${hostapd_config}
		fi
	fi
}

check_ssid
#ifconfig wlan0 down
#ifconfig wlan0 up
ifconfig wlan0 192.168.5.1

#iptables -t nat -I POSTROUTING -o wlan0 -j MASQUERADE
#iptables -A FORWARD -s 192.168.5.0/24 -j ACCEPT
#iptables -A FORWARD -d 192.168.5.0/24 -j ACCEPT

hostapd /etc/hostapd.conf

