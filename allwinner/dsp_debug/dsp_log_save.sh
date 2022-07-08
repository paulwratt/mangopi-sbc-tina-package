#!/bin/sh

while true
do
	dsp_debug -d /dev/dsp_debug0 -r >> dsp0_log.txt
	dsp_debug -d /dev/dsp_debug1 -r >> dsp1_log.txt
	usleep 200000
done
