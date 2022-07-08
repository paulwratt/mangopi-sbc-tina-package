#!/bin/sh
# (C) 2008 openwrt.org
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/ts
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event3

# check if media is exist
if [ ! -f /mnt/UDISK/MEDIA ]; then
    cp /data/*  /mnt/UDISK/
    touch /mnt/UDISK/MEDIA
fi


/usr/bin/multimedia-test
