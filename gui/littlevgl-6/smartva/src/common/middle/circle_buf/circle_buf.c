#include "circle_buf.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
static pthread_mutex_t circle_mutex;

int CircleBufCreate(__audio_dev_buf_manager_t *buf_par, unsigned int size)
{
	if (!buf_par || !size) {
		printf("[]parameter is invalid when create circle buffer!\n");
		return -1;
	}

	//clear buffer parameter
	memset(buf_par, 0, sizeof(__audio_dev_buf_manager_t));
	//request buffer circle buffer
	buf_par->pStart = malloc(size);

	if (!buf_par->pStart) {
		printf("Palloc buffer for audio circle buffer failed!\n");
		return -1;
	}

	printf("CircleBufCreate, size=%dk\n", size / 1024);
	buf_par->uTotalSize = size;
	buf_par->uBufSize = size;
	buf_par->uDataSize = 0;
	buf_par->uFreeSize = buf_par->uBufSize;
	buf_par->pRdPtr = buf_par->pStart;
	buf_par->pWrPtr = buf_par->pStart;
	pthread_mutex_init(&circle_mutex, NULL);
	return 0;
}

int CircleBufDestroy(__audio_dev_buf_manager_t *buf_par)
{
	if (!buf_par) {
		printf("Circle buffer parameter is invalid when destroy it!\n");
		return -1;
	}

	if (buf_par->pStart && buf_par->uTotalSize) {
		//esMEMS_Pfree(buf_par->pStart, buf_par->uTotalSize / 1024);
		free(buf_par->pStart);
	}

	memset(buf_par, 0, sizeof(__audio_dev_buf_manager_t));
	return 0;
}

int CircleBufRead(__audio_dev_buf_manager_t *buf_par, unsigned char *buf,
		  unsigned int size)
{
	int result = 0;
	unsigned int tmpSize0, tmpSize1;
	unsigned char *tmpPtr0, *tmpPtr1;

	if (!buf_par || !buf || !size) {
		printf("No data need be read!\n");
		return 0;
	}

	if (!buf_par->uDataSize) {
		printf("No audio data in buffer!\n");
		return 0;
	}

	//calculate current cache data status
	if ((buf_par->pRdPtr + buf_par->uDataSize) >
	    (buf_par->pStart + buf_par->uBufSize)) {
		tmpPtr0 = buf_par->pRdPtr;
		tmpSize0 =
			buf_par->pStart + buf_par->uBufSize - buf_par->pRdPtr;
		tmpPtr1 = buf_par->pStart;
		tmpSize1 = buf_par->uDataSize - tmpSize0;
	} else {
		tmpPtr0 = buf_par->pRdPtr;
		tmpSize0 = buf_par->uDataSize;
		tmpPtr1 = buf_par->pRdPtr + tmpSize0;
		tmpSize1 = 0;
	}

	if ((tmpSize0 + tmpSize1) >= size) {
		//cache data is enough for read
		if (tmpSize0 >= size) {
			memcpy(buf, tmpPtr0, size);
		} else {
			memcpy(buf, tmpPtr0, tmpSize0);
			memcpy(buf + tmpSize0, tmpPtr1, size - tmpSize0);
		}

		//update buffer parameter
		//DISABLE_OS_TASK_SWITCH();;
		pthread_mutex_lock(&circle_mutex);
		buf_par->pRdPtr += size;
		buf_par->uDataSize -= size;
		buf_par->uFreeSize += size;

		if (buf_par->pRdPtr >= buf_par->pStart + buf_par->uBufSize) {
			buf_par->pRdPtr -= buf_par->uBufSize;
		}
		pthread_mutex_unlock(&circle_mutex);
		//ENABLE_OS_TASK_SWITCH();;
		result = size;
	} else {
		//cache data is not enough for read
		memcpy(buf, tmpPtr0, tmpSize0);

		if (tmpSize1) {
			memcpy(buf + tmpSize0, tmpPtr1, tmpSize1);
		}

		//update buffer parameter
		//DISABLE_OS_TASK_SWITCH();;
		pthread_mutex_lock(&circle_mutex);
		buf_par->pRdPtr += tmpSize0 + tmpSize1;
		buf_par->uDataSize -= tmpSize0 + tmpSize1;
		buf_par->uFreeSize += tmpSize0 + tmpSize1;

		if (buf_par->pRdPtr >= buf_par->pStart + buf_par->uBufSize) {
			buf_par->pRdPtr -= buf_par->uBufSize;
		}

		//ENABLE_OS_TASK_SWITCH();;
		pthread_mutex_unlock(&circle_mutex);
		result = tmpSize0 + tmpSize1;
	}

	return result;
}

