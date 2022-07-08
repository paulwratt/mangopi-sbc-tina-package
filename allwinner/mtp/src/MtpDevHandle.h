#ifndef _MTPDEVHANDLE_H
#define _MTPDEVHANDLE_H

#include "f_mtp.h"

struct MtpDevHandle;



struct MtpDevHandle *MtpDevHandleInit(int fd);
int MtpDevHandleRelease(struct MtpDevHandle *handle);
int MtpDevHandleSendFile(struct mtp_file_range *mfr, struct MtpDevHandle *handle);
int MtpDevHandleReceiveFile(struct mtp_file_range *mfr, bool zero_packet, struct MtpDevHandle *handle);
int MtpDevHandleSendEvent(struct mtp_event *em, struct MtpDevHandle *handle);

int MtpDevHandleRead(void *data, size_t len, struct MtpDevHandle *handle);
int MtpDevHandleWrite(void *data, size_t len, struct MtpDevHandle *handle);

int writeDescroptors(int fd);
#endif
