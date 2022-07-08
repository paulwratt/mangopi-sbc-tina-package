#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <signal.h>
#include "va_power.h"

#define BATTERYDEV_NAME   "/sys/devices/virtual/input/input0/tpadc"

int va_power_get_battery_level(void)
{
    int level = -1;
	char pwr_val[5];
	int ival = 0;
	int pwr_fd;
	int ret;

	pwr_fd = open(BATTERYDEV_NAME, O_RDONLY);
	if (pwr_fd == -1) {
		printf("%s:open disp handle is not exist!\r\n", __func__);
		return -1;
	}
	memset(pwr_val, 0, sizeof(pwr_val));
	ret = read(pwr_fd, pwr_val, 5);
	if (ret < 0) {
		printf("read power val failed!\n");
		level = -1;
		goto error_out;
	}
	ival = atoi(pwr_val);
	//printf("%s:read power val is %d\r\n", __func__, ival);

	if (ival <= 3450) {
		level = POWER_LEVEL_0;
	}
	else if ((ival > 3450) && (ival <= 3536)) {
		level = POWER_LEVEL_1;
	}
	else if ((ival > 3536) && (ival <= 3681)) {
		level = POWER_LEVEL_2;
	}
	else if ((ival > 3681) && (ival <= 3824)) {
		level = POWER_LEVEL_3;
	}
	else if ((ival > 3824) && (ival <= 3919)) {
		level = POWER_LEVEL_4;
	}
	else if (ival > 3919) {
		level = POWER_LEVEL_5;
	}

error_out:

	close(pwr_fd);

	return level;
}

int va_power_is_charging(void)
{
	int pwr_type = -1;
	char val[2];
	int ival = 0;
	int pwr_fd;
	int ret;

	pwr_fd = open(BATTERYDEV_NAME, O_RDONLY);
	if(pwr_fd == -1){
		printf("%s:open disp handle is not exist!\r\n", __func__);
		return -1;
	}

	memset(val, 0, sizeof(val));
	ret = read(pwr_fd, val, 1);
	if(ret < 0){
		printf("read power val failed!\n");
		pwr_type = -1;
		goto error_out;
	}
	ival = atoi(val);
	switch(ival){
	case 0:
		pwr_type = POWER_BAT_ONLY;
		break;
	case 1:
		pwr_type = POWER_CHARGER_LINK;
		break;
	case 2:
		pwr_type = POWER_PC_LINK;
		break;
	default:
		printf("not know the type.\n");
		pwr_type = -1;
		break;
	}

error_out:

	close(pwr_fd);

	return pwr_type;
}

/* 1 低电量 0 正常电量*/
int  va_power_is_low(void)
{
	int level = 0;

	level = va_power_get_battery_level();
	if(level < POWER_LEVEL_2)
		return 1;
	else
		return 0;
}

int va_power_power_off(void)
{
	system("poweroff");
	return 0;
}
