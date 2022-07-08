#include <sys/epoll.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/mount.h>
#include <pthread.h>
#include <sys/stat.h>

#include "dbList.h"
#include "smt_config.h"
#include "DiskManager.h"

#define MAX_EPOLL_EVENTS 40
#define UEVENT_MSG_LEN 2048
#define SYS_BLOCK_DEV_PATH "/sys/block"

#define MOUNTPOINT_MAX_LENGTH	(64)
//#define DISKMANAGER_DEBUG
#define MMC_DEVICE_MAJOR	(179)
#define USB_DEVICE_MAJOR	(8)

#ifdef DISKMANAGER_DEBUG
#define LOG_INFO(fmt, ...) do { \
	   printf("%s %d %s ",__FILE__,__LINE__,__func__);\
	   printf(fmt,##__VA_ARGS__);}\
	   while(0)
#else
#define LOG_INFO(fmt, ...) \
       do {;} while (0)
#endif

typedef struct _MountPoint_Info {
	char NeedCreate_Clean;
	char MountPoint[MOUNTPOINT_MAX_LENGTH];
} MountPoint_Info_t;

static int uevent_fd = 0;
static int epollfd = 0;
static int eventct = 0;

static db_list_t *disk_info_list = NULL;
static db_list_t *hotplug_message_list = NULL;
static db_list_t *hotplug_message_focus_app_list = NULL;

static MountPoint_Info_t UsbMountPoint[] = {
	{0, "/mnt/exUDISK"},
	{1, "/tmp/exUDISK_0"},
	{1, "/tmp/exUDISK_1"},
	{1, "/tmp/exUDISK_2"},
	{1, "/tmp/exUDISK_3"},
	{1, "/tmp/exUDISK_4"},
	{1, "/tmp/exUDISK_5"},
	{1, "/tmp/exUDISK_6"}
};
static MountPoint_Info_t SdcardMountPoint[] = {
	{0, "/mnt/SDCARD"},
	{1, "/tmp/SDCARD_0"},
	{1, "/tmp/SDCARD_1"},
	{1, "/tmp/SDCARD_2"},
	{1, "/tmp/SDCARD_3"},
	{1, "/tmp/SDCARD_4"},
	{1, "/tmp/SDCARD_5"},
	{1, "/tmp/SDCARD_6"}
};
static char *FileSystemType[] = {
	"vfat",
	"ntfs-3g",
	"exfat"
};

static int DiskManager_MountDevice(DiskInfo_t *DeviceInfo, int bootcheck);
static int DiskManager_UnMountDevice(DiskInfo_t *DeviceInfo);
static int DiskManager_BootCheck(void);
static int DiskManager_Clean_MountPoint(void);
static int DiskManager_Free_MountPoint(DiskInfo_t *DeviceInfo);
static void DiskManager_Del_DiskInfo(DiskInfo_t *DeviceInfo);


static int _DiskManager_BootCheck(char *FilePath) {
	FILE *fp = NULL;
	char buf[256] = {0};
	DiskInfo_t DeviceInfo;

	fp = fopen(FilePath, "r");
	if (NULL == fp) {
		LOG_INFO("failed to open %s\n", buf);
		return -1;
	}

	memset(&DeviceInfo, 0x00, sizeof(DeviceInfo));
	while(!feof(fp))
	{
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf) - 1, fp); // 包含了换行符
		LOG_INFO("buf:%s\n", buf);
//		if (!strncmp(buf, "DEVTYPE=partition", strlen("DEVTYPE=partition"))) {
//			DiskManager_MountDevice(&DeviceInfo, 1);
//			fclose(fp);
//			return 0;
//		} else
		if (!strncmp(buf, "DEVNAME=", strlen("DEVNAME="))) {
			//strncpy(DeviceInfo.DeviceName, "/dev/", strlen("/dev/"));
			strcpy(DeviceInfo.DeviceName, "/dev/");
			if (strchr(buf, '\n') != NULL) {
				strncat(DeviceInfo.DeviceName, buf + strlen("DEVNAME="), strchr(buf, '\n') - (buf + strlen("DEVNAME=")));
			} else {
				com_err("strchr(buf, '\n') null\n");
				fclose(fp);
				return 0;
			}
			DiskManager_MountDevice(&DeviceInfo, 1);
			fclose(fp);
			return 0;
		} else if (!strncmp(buf, "MAJOR=", strlen("MAJOR="))) {
			DeviceInfo.Major = atoi(buf + strlen("MAJOR="));
			if (DeviceInfo.Major == USB_DEVICE_MAJOR) {
				DeviceInfo.MediaType = MEDIUM_USB_MASSSTORAGE;
			} else if (DeviceInfo.Major == MMC_DEVICE_MAJOR) {
				DeviceInfo.MediaType = MEDIUM_SD_CARD;
			}
		} else if (!strncmp(buf, "MINOR=", strlen("MINOR="))) {
			DeviceInfo.Minor = atoi(buf + strlen("MINOR="));
		}
	}
	fclose(fp);
	return 0;
}
static int IS_DIR(char *path)
{
	int  fd = 0;
	struct stat buf;

	fd = open(path, O_RDONLY);
	if(fd < 0) {
		LOG_INFO("err %s %s\n", path, strerror(errno));
	}

	memset(&buf, 0x00, sizeof(struct stat));
	fstat(fd, &buf);
	close(fd);
	LOG_INFO("file_path:%s S_ISDIR(buf.st_mode):%d\n", path, S_ISDIR(buf.st_mode));
	return S_ISDIR(buf.st_mode);
}
static int DiskManager_BootCheck(void) {
	DIR *pCurDir = NULL, *TmpDir = NULL;
	struct dirent *pDirent = NULL, *Tmp_Dirent = NULL;
	char file_path[256] = {0};

	pCurDir = opendir(SYS_BLOCK_DEV_PATH);
	if (NULL == pCurDir) {
		return -1;
	}
	chdir(SYS_BLOCK_DEV_PATH);
	while (NULL != (pDirent = readdir(pCurDir))) {
		int removable = 0;
		FILE *fp = NULL;
		snprintf(file_path, sizeof(file_path), "%s", (char*)pDirent->d_name);
		LOG_INFO("file_path:%s\n", file_path);
		chdir(SYS_BLOCK_DEV_PATH);
		if (IS_DIR(file_path) && pDirent->d_name[0] != '.') {
			memset(file_path, 0x00, sizeof(file_path));
			snprintf(file_path, sizeof(file_path), "%s/removable", (char*)pDirent->d_name);
			fp = fopen(file_path, "r");
			if (NULL == fp) {
				com_err("failed to open %s\n", file_path);
				return -1;
			}
			fscanf(fp, "%d", &removable);
			fclose(fp);
			LOG_INFO("file_path:%s\n", file_path);
			if (removable == 1 || 0 == strncmp(file_path, "mmcblk", strlen("mmcblk")))
			{
				LOG_INFO("file_path:%s\n", (char*)pDirent->d_name);
				TmpDir = opendir((char*)pDirent->d_name);
				if (NULL == TmpDir) {
					return -1;
				}
				chdir((char*)pDirent->d_name);
				while (NULL != (Tmp_Dirent = readdir(TmpDir))) {
					LOG_INFO("file_path:%s\n", (char*)Tmp_Dirent->d_name);
					if (IS_DIR((char*)Tmp_Dirent->d_name) && Tmp_Dirent->d_name[0] != '.' &&
						!strncmp((char*)pDirent->d_name, (char*)Tmp_Dirent->d_name, strlen((char*)pDirent->d_name))) {
						memset(file_path, 0x00, sizeof(file_path));
						snprintf(file_path, sizeof(file_path), "%s/%s", (char*)Tmp_Dirent->d_name, "uevent");
						LOG_INFO("file_path:%s\n", file_path);
						_DiskManager_BootCheck(file_path);
					} else if ((!IS_DIR((char*)Tmp_Dirent->d_name)) && (strcmp(Tmp_Dirent->d_name, "uevent") == 0)){
						_DiskManager_BootCheck((char*)Tmp_Dirent->d_name);
					}
				}
				chdir("..");
			}
		}
	}
	chdir("..");
	return 0;
}
static int DiskManager_Check_StaticDisk(DiskInfo_t *StaticDiskInfo) {
	char buf[2048] = {0};

    FILE *fp = fopen("/proc/mounts", "r");
    if (NULL == fp) {
        com_err("failed to open /proc/mounts\n");
        return 1;
    }
    while(!feof(fp))
    {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf) - 1, fp); // 包含了换行符
		LOG_INFO("buf:%s\n", buf);
		if (!strncmp(buf, StaticDiskInfo->DeviceName, strlen(StaticDiskInfo->DeviceName)))	{
			fclose(fp);
			return 0;
		}
    }
	LOG_INFO("buf end\n");
	fclose(fp);
	return -1;
}

