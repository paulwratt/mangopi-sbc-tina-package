/*
*********************************************************************************************************
*                                                    ePDK
*                                    the Easy Portable/Player Develop Kits
*                                             audio driver Module
*
*                                    (c) Copyright 2008-2009, kevin China
*                                             All Rights Reserved
*
* File    : circle_buf.h
* By      : kevin
* Version : V1.0
* Date    : 2009-4-2 19:46
*********************************************************************************************************
*/
#ifndef _CIRCLE_BUF_H_
#define _CIRCLE_BUF_H_

#include <stdio.h>

typedef struct __AUDIO_DEV_BUF_MANAGER {
	unsigned char *pStart; //buffer start address
	unsigned int uTotalSize; //buffer total size
	unsigned int
		uBufSize; //buffer use size, should be part or full of total size

	unsigned char *pRdPtr; //read pointer
	unsigned char *pWrPtr; //write pointer

	unsigned int uDataSize; //data size
	unsigned int uFreeSize; //free buffer size

} __audio_dev_buf_manager_t;

typedef enum __AUDIO_DEV_QUERY_BUF_SIZE_TYPE {
	AUDIO_DEV_QUERY_BUF_SIZE_NONE =
		0, /* 未定义要获取空间的类型           */
	AUDIO_DEV_QUERY_BUF_SIZE_DATA, /* 获取缓冲区内的数据size           */
	AUDIO_DEV_QUERY_BUF_SIZE_FREE, /* 获取缓冲区内的空闲空间size       */
	AUDIO_DEV_QUERY_BUF_SIZE_TOTAL, /* 获取缓冲区总空间                 */

	AUDIO_DEV_QUERY_BUF_SIZE_

} __audio_dev_query_buf_size_type_t;

extern int CircleBufCreate(__audio_dev_buf_manager_t *buf_par,
			   unsigned int size);
extern int CircleBufDestroy(__audio_dev_buf_manager_t *buf_par);
extern int CircleBufRead(__audio_dev_buf_manager_t *buf_par, unsigned char *buf,
			 unsigned int size);
extern int CircleBufReadZero(__audio_dev_buf_manager_t *buf_par,
			     unsigned char *buf, unsigned int size);

extern int CircleBufWrite(__audio_dev_buf_manager_t *buf_par,
			  unsigned char *buf, unsigned int size);
extern int CircleBufFlush(__audio_dev_buf_manager_t *buf_par);
extern int CircleBufQuerySize(__audio_dev_buf_manager_t *buf_par,
			      unsigned int type);
extern int CircleBufResize(__audio_dev_buf_manager_t *buf_par, int size);

#endif /* _CIRCLE_BUF_I_H_ */
