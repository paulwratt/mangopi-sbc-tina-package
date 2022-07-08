#ifndef _MTPSERVER_H
#define _MTPSERVER_H

#include "MtpPacket.h"
#include "MtpDataBase.h"
#include "MtpStorage.h"
#include "MtpCommon.h"
#include <stdbool.h>
#include <pthread.h>

struct MtpServer {
	struct MtpDevHandle *mDevHandle;
	pthread_t mTid;

	bool mPtp;
	bool mSessionOpen;
	MtpSessionID mSessionID;

	struct MtpPacket *mRequestPacket;
	struct MtpPacket *mResponsePacket;
	struct MtpPacket *mDataPacket;
	struct MtpPacket *mEventPacket;

	struct MtpDataBase *mDataBase;
	Vector mStorageList;

	Vector mObjectEditList;

	int mFileGroup;
	int mFilePermission;
	int mDirectoryPermission;


	MtpObjectHandle mSendObjectHandle;
	MtpObjectFormat mSendObjectFormat;
	char * mSendObjectFilePath;
	size_t mSendObjectFileSize;

	int (*run)(struct MtpServer *mServer);
	void (*send_object_added)(void);
	void (*send_object_removed)(void);
	void (*send_device_property_changed)(void);
	void (*add_storage)(void);
	void (*remove_storage)(void);
};

struct MtpServer *MtpServerInit(int fileGroup, int filePerm, int directoryPerm, int fd);
void MtpServerRelease(struct MtpServer *mServer);

void MtpServerAddStorage(struct MtpStorage *mStorage, struct MtpServer *mServer);
void MtpServerSendEvent(MtpEventCode code, uint32_t param1, struct MtpServer *mServer);


#endif