static int diskmanager_compare_mountpoint(void *data, void *compare_param)
{
	DiskInfo_t *disk_tmp = (DiskInfo_t *)data;
	char *MountPoint = (char *)compare_param;

	if (strlen(disk_tmp->MountPoint) == strlen(MountPoint) &&
		0 == strncmp(disk_tmp->MountPoint, MountPoint, strlen(MountPoint))) {
		return 0;
	}
	return 1;
}

static int diskmanager_compare_diskinfo(void *data, void *compare_param)
{
	DiskInfo_t *disk_tmp = (DiskInfo_t *)data;
	DiskInfo_t *disk_compare_info = (DiskInfo_t *)compare_param;

//	com_err("DeviceName:%s\n", disk_tmp->DeviceName);
//	com_err("MountPoint:%s\n", disk_tmp->MountPoint);
	if (strlen(disk_tmp->DeviceName) == strlen(disk_compare_info->DeviceName) &&
		0 == strncmp(disk_tmp->DeviceName, disk_compare_info->DeviceName, strlen(disk_compare_info->DeviceName)) &&
		strlen(disk_tmp->MountPoint) == strlen(disk_compare_info->MountPoint) &&
		0 == strncmp(disk_tmp->MountPoint, disk_compare_info->MountPoint, strlen(disk_compare_info->MountPoint))) {
		return 0;
	}
	return 1;
}