int CircleBufReadZero(__audio_dev_buf_manager_t *buf_par, unsigned char *buf,
		      unsigned int size)
{
	int result = 0;
	unsigned int tmpSize0, tmpSize1;
	unsigned char *tmpPtr0, *tmpPtr1;

	if (!buf_par || !buf || !size) {
		printf("No data need be read!\n");
		return 0;
	}

	if (!buf_par->uDataSize) {
		printf("No audio data in buffer!\n");
		return 0;
	}

	//calculate current cache data status
	if ((buf_par->pRdPtr + buf_par->uDataSize) >
	    (buf_par->pStart + buf_par->uBufSize)) {
		tmpPtr0 = buf_par->pRdPtr;
		tmpSize0 =
			buf_par->pStart + buf_par->uBufSize - buf_par->pRdPtr;
		tmpPtr1 = buf_par->pStart;
		tmpSize1 = buf_par->uDataSize - tmpSize0;
	} else {
		tmpPtr0 = buf_par->pRdPtr;
		tmpSize0 = buf_par->uDataSize;
		tmpPtr1 = buf_par->pRdPtr + tmpSize0;
		tmpSize1 = 0;
	}

	if ((tmpSize0 + tmpSize1) >= size) {
		//cache data is enough for read
		if (tmpSize0 >= size) {
			memcpy(buf, tmpPtr0, size);
			memset(buf, 0, size);
		} else {
			memcpy(buf, tmpPtr0, tmpSize0);
			memset(buf, 0, tmpSize0);
			memcpy(buf + tmpSize0, tmpPtr1, size - tmpSize0);
			memset(buf + tmpSize0, 0, size - tmpSize0);
		}

		//update buffer parameter
		//DISABLE_OS_TASK_SWITCH();;
		pthread_mutex_lock(&circle_mutex);
		buf_par->pRdPtr += size;
		buf_par->uDataSize -= size;
		buf_par->uFreeSize += size;

		if (buf_par->pRdPtr >= buf_par->pStart + buf_par->uBufSize) {
			buf_par->pRdPtr -= buf_par->uBufSize;
		}

		//ENABLE_OS_TASK_SWITCH();;
		pthread_mutex_unlock(&circle_mutex);
		result = size;
	} else {
		//cache data is not enough for read
		memcpy(buf, tmpPtr0, tmpSize0);
		memset(buf, 0, tmpSize0);

		if (tmpSize1) {
			memcpy(buf + tmpSize0, tmpPtr1, tmpSize1);
			memset(buf + tmpSize0, 0, tmpSize1);
		}

		//update buffer parameter
		//DISABLE_OS_TASK_SWITCH();;
		pthread_mutex_lock(&circle_mutex);
		buf_par->pRdPtr += tmpSize0 + tmpSize1;
		buf_par->uDataSize -= tmpSize0 + tmpSize1;
		buf_par->uFreeSize += tmpSize0 + tmpSize1;

		if (buf_par->pRdPtr >= buf_par->pStart + buf_par->uBufSize) {
			buf_par->pRdPtr -= buf_par->uBufSize;
		}

		//ENABLE_OS_TASK_SWITCH();;
		pthread_mutex_unlock(&circle_mutex);
		result = tmpSize0 + tmpSize1;
	}

	return result;
}

