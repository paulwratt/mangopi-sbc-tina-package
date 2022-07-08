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

#include "DiskManager.h"

#define MAX_EPOLL_EVENTS 40
#define UEVENT_MSG_LEN 2048
#define SYS_BLOCK_DEV_PATH "/sys/block"

#define MOUNTPOINT_MAX_LENGTH	(64)
//#define DISKMANAGER_DEBUG
#define MMC_DEVICE_MAJOR	(179)
#define USB_DEVICE_MAJOR	(8)
#ifdef DISKMANAGER_DEBUG
#define LOG_HERE(fmt, ...) do { \
						printf("%s %d %s \n",__FILE__,__LINE__,__func__);\
						}\
						while(0)
#define LOG_INFO(fmt, ...) do { \
						printf("%s %d %s ",__FILE__,__LINE__,__func__);\
						printf(fmt,##__VA_ARGS__);}\
						while(0)
#define LOG_ERR(fmt, ...) do { \
						printf("%s %d %s ",__FILE__,__LINE__,__func__);\
						printf(fmt,##__VA_ARGS__);}\
						while(0)
#else
#define LOG_HERE(fmt, ...) \
			do {;}while(0)

#define LOG_INFO(fmt, ...) \
			do {;} while (0)

#define LOG_ERR(fmt, ...) do { \
						printf("%s %d %s ",__FILE__,__LINE__,__func__);\
						printf(fmt,##__VA_ARGS__);}\
						while(0)
#endif
typedef struct _MountPoint_Info {
	char NeedCreate_Clean;
	char MountPoint[MOUNTPOINT_MAX_LENGTH];
} MountPoint_Info_t;

static int uevent_fd = 0;
static int epollfd = 0;
static int eventct = 0;
static pthread_cond_t HotPlug_Message_wait_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t HotPlug_Message_wait_mutex;
static pthread_mutex_t HotPlug_Message_List_Mutex;
static pthread_mutex_t HotPlug_Message_Focus_List_Mutex;
static pthread_mutex_t Disk_Info_List_Mutex;
static LIST_HEAD(HotPlug_Message_Focus_List_Head);
static LIST_HEAD(HotPlug_Message_List_Head);
static LIST_HEAD(Disk_Info_List_Head);

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
static int DiskManager_Del_MountPoint(DiskInfo_t *DeviceInfo);
static void DiskManager_Del_DiskInfo(DiskInfo_t *DeviceInfo);
static void hotplug_message_up(void);
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
		LOG_ERR("epoll_ctl failed; errno=%d\n", errno);
		return -1;
	}

	eventct++;
	return 0;
}
static void uevent_init(void)
{
	uevent_fd = uevent_open_socket(64 * 1024);

	if (uevent_fd < 0) {
		LOG_ERR("uevent_init: uevent_open_socket failed\n");
		return;
	}
	fcntl(uevent_fd, F_SETFL, O_NONBLOCK);
	if (healthd_register_event(uevent_fd, uevent_event)) {
		LOG_ERR("register for uevent events failed\n");
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
			LOG_ERR("healthd_mainloop: epoll_wait failed\n");
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
static void hotplug_message_down(void)
{
	pthread_mutex_lock(&HotPlug_Message_wait_mutex);
	pthread_cond_wait(&HotPlug_Message_wait_cond, &HotPlug_Message_wait_mutex);
	pthread_mutex_unlock(&HotPlug_Message_wait_mutex);
}
static void hotplug_message_up(void)
{
	pthread_mutex_lock(&HotPlug_Message_wait_mutex);
	pthread_cond_signal(&HotPlug_Message_wait_cond);
	pthread_mutex_unlock(&HotPlug_Message_wait_mutex);
}

static void *hotplug_message_process_loop(void *arg) {
	int ret = 0;
	DiskInfo_t *hotplug_message_tmp = NULL, *hotplug_message = NULL;
	hotplug_message_focus_win_t *registerinfo_tmp = NULL, *registerinfo = NULL;
	while (1) {
		pthread_mutex_lock(&HotPlug_Message_List_Mutex);
		if (!list_empty(&HotPlug_Message_List_Head)) {
			list_for_each_entry_safe(hotplug_message, hotplug_message_tmp, &HotPlug_Message_List_Head, Hotplug_Message_list) {
				pthread_mutex_lock(&HotPlug_Message_Focus_List_Mutex);
				if (!list_empty(&HotPlug_Message_Focus_List_Head)) {
					list_for_each_entry_safe(registerinfo, registerinfo_tmp, &HotPlug_Message_Focus_List_Head, list) {
						if (registerinfo->status) {
							LOG_INFO("Cur_Win:%s\n", registerinfo->Cur_Win);
							LOG_INFO("operate:%d\n", hotplug_message->operate);
							LOG_INFO("DeviceName:%s\n", hotplug_message->DeviceName);
							LOG_INFO("MountPoint:%s\n", hotplug_message->MountPoint);
							registerinfo->CallBackFunction(hotplug_message);
						}
						// can't use if else; after callback maybe status change
						if (registerinfo->status == 0) {
							list_del(&registerinfo->list);
						}
					}
				}
				pthread_mutex_unlock(&HotPlug_Message_Focus_List_Mutex);
				list_del(&hotplug_message->Hotplug_Message_list);
				if (hotplug_message->operate == MEDIUM_PLUGOUT) {
					ret = umount(hotplug_message->MountPoint);
					if (ret != 0) {
						LOG_ERR("DeviceName:%s MountPoint:%s unmount fail\n", hotplug_message->DeviceName, hotplug_message->MountPoint);
						umount(hotplug_message->MountPoint);
					}
					DiskManager_Del_MountPoint(hotplug_message);
					DiskManager_Del_DiskInfo(hotplug_message);
					free(hotplug_message);
				}
			}
			pthread_mutex_unlock(&HotPlug_Message_List_Mutex);
		} else {
			pthread_mutex_unlock(&HotPlug_Message_List_Mutex);
			hotplug_message_down();
		}
	}
	return NULL;
}

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
				LOG_ERR("strchr(buf, '\n') null\n");
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
				LOG_ERR("failed to open %s\n", file_path);
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
        LOG_ERR("failed to open /proc/mounts\n");
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
static void DiskManager_HotPlug_Message_Info(DiskInfo_t *DeviceInfo) {
	pthread_mutex_lock(&HotPlug_Message_List_Mutex);
	list_add_tail(&DeviceInfo->Hotplug_Message_list, &HotPlug_Message_List_Head);
	pthread_mutex_unlock(&HotPlug_Message_List_Mutex);
	hotplug_message_up();
}

static void DiskManager_Add_DiskInfo(DiskInfo_t *DeviceInfo) {
	pthread_mutex_lock(&Disk_Info_List_Mutex);
	list_add_tail(&DeviceInfo->Disk_Info_list, &Disk_Info_List_Head);
	pthread_mutex_unlock(&Disk_Info_List_Mutex);
}

static void DiskManager_Del_DiskInfo(DiskInfo_t *DeviceInfo) {
	pthread_mutex_lock(&Disk_Info_List_Mutex);
	list_del(&DeviceInfo->Disk_Info_list);
	pthread_mutex_unlock(&Disk_Info_List_Mutex);
}
static DiskInfo_t *DiskManager_Find_ItemInfo(DiskInfo_t *DeviceInfo) {
	DiskInfo_t *disk_tmp = NULL;

	pthread_mutex_lock(&Disk_Info_List_Mutex);
	list_for_each_entry(disk_tmp, &Disk_Info_List_Head, Disk_Info_list) {
		LOG_INFO("%s %d dev:%s\n", __FILE__, __LINE__, disk_tmp->DeviceName);
		if (!strncmp(disk_tmp->DeviceName, DeviceInfo->DeviceName, strlen(DeviceInfo->DeviceName)) &&
			strlen(DeviceInfo->DeviceName) == strlen(disk_tmp->DeviceName)) {
			LOG_INFO("dev:%s found\n", DeviceInfo->DeviceName);
			pthread_mutex_unlock(&Disk_Info_List_Mutex);
			return disk_tmp;
		}
	}
	pthread_mutex_unlock(&Disk_Info_List_Mutex);
	return NULL;
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

static int DiskManager_MountPoint_Is_Used(const char *MountPoint) {
	DiskInfo_t *disk_tmp = NULL;
	pthread_mutex_lock(&Disk_Info_List_Mutex);
	list_for_each_entry(disk_tmp, &Disk_Info_List_Head, Disk_Info_list) {
		if (!strncmp(disk_tmp->MountPoint, MountPoint, strlen(MountPoint))) {
			LOG_INFO("disk_tmp->DeviceName:%s\n", disk_tmp->DeviceName);
			LOG_INFO("disk_tmp->MountPoint:%s\n", disk_tmp->MountPoint);
			LOG_INFO("disk_tmp->MediaType:%d\n", disk_tmp->MediaType);
			LOG_INFO("disk_tmp->Major:%d\n", disk_tmp->Major);
			LOG_INFO("disk_tmp->Minor:%d\n", disk_tmp->Minor);
			pthread_mutex_unlock(&Disk_Info_List_Mutex);
			return 1;
		}
	}
	pthread_mutex_unlock(&Disk_Info_List_Mutex);
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
static int DiskManager_Del_MountPoint(DiskInfo_t *DeviceInfo){
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
        LOG_ERR("failed to open /proc/mounts\n");
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
			LOG_ERR("DiskManager_GetMountPoint fail\n");
			return -1;
		}
		for (Index = 0; Index < sizeof(FileSystemType)/sizeof(FileSystemType[0]); Index++) {
			if (0) {
				ret = mount(DeviceInfo->DeviceName, DeviceInfo->MountPoint, FileSystemType[Index], 0, NULL);
			} else {
				char command[128] = {0};
				snprintf(command, 128, "mount -t %s %s %s", FileSystemType[Index], DeviceInfo->DeviceName, DeviceInfo->MountPoint);
//				LOG_ERR("command:%s\n", command);
				ret = system(command);
			}
			if (ret != 0) {
//				LOG_ERR("DeviceName:%s\n", DeviceInfo->DeviceName);
//				LOG_ERR("MountPoint:%s\n", DeviceInfo->MountPoint);
//				LOG_ERR("FileSystemType:%s\n", FileSystemType[Index]);
//				LOG_ERR("mount fail:%s\n", strerror(errno));
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
			DiskManager_Del_MountPoint(DeviceInfo);
			return -1;
		}
		DiskManager_Item = (DiskInfo_t *)malloc(sizeof(DiskInfo_t));
		if (DiskManager_Item == NULL) {
			umount(DeviceInfo->MountPoint);
			DiskManager_Del_MountPoint(DeviceInfo);
			LOG_ERR("malloc fail\n");
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
		LOG_INFO("DeviceName:%s\n", DeviceInfo->DeviceName);
		LOG_INFO("MountPoint:%s\n", DeviceInfo->MountPoint);
		DiskManager_HotPlug_Message_Info(DiskManager_Item);
	}
	return 0;
}

static int DiskManager_UnMountDevice(DiskInfo_t *DeviceInfo) {
	DiskInfo_t *DiskManager_Item = NULL;
	DiskManager_Item = DiskManager_Find_ItemInfo(DeviceInfo);
	if (DiskManager_Item == NULL) {
		LOG_ERR("dev:%s not found\n", DeviceInfo->DeviceName);
		return -1;
	}
	DiskManager_Item->operate = MEDIUM_PLUGOUT;
	LOG_INFO("DeviceName:%s\n", DeviceInfo->DeviceName);
	LOG_INFO("MountPoint:%s\n", DeviceInfo->MountPoint);
	DiskManager_HotPlug_Message_Info(DiskManager_Item);
	return 0;
}

int DiskManager_Register_StaticDisk(DiskInfo_t *DeviceInfo) {
	int ret = 0;
	DiskInfo_t *DiskManager_Item = NULL;

	if (DeviceInfo == NULL) {
		LOG_ERR("Paramter Error\n");
		return -1;
	}

	DiskManager_Item = DiskManager_Find_ItemInfo(DeviceInfo);
	if (DiskManager_Item != NULL) {
		LOG_ERR("dev:%s exist\n", DeviceInfo->DeviceName);
		return -1;
	}
	ret = DiskManager_Check_StaticDisk(DeviceInfo);
	if (ret == -1) {
		LOG_ERR("dev:%s not mount\n", DeviceInfo->DeviceName);
		return -1;
	}
	DiskManager_Item = (DiskInfo_t *)malloc(sizeof(DiskInfo_t));
	memset(DiskManager_Item, 0x00, sizeof(DiskInfo_t));
	memcpy(DiskManager_Item, DeviceInfo, sizeof(DiskInfo_t));
	DiskManager_Add_DiskInfo(DiskManager_Item);
	return 0;
}
int DiskManager_UnRegister_StaticDisk(DiskInfo_t *DeviceInfo) {
	DiskInfo_t *DiskManager_Item = NULL;

	if (DeviceInfo == NULL) {
		LOG_ERR("Paramter Error\n");
		return -1;
	}
	DiskManager_Item = DiskManager_Find_ItemInfo(DeviceInfo);
	if (DiskManager_Item == NULL) {
		LOG_ERR("dev:%s not exist\n", DeviceInfo->DeviceName);
		return -1;
	}
	DiskManager_Del_DiskInfo(DiskManager_Item);
	return 0;
}
unsigned int DiskManager_GetDiskNum(void) {
	unsigned int count = 0;
	DiskInfo_t *tmp = NULL;

	pthread_mutex_lock(&Disk_Info_List_Mutex);
	list_for_each_entry(tmp, &Disk_Info_List_Head, Disk_Info_list) {
		count++;
	}
	pthread_mutex_unlock(&Disk_Info_List_Mutex);
	return count;
}
DiskInfo_t *DiskManager_GetDiskInfoByIndex(unsigned int Index) {
	unsigned int i = 0;
	DiskInfo_t *tmp = NULL;

	pthread_mutex_lock(&Disk_Info_List_Mutex);
	list_for_each_entry(tmp, &Disk_Info_List_Head, Disk_Info_list) {
		if (i == Index) {
			break;
		}
		i++;
	}
	pthread_mutex_unlock(&Disk_Info_List_Mutex);
	if (i < Index) {
		return NULL;
	} else {
		return tmp;
	}
}
int DiskManager_Register(hotplug_message_focus_win_t *RegisterInfo) {

	if (RegisterInfo != NULL) {
		int found = 0;
		hotplug_message_focus_win_t *tmp = NULL;
		pthread_mutex_lock(&HotPlug_Message_Focus_List_Mutex);
		RegisterInfo->status = 1;
		list_for_each_entry(tmp, &HotPlug_Message_Focus_List_Head, list) {
			if (strncmp(tmp->Cur_Win, RegisterInfo->Cur_Win, strlen(RegisterInfo->Cur_Win)) == 0) {
				found = 1;
				break;
			}
		}
		if (found == 0) {
//			printf("%s %d %s Cur_Win:%s\n",__FILE__,__LINE__,__func__, RegisterInfo->Cur_Win);
			list_add_tail(&RegisterInfo->list, &HotPlug_Message_Focus_List_Head);
		}
		pthread_mutex_unlock(&HotPlug_Message_Focus_List_Mutex);
		return 0;
	} else {
		LOG_ERR("Paramter Error!\n");
		return -1;
	}
}
int DiskManager_UnRegister(hotplug_message_focus_win_t *RegisterInfo) {

	if (RegisterInfo != NULL) {
		RegisterInfo->status = 0;
		return 0;
	} else {
		LOG_ERR("Paramter Error!\n");
		return -1;
	}
}

int DiskManager_Init(void) {
	pthread_mutex_init(&HotPlug_Message_List_Mutex, NULL);
	pthread_mutex_init(&HotPlug_Message_Focus_List_Mutex, NULL);
	pthread_mutex_init(&Disk_Info_List_Mutex, NULL);
	pthread_mutex_init(&HotPlug_Message_wait_mutex, NULL);
	pthread_cond_init(&HotPlug_Message_wait_cond, NULL);
	return 0;
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