static int diskmanager_compare_diskinfo_device(void *data, void *compare_param)
{
	DiskInfo_t *disk_tmp = (DiskInfo_t *)data;
	DiskInfo_t *disk_compare_info = (DiskInfo_t *)compare_param;

//	com_err("DeviceName:%s\n", disk_tmp->DeviceName);
//	com_err("MountPoint:%s\n", disk_tmp->MountPoint);
	if (strlen(disk_tmp->DeviceName) == strlen(disk_compare_info->DeviceName) &&
		0 == strncmp(disk_tmp->DeviceName, disk_compare_info->DeviceName, strlen(disk_compare_info->DeviceName))) {
		return 0;
	}
	return 1;
}

static void DiskManager_Filter_HotPlug_Message_Info(DiskInfo_t *DeviceInfo) {
	__db_list_search_and_pop(hotplug_message_list, DeviceInfo, diskmanager_compare_diskinfo);
}
static void DiskManager_HotPlug_Message_Info(DiskInfo_t *DeviceInfo) {
	__db_list_put_tail(hotplug_message_list, DeviceInfo);
}

static DiskInfo_t *DiskManager_Find_ItemInfo_by_devicename(DiskInfo_t *DeviceInfo) {
	DiskInfo_t *disk_tmp = NULL;

	disk_tmp = __db_list_search_node(disk_info_list, DeviceInfo, diskmanager_compare_diskinfo_device);

	return disk_tmp;
}

static int DiskManager_MountPoint_Is_Used(const char *MountPoint) {
	DiskInfo_t *disk_tmp = NULL;

	disk_tmp = __db_list_search_node(disk_info_list, (void *)MountPoint, diskmanager_compare_mountpoint);
	if (NULL != disk_tmp) {
		return 1;
	}
	return 0;
}
static void DiskManager_Add_DiskInfo(DiskInfo_t *DeviceInfo) {
	__db_list_put_tail(disk_info_list, DeviceInfo);
}

static void DiskManager_Del_DiskInfo(DiskInfo_t *DeviceInfo) {
	DiskInfo_t *disk_tmp = NULL;
	disk_tmp = __db_list_search_and_pop(disk_info_list, DeviceInfo, diskmanager_compare_diskinfo);
}

static int DiskManager_Clean_MountPoint(void) {
	int Index = 0;
	FILE *fp = NULL;
	char buf[256] = {0};

	fp = fopen("/proc/sys/kernel/hotplug", "wb+");
	if (NULL != fp) {
		strcpy(buf, "Not Need Mdev HotPlug");
		fwrite(buf, sizeof(char), strlen(buf), fp);
		fclose(fp);
	}

	for (;Index < sizeof(UsbMountPoint)/sizeof(UsbMountPoint[0]); Index++) {
		umount(UsbMountPoint[Index].MountPoint);
		if (UsbMountPoint[Index].NeedCreate_Clean) {
			char Cmd[64] = {0};
			sprintf(Cmd, "rm -rf %s", UsbMountPoint[Index].MountPoint);
			system(Cmd);
		}
	}

	Index = 0;
	for (;Index < sizeof(SdcardMountPoint)/sizeof(SdcardMountPoint[0]); Index++) {
		umount(SdcardMountPoint[Index].MountPoint);
		if (SdcardMountPoint[Index].NeedCreate_Clean) {
			char Cmd[64] = {0};
			sprintf(Cmd, "rm -rf %s", SdcardMountPoint[Index].MountPoint);
			system(Cmd);
		}
	}
	return 0;
}