int CircleBufWrite(__audio_dev_buf_manager_t *buf_par, unsigned char *buf,
		   unsigned int size)
{
	int result = 0;
	unsigned int tmpSize0, tmpSize1;
	unsigned char *tmpPtr0, *tmpPtr1;

	if (!buf_par || !buf || !size) {
		printf("No data need be write![%p][%p][%d]\n", buf_par, buf,
		       size);
		return 0;
	}

	if (!buf_par->uFreeSize) {
		printf("No free buffer for store audio data!\n");
		return 0;
	}

	//calculate current free buffer status
	if ((buf_par->pWrPtr + buf_par->uFreeSize) >
	    (buf_par->pStart + buf_par->uBufSize)) {
		//free buffer is two segment
		tmpPtr0 = buf_par->pWrPtr;
		tmpSize0 =
			buf_par->pStart + buf_par->uBufSize - buf_par->pWrPtr;
		tmpPtr1 = buf_par->pStart;
		tmpSize1 = buf_par->uFreeSize - tmpSize0;
	} else {
		tmpPtr0 = buf_par->pWrPtr;
		tmpSize0 = buf_par->uFreeSize;
		tmpPtr1 = buf_par->pWrPtr + tmpSize0;
		tmpSize1 = 0;
	}

	if ((tmpSize0 + tmpSize1) >= size) {
		//free buffer is enough for write
		if (tmpSize0 >= size) {
			memcpy(tmpPtr0, buf, size);
		} else {
			memcpy(tmpPtr0, buf, tmpSize0);
			memcpy(tmpPtr1, buf + tmpSize0, size - tmpSize0);
		}

		//update buffer parameter
		//DISABLE_OS_TASK_SWITCH();;
		pthread_mutex_lock(&circle_mutex);
		buf_par->pWrPtr += size;
		buf_par->uFreeSize -= size;
		buf_par->uDataSize += size;

		if (buf_par->pWrPtr >= buf_par->pStart + buf_par->uBufSize) {
			buf_par->pWrPtr -= buf_par->uBufSize;
		}

		//ENABLE_OS_TASK_SWITCH();;
		pthread_mutex_unlock(&circle_mutex);
		result = size;
	} else {
		//cache data is not enough for read
		memcpy(tmpPtr0, buf, tmpSize0);

		if (tmpSize1) {
			memcpy(tmpPtr1, buf + tmpSize0, tmpSize1);
		}

		//update buffer parameter
		//DISABLE_OS_TASK_SWITCH();;
		pthread_mutex_lock(&circle_mutex);
		buf_par->pWrPtr += tmpSize0 + tmpSize1;
		buf_par->uDataSize += tmpSize0 + tmpSize1;
		buf_par->uFreeSize -= tmpSize0 + tmpSize1;

		if (buf_par->pWrPtr >= buf_par->pStart + buf_par->uBufSize) {
			buf_par->pWrPtr -= buf_par->uBufSize;
		}

		//ENABLE_OS_TASK_SWITCH();;
		pthread_mutex_unlock(&circle_mutex);
		result = tmpSize0 + tmpSize1;
	}

	//printf("current data size:%x", buf_par->uDataSize);
	return result;
}

int CircleBufFlush(__audio_dev_buf_manager_t *buf_par)
{
	if (!buf_par) {
		printf("buffer manager parameter is invalid!\n");
		return -1;
	}

	//DISABLE_OS_TASK_SWITCH();;
	pthread_mutex_lock(&circle_mutex);
	buf_par->pRdPtr = buf_par->pWrPtr;
	buf_par->uDataSize = 0;
	buf_par->uFreeSize = buf_par->uBufSize;
	pthread_mutex_unlock(&circle_mutex);
	//ENABLE_OS_TASK_SWITCH();;
	return 0;
}

int CircleBufQuerySize(__audio_dev_buf_manager_t *buf_par, unsigned int type)
{
	if (!buf_par) {
		printf("buffer manager parameter is invalid!\n");
		return 0;
	}
	//pthread_mutex_lock(&circle_mutex);
	switch (type) {
	case AUDIO_DEV_QUERY_BUF_SIZE_DATA:
		return buf_par->uDataSize;

	case AUDIO_DEV_QUERY_BUF_SIZE_FREE:
		return buf_par->uFreeSize;

	case AUDIO_DEV_QUERY_BUF_SIZE_TOTAL:
		return buf_par->uBufSize;

	default: {
		printf("Unknown type when query buffer size!\n");
		return -1;
	}
	}
	//pthread_mutex_unlock(&circle_mutex);
}

int CircleBufResize(__audio_dev_buf_manager_t *buf_par, int size)
{
	if (!buf_par) {
		printf("Circle buffer parameter is invalid when resize buffer!\n");
		return -1;
	}

	//DISABLE_OS_TASK_SWITCH();;
	pthread_mutex_lock(&circle_mutex);
	printf("CircleBufResize\n");
	CircleBufDestroy(buf_par);
	CircleBufCreate(buf_par, size);
	//ENABLE_OS_TASK_SWITCH();;
	pthread_mutex_unlock(&circle_mutex);
	return 0;
}
