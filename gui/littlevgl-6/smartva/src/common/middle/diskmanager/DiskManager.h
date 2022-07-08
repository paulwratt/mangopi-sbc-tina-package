#ifndef __DEVICE_CHECK_H__
#define __DEVICE_CHECK_H__
#include "list.h"
typedef enum {
	MEDIUM_USB_MASSSTORAGE = 0,
	MEDIUM_SD_CARD,
	MEDIUM_LOCAL_DISK
}Media_type_t;

typedef enum {
	MEDIUM_PLUGIN = 0,
	MEDIUM_PLUGOUT,
	MEDIUM_STATIC_PLUGIN
}Media_operate_t;
#define MOUNT_PATH_MAX_LENGTH	64
typedef struct _DiskInfo_tag {
	unsigned int Major;
	unsigned int Minor;
	Media_operate_t operate;
	Media_type_t MediaType;
	char Volume[MOUNT_PATH_MAX_LENGTH];
	char DeviceName[MOUNT_PATH_MAX_LENGTH];
	char MountPoint[MOUNT_PATH_MAX_LENGTH];
} DiskInfo_t;

typedef void (*DiskManager_CallBack)(DiskInfo_t *);
typedef struct _RegisterInfo {
	int status;
	char Cur_Win[64];
	DiskManager_CallBack CallBackFunction;
} hotplug_message_focus_win_t;

int DiskManager_Init(void);
int DiskManager_detect(void);
unsigned int DiskManager_GetDiskNum(void);
DiskInfo_t *DiskManager_GetDiskInfoByIndex(unsigned int Index);
int DiskManager_Register_StaticDisk(DiskInfo_t *DeviceInfo);
int DiskManager_Register(hotplug_message_focus_win_t *RegisterInfo);
int DiskManager_UnRegister(hotplug_message_focus_win_t *RegisterInfo);

#endif