static int DiskManager_GetMountPoint(DiskInfo_t *DeviceInfo) {
	int Index = 0;
	if (DeviceInfo->MediaType == MEDIUM_USB_MASSSTORAGE) {
		for (;Index < sizeof(UsbMountPoint)/sizeof(UsbMountPoint[0]); Index++) {
			if (DiskManager_MountPoint_Is_Used(UsbMountPoint[Index].MountPoint) == 0) {
				if (UsbMountPoint[Index].NeedCreate_Clean) {
					char Cmd[64] = {0};
					sprintf(Cmd, "mkdir -p %s", UsbMountPoint[Index].MountPoint);
					system(Cmd);
				}
				strcpy(DeviceInfo->MountPoint, UsbMountPoint[Index].MountPoint);
				return 0;
			}
		}
	} else if (DeviceInfo->MediaType == MEDIUM_SD_CARD) {
		for (;Index < sizeof(SdcardMountPoint)/sizeof(SdcardMountPoint[0]); Index++) {
			if (DiskManager_MountPoint_Is_Used(SdcardMountPoint[Index].MountPoint) == 0) {
				if (SdcardMountPoint[Index].NeedCreate_Clean) {
					char Cmd[64] = {0};
					sprintf(Cmd, "mkdir -p %s", SdcardMountPoint[Index].MountPoint);
					system(Cmd);
				}
				strcpy(DeviceInfo->MountPoint, SdcardMountPoint[Index].MountPoint);
				return 0;
			}
		}
	}
	return -1;
}
static int DiskManager_Free_MountPoint(DiskInfo_t *DeviceInfo){
	int Index = 0;
	if (DeviceInfo->MediaType == MEDIUM_USB_MASSSTORAGE) {
		for (;Index < sizeof(UsbMountPoint)/sizeof(UsbMountPoint[0]); Index++) {
			if (strcmp(DeviceInfo->MountPoint, UsbMountPoint[Index].MountPoint) == 0) {
				if (UsbMountPoint[Index].NeedCreate_Clean) {
					char Cmd[64] = {0};
					sprintf(Cmd, "rm -rf %s", UsbMountPoint[Index].MountPoint);
					system(Cmd);
				}
				return 0;
			}
		}
	} else if (DeviceInfo->MediaType == MEDIUM_SD_CARD) {
		for (;Index < sizeof(SdcardMountPoint)/sizeof(SdcardMountPoint[0]); Index++) {
			if (strcmp(DeviceInfo->MountPoint, UsbMountPoint[Index].MountPoint) == 0) {
				if (SdcardMountPoint[Index].NeedCreate_Clean) {
					char Cmd[64] = {0};
					sprintf(Cmd, "rm -rf %s", SdcardMountPoint[Index].MountPoint);
					system(Cmd);
				}
				return 0;
			}
		}
	}
	return -1;
}

static int DiskManager_Check_DiskMountStatus(DiskInfo_t *DeviceInfo) {
	char buf[2048] = {0};

    FILE *fp = fopen("/proc/mounts", "r");
    if (NULL == fp) {
        com_err("failed to open /proc/mounts\n");
        return 1;
    }
    while(!feof(fp))
    {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf) - 1, fp); // 包含了换行符
		LOG_INFO("buf:%s\n", buf);
		if (!strncmp(buf, DeviceInfo->DeviceName, strlen(DeviceInfo->DeviceName)))	{
			char str[64] = {0}, *temp;
			sprintf(str, "%s", DeviceInfo->DeviceName);
			LOG_INFO("str:%s\n", str);
			if (strstr(buf, str) != NULL) {
				temp = buf + strlen(str);
				LOG_INFO("temp:%s\n", buf);
				//LOG_INFO("strchr(temp, ' ') - temp:%d\n", strchr(temp, ' ') - temp);
				strncpy(DeviceInfo->MountPoint, temp, strchr(temp, ' ') - temp);
				LOG_INFO("MountPoint:%s\n", DeviceInfo->MountPoint);
				fclose(fp);
				return 1;
			}
			fclose(fp);
			return 0;
		}
    }
	LOG_INFO("buf end\n");
	fclose(fp);
	return 0;
}
static int diskmanager_get_disk_volume(DiskInfo_t *DeviceInfo)
{
	char *temp = NULL;
    FILE *stream = NULL;
    char str_buf[256] = {0};

	sprintf(str_buf, "blkid -s LABEL %s", DeviceInfo->DeviceName);
    stream = popen(str_buf, "r");
    if(!stream)
		return -1;
	memset(str_buf, 0x00, sizeof(str_buf));
    fread(str_buf, sizeof(char), sizeof(str_buf), stream);
    pclose(stream);
	temp = strstr(str_buf, "LABEL=");
	if (temp) {
		strcpy(DeviceInfo->Volume, temp + strlen("LABEL="));
	}
	return 0;
}
static int DiskManager_MountDevice(DiskInfo_t *DeviceInfo, int bootcheck) {
	int ret = 0;
	DiskInfo_t *DiskManager_Item = NULL;

	if (DiskManager_Check_DiskMountStatus(DeviceInfo) == 0)
	{
		unsigned int Index = 0;
		if (DiskManager_GetMountPoint(DeviceInfo) != 0) {
			com_err("DiskManager_GetMountPoint fail\n");
			return -1;
		}
		for (Index = 0; Index < sizeof(FileSystemType)/sizeof(FileSystemType[0]); Index++) {
			if (0) {
				ret = mount(DeviceInfo->DeviceName, DeviceInfo->MountPoint, FileSystemType[Index], 0, NULL);
			} else {
				char command[128] = {0};
				snprintf(command, 128, "mount -t %s %s %s", FileSystemType[Index], DeviceInfo->DeviceName, DeviceInfo->MountPoint);
//				com_err("command:%s\n", command);
				ret = system(command);
			}
			if (ret != 0) {
//				com_err("DeviceName:%s\n", DeviceInfo->DeviceName);
//				com_err("MountPoint:%s\n", DeviceInfo->MountPoint);
//				com_err("FileSystemType:%s\n", FileSystemType[Index]);
//				com_err("mount fail:%s\n", strerror(errno));
				continue;
			}
			LOG_INFO("MountPoint:%s\n", DeviceInfo->MountPoint);
			LOG_INFO("DeviceName:%s\n", DeviceInfo->DeviceName);
			LOG_INFO("FileSystemType:%s\n", FileSystemType[Index]);
			LOG_INFO("mount success!!!\n");
			diskmanager_get_disk_volume(DeviceInfo);
			break;
		}

		if (Index == sizeof(FileSystemType)/sizeof(FileSystemType[0])) {
			LOG_INFO("DeviceName:%s mount fail\n", DeviceInfo->DeviceName);
			DiskManager_Free_MountPoint(DeviceInfo);
			return -1;
		}
		DiskManager_Item = (DiskInfo_t *)malloc(sizeof(DiskInfo_t));
		if (DiskManager_Item == NULL) {
			umount(DeviceInfo->MountPoint);
			DiskManager_Free_MountPoint(DeviceInfo);
			com_err("malloc fail\n");
			return -1;
		}
		if (bootcheck == 0) {
			DeviceInfo->operate = MEDIUM_PLUGIN;
		} else {
			DeviceInfo->operate = MEDIUM_STATIC_PLUGIN;
		}
		memset(DiskManager_Item, 0x00, sizeof(DiskInfo_t));
		memcpy(DiskManager_Item, DeviceInfo, sizeof(DiskInfo_t));
		DiskManager_Add_DiskInfo(DiskManager_Item);
		DiskManager_HotPlug_Message_Info(DiskManager_Item);
	}
	return 0;
}
static int DiskManager_UnMountDevice(DiskInfo_t *DeviceInfo) {
	DiskInfo_t *DiskManager_Item = NULL;

	DiskManager_Item = DiskManager_Find_ItemInfo_by_devicename(DeviceInfo);
	if (DiskManager_Item == NULL) {
		com_err("dev:%s not found\n", DeviceInfo->DeviceName);
		return -1;
	}
	DiskManager_Filter_HotPlug_Message_Info(DiskManager_Item);
	DiskManager_Item->operate = MEDIUM_PLUGOUT;
	com_err("DeviceName:%s\n", DiskManager_Item->DeviceName);
	com_err("MountPoint:%s\n", DiskManager_Item->MountPoint);
	DiskManager_HotPlug_Message_Info(DiskManager_Item);
	return 0;
}

int DiskManager_Register_StaticDisk(DiskInfo_t *DeviceInfo) {
	int ret = 0;
	DiskInfo_t *DiskManager_Item = NULL;

	if (DeviceInfo == NULL) {
		com_err("Paramter Error\n");
		return -1;
	}

	DiskManager_Item = DiskManager_Find_ItemInfo_by_devicename(DeviceInfo);
	if (DiskManager_Item != NULL) {
		com_err("dev:%s exist\n", DeviceInfo->DeviceName);
		return -1;
	}
	ret = DiskManager_Check_StaticDisk(DeviceInfo);
	if (ret == -1) {
		com_err("dev:%s not mount\n", DeviceInfo->DeviceName);
		return -1;
	}
	DiskManager_Item = (DiskInfo_t *)malloc(sizeof(DiskInfo_t));
	memset(DiskManager_Item, 0x00, sizeof(DiskInfo_t));
	memcpy(DiskManager_Item, DeviceInfo, sizeof(DiskInfo_t));
	DiskManager_Add_DiskInfo(DiskManager_Item);
	return 0;
}

unsigned int DiskManager_GetDiskNum(void) {
	return __db_list_get_num(disk_info_list);
}

DiskInfo_t *DiskManager_GetDiskInfoByIndex(unsigned int Index) {
	DiskInfo_t *tmp = NULL;

	tmp = __db_list_search_node_byindex(disk_info_list, Index);
	return tmp;
}
static int diskmanage_compare_registerinfo(void *data, void *compare_param)
{
	hotplug_message_focus_win_t *RegisterInfo = (hotplug_message_focus_win_t *)data;
	hotplug_message_focus_win_t *compare_registerinfo = (hotplug_message_focus_win_t *)compare_param;

	if (strncmp(RegisterInfo->Cur_Win, compare_registerinfo->Cur_Win, strlen(RegisterInfo->Cur_Win)) == 0 &&
		strlen(RegisterInfo->Cur_Win) == strlen(compare_registerinfo->Cur_Win)) {
		return 0;
	}
	return 1;
}
/*hotplug_message_focus_win_t *RegisterInfo must be malloc */
int DiskManager_Register(hotplug_message_focus_win_t *RegisterInfo)
{
	hotplug_message_focus_win_t *tmp = NULL;
	if (NULL == RegisterInfo) {
		com_err("Paramter Error!\n");
		return -1;
	}

	RegisterInfo->status = 1;
	tmp = __db_list_search_and_pop(hotplug_message_focus_app_list, RegisterInfo, diskmanage_compare_registerinfo);
	if (NULL == tmp) {
		__db_list_put_tail(hotplug_message_focus_app_list, RegisterInfo);
	} else {
		com_err("%s already register!\n", RegisterInfo->Cur_Win);
	}
	return 0;
}
int DiskManager_UnRegister(hotplug_message_focus_win_t *RegisterInfo)
{
	if (RegisterInfo != NULL) {
		RegisterInfo->status = 0;
		return 0;
	} else {
		com_err("Paramter Error!\n");
		return -1;
	}
}

int DiskManager_Init(void) {
	disk_info_list = db_list_create("disk_info_list", 0);
	if (NULL == disk_info_list) {
		com_err("db_list_create Error!\n");
		goto done;
	}
	hotplug_message_list = db_list_create("hotplug_message", 1);
	if (NULL == hotplug_message_list) {
		com_err("db_list_create Error!\n");
		goto error;
	}

	hotplug_message_focus_app_list = db_list_create("hotplug_message_focus_app_list", 0);
	if (NULL == hotplug_message_focus_app_list) {
		com_err("db_list_create Error!\n");
		goto error1;
	}
	return 0;
error1:
	__db_list_destory(hotplug_message_list);
	hotplug_message_list = NULL;
error:
	__db_list_destory(disk_info_list);
	disk_info_list = NULL;
done:
	return -1;
}

static ssize_t uevent_kernel_multicast_uid_recv(int socket, void *buffer, size_t length, uid_t * user)
{
	struct iovec iov = { buffer, length };
	struct sockaddr_nl addr;
#ifdef struct_ucred
	char control[CMSG_SPACE(sizeof(struct ucred))] = {0};
#endif
	struct msghdr hdr = {
		&addr,
		sizeof(addr),
		&iov,
		1,
	};
#ifdef struct_ucred
	hdr.msg_control = (void *)control;
	hdr.msg_controllen = sizeof(control);
#endif
	*user = -1;
	ssize_t n = recvmsg(socket, &hdr, 0);
	if (n <= 0) {
		LOG_INFO("there\n");
		return n;
	}
	return n;
}
static ssize_t uevent_kernel_multicast_recv(int socket, void *buffer, size_t length)
{
	uid_t user = -1;
	return uevent_kernel_multicast_uid_recv(socket, buffer, length, &user);
}
static void uevent_event(int arg)
{
	int recv_len = 0, Index = 0, add = 0, remove = 0;
	char msg[UEVENT_MSG_LEN + 2] = {0}, *cp = NULL;
	DiskInfo_t DeviceInfo;

	recv_len = uevent_kernel_multicast_recv(uevent_fd, msg, UEVENT_MSG_LEN);
	if (recv_len <= 0){
		LOG_INFO("there\n");
		return;
	}
	if (recv_len >= UEVENT_MSG_LEN){
		LOG_INFO("there\n");
		return;
	}
	msg[recv_len] = '\0';
	msg[recv_len + 1] = '\0';
	cp = msg;

	/*此处只关心block设备插入情况*/
	{
		memset(&DeviceInfo, 0x00, sizeof(DeviceInfo));
		for (Index = 0; Index < recv_len; Index++) {
			if (!strncmp(cp, "ACTION=", strlen("ACTION="))) {
				if (!strncmp(cp, "ACTION=add", strlen("ACTION=add"))) {
					add = 1;
					remove = 0;
				} else if (!strncmp(cp, "ACTION=remove", strlen("ACTION=remove"))) {
					add = 0;
					remove = 1;
				}
				cp += strlen(cp) + 1;
				continue;
			} else if (!strncmp(cp, "SUBSYSTEM=", strlen("SUBSYSTEM="))) {
				/*DEVTYPE=partition 此时才会挂载成功*/
				if (!strncmp(cp, "SUBSYSTEM=block", strlen("SUBSYSTEM=block"))) {
					cp += strlen(cp) + 1;
					continue;
				} else {
					goto finish;
				}
//			} else if (!strncmp(cp, "DEVTYPE=", strlen("DEVTYPE="))) {
//				/*DEVTYPE=partition 此时才会挂载成功*/
//				if (!strncmp(cp, "DEVTYPE=partition", strlen("DEVTYPE=partition"))) {
//					cp += strlen(cp) + 1;
//					continue;
//				} else {
//					goto finish;
//				}
			} else if (!strncmp(cp, "DEVNAME=", strlen("DEVNAME="))) {
				sprintf(DeviceInfo.DeviceName, "/dev/%s", cp + strlen("DEVNAME="));
				cp += strlen(cp) + 1;
				continue;
			} else if (!strncmp(cp, "MAJOR=", strlen("MAJOR="))) {
				LOG_INFO("DeviceInfo.Major:%s\n", cp);
				DeviceInfo.Major = atoi(cp + strlen("MAJOR="));
				cp += strlen(cp) + 1;
				if (DeviceInfo.Major == USB_DEVICE_MAJOR) {
					DeviceInfo.MediaType = MEDIUM_USB_MASSSTORAGE;
				} else if (DeviceInfo.Major == MMC_DEVICE_MAJOR) {
					DeviceInfo.MediaType = MEDIUM_SD_CARD;
				}
				LOG_INFO("DeviceInfo.Major:%d\n", DeviceInfo.Major);
				continue;
			} else if (!strncmp(cp, "MINOR=", strlen("MINOR="))) {
				//DeviceInfo.Minor = (__u32 )*(cp + strlen("MINOR="));
				LOG_INFO("DeviceInfo.MINOR:%s\n", cp);
				DeviceInfo.Minor = atoi(cp + strlen("MINOR="));
				cp += strlen(cp) + 1;
				LOG_INFO("DeviceInfo.Minor:%d\n", DeviceInfo.Minor);
				continue;
			}
			cp++;
		}
		if (add == 1) {
			/*等待Mount 成功*/
			DiskManager_MountDevice(&DeviceInfo, 0);
		} else if (remove == 1){
			DiskManager_UnMountDevice(&DeviceInfo);
		}
	}
finish:
	add = 0;
	remove = 0;
	return ;
}
static int uevent_open_socket(int buf_sz)
{
	struct sockaddr_nl addr;
	int on = 1;
	int s;

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xffffffff;
	s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (s < 0)
		return -1;

	setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &buf_sz, sizeof(buf_sz));
	setsockopt(s, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));
	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(s);
		return -1;
	}
	return s;
}
static int healthd_register_event(int fd, void (*handler) (int))
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.ptr = (void *)handler;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		com_err("epoll_ctl failed; errno=%d\n", errno);
		return -1;
	}

	eventct++;
	return 0;
}
static void uevent_init(void)
{
	uevent_fd = uevent_open_socket(64 * 1024);
	if (uevent_fd < 0) {
		com_err("uevent_init: uevent_open_socket failed\n");
		return;
	}
	fcntl(uevent_fd, F_SETFL, O_NONBLOCK);
	if (healthd_register_event(uevent_fd, uevent_event)) {
		com_err("register for uevent events failed\n");
		return ;
	}

}
static void *healthd_mainloop(void *arg){

	LOG_INFO("healthd_mainloop start\n");
	//sleep(3);
	DiskManager_Clean_MountPoint();
	DiskManager_BootCheck();
	while (1) {
		struct epoll_event events[eventct];
		int nevents = 0;
		nevents = epoll_wait(epollfd, events, eventct, -1);
		if (nevents == -1) {
			if (errno == EINTR)
				continue;
			com_err("healthd_mainloop: epoll_wait failed\n");
			break;
		}

		for (int n = 0; n < nevents; ++n) {
			if (events[n].data.ptr)
				(*(void (*)(int))events[n].data.ptr) (events[n].events);
		}
	}
	LOG_INFO("healthd_mainloop end\n");
	return NULL;
}

static int scan_register_info_proccess_mesg(void *data, void *param)
{
	hotplug_message_focus_win_t *RegisterInfo = (hotplug_message_focus_win_t *)data;
	DiskInfo_t *hotplug_message = (DiskInfo_t *)param;

	if (RegisterInfo->status) {
		LOG_INFO("Cur_Win:%s\n", RegisterInfo->Cur_Win);
		LOG_INFO("operate:%d\n", hotplug_message->operate);
		LOG_INFO("DeviceName:%s\n", hotplug_message->DeviceName);
		LOG_INFO("MountPoint:%s\n", hotplug_message->MountPoint);
		RegisterInfo->CallBackFunction(hotplug_message);
	}
	// can't use if else; after callback maybe status change
	if (RegisterInfo->status == 0) {
		free(RegisterInfo);
		return 1;
	}
	return 0;
}

static void *hotplug_message_process_loop(void *arg) {
	int ret = 0;
	DiskInfo_t *hotplug_message = NULL;
	while (1) {
		hotplug_message = (DiskInfo_t *)__db_list_pop(hotplug_message_list);
		__db_list_for_each_entry_and_pop(hotplug_message_focus_app_list, hotplug_message, scan_register_info_proccess_mesg);
		if (hotplug_message->operate == MEDIUM_PLUGOUT) {
			ret = umount(hotplug_message->MountPoint);
			if (ret != 0) {
				perror("mount fail:");
				com_err("DeviceName:%s MountPoint:%s unmount fail\n", hotplug_message->DeviceName, hotplug_message->MountPoint);
				umount(hotplug_message->MountPoint);
				perror("mount fail:");
			}
			DiskManager_Free_MountPoint(hotplug_message);
			DiskManager_Del_DiskInfo(hotplug_message);
			free(hotplug_message);
		}
	}
	return NULL;
}

int DiskManager_detect(void) {
	int ret = 0;
	pthread_attr_t attr;
	pthread_t wait_pthread;
	pthread_t message_process_pthread;

	epollfd = epoll_create(MAX_EPOLL_EVENTS);
	if (epollfd == -1) {
		LOG_INFO("epoll_create failed; errno=%d\n", errno);
		return -1;
	}
	uevent_init();
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x4000);
	ret = pthread_create(&wait_pthread, &attr, &healthd_mainloop, NULL);
	if (ret < 0){
		LOG_INFO("pthread_create error\n");
		pthread_attr_destroy(&attr);
		return -1;
	}
	ret = pthread_create(&message_process_pthread, &attr, &hotplug_message_process_loop, NULL);
	if (ret < 0) {
		LOG_INFO("pthread_create error\n");
		pthread_attr_destroy(&attr);
		return -1;
	}
	pthread_attr_destroy(&attr);
	return 0;
}
