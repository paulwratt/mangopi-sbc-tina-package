#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <stdarg.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include "dsp_debug.h"

#define CHOOSE_DSP_WRITE_SPACE		0
#define CHOOSE_ARM_WRITE_SPACE		1
#define CHOOSE_DSP_LOG_SPACE		2

static const char usage_short_opts[] = "d:hr";
static struct option const usage_long_opts[] = {
	{"dev-path",	1,	0,	'd'},
	{"help",	0,	0,	'h'},
	{"read-dsplog",	0,	0,	'r'},
	{0, 0, 0, 0}
};

void usage()
{
	printf("Options:\n");
	printf(" -d, --dev-path\t\t\t path of dsp debug device us\n");
	printf(" -r, --read-dsplog\t\t\t will read dsp save log\n");
	printf(" -h, --help\t\t print this help screen\n");
}

int log_read_addr = 0;

struct dsp_sharespace_t dsp_sharespace;

static int dsp_debug_log_show(char* pstr, int32_t size)
{
	uint32_t len = 0;
	int sum = 0;

	while(1) {
		len = strlen(pstr) + 1;
		if ((len & (~0x3U)) != len){
			len &= (~0x3U);
			len += 4;
		}
		sum += len;
		printf("%s", pstr);

		if (sum == size)
			break;
		pstr += len;
	}

	return 0;
}

static int choose_sharespace(int fd, struct dsp_sharespace_t* msg, uint32_t choose)
{
	int ret = 0;

	/* get debug msg value */
	ret = ioctl(fd, CMD_READ_DEBUG_MSG, (unsigned long)msg);
	if (ret < 0)
		return ret;

	//printf("%x, %x, %x",msg->dsp_write_addr,msg->arm_write_addr,msg->dsp_log_addr);
	switch(choose)
	{
	case CHOOSE_DSP_WRITE_SPACE:
		msg->mmap_phy_addr = msg->dsp_write_addr;
		break;

	case CHOOSE_ARM_WRITE_SPACE:
		msg->mmap_phy_addr = msg->arm_write_addr;
		break;

	case CHOOSE_DSP_LOG_SPACE:
		msg->mmap_phy_addr = msg->dsp_log_addr;
		break;

	}
	/* update debug msg */
	ret = ioctl(fd, CMD_WRITE_DEBUG_MSG, (unsigned long)msg);
	return ret;
}

static int refresh_dsp_write_space(int fd, struct dsp_sharespace_t* msg)
{
	int ret = 0;
	char *mapBuf;

	ret = choose_sharespace(fd, msg, CHOOSE_DSP_WRITE_SPACE);
	if (ret < 0)
		return ret;

	mapBuf = mmap(NULL, msg->dsp_write_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (mapBuf < 0) {
		printf("dev mmap to fail\n");
		ret = -1;
		goto err0;
	}

	memcpy((void *)&msg->value, (void *)mapBuf, sizeof(struct debug_msg_t));
	munmap(mapBuf, msg->dsp_write_size);

	/* update debug msg to driver */
	ret = ioctl(fd, CMD_WRITE_DEBUG_MSG, (unsigned long)msg);
	if (ret < 0)
		printf("dev write to fail\n");

err0:
	return ret;
}

static int refresh_dsp_log_space(int fd, struct dsp_sharespace_t* msg)
{

	int ret = -1, i = 0, tmp = 0;
	uint32_t old_log_head, new_log_head;
	uint32_t log_start_addr, log_end_addr;
	char *mapBuf = NULL;
	char *pstr = NULL;

	ret = choose_sharespace(fd, msg, CHOOSE_DSP_LOG_SPACE);
	if (ret < 0)
		return ret;

	old_log_head = msg->arm_read_dsp_log_addr - msg->dsp_log_addr;
	new_log_head = msg->value.log_head_addr - msg->dsp_log_addr;
	//printf("log_head_addr %x\n", msg->value.log_head_addr);
	//printf("old_log_head, new_log_head = %x , %x\n", old_log_head, new_log_head);

	log_start_addr = 0;
	log_end_addr = log_start_addr + msg->dsp_log_size;
	//printf("log_start_addr, log_end_addr = [%x, %x]\n", log_start_addr, log_end_addr);
	//printf("log_head_addr = %x, arm_read_dsp_log_addr = %x\n", msg->value.log_head_addr, msg->arm_read_dsp_log_addr);

	/* printf dsp log */
	if (old_log_head != new_log_head) {

		/* remap share space */
		mapBuf = mmap(NULL, msg->dsp_log_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		if (mapBuf < 0) {
			printf("dev mmap to fail\n");
			ret = -1;
			goto err0;
		}

		if (new_log_head > old_log_head)
			tmp = new_log_head - old_log_head;
		else
			tmp = msg->dsp_log_size - (old_log_head - new_log_head);

		pstr = (char *)malloc(tmp);
		if (pstr == NULL) {
			printf("dev malloc to fail\n");
			ret = -1;
			goto err1;
		}

		for (i = 0; i < tmp; i++) {
			pstr[i] = mapBuf[old_log_head];
			old_log_head++;
			if (old_log_head >= log_end_addr) {
				old_log_head = log_start_addr;
			}
		}
		dsp_debug_log_show((char*)pstr, tmp);

		/* update debug msg to driver */
		msg->arm_read_dsp_log_addr = msg->value.log_head_addr;
		ret = ioctl(fd, CMD_WRITE_DEBUG_MSG, (unsigned long)msg);
		if (ret < 0)
			printf("dev write to fail\n");

	}

err1:
	if (pstr != NULL)
		free(pstr);

	munmap(mapBuf, msg->dsp_log_size);
err0:
	return ret;
}

int main(int argc, char *argv[])
{
	int fd, i, ret, tmp;
	char *mapBuf;
	int size = 30;
	char *pstr = NULL;
	struct debug_msg_t* pdebug_msg;
	uint32_t old_log_head, new_log_head;
	uint32_t log_start_addr, log_end_addr;
	uint32_t data_size = 0;
	const char *dev_path = NULL;
	int opt;
	int option_index = 0;

	if (argc <= 1) {
		usage();
		return -1;
	}
	while (1) {
		opt = getopt_long (argc, argv, usage_short_opts,
				usage_long_opts, &option_index);
		if (opt == -1)
			break;
		switch (opt) {
		case 'd':
			dev_path = optarg;
			break;
		case 'r':
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	/* open dev */
	fd = open(dev_path, O_RDWR);
	if (fd < 0) {
		printf("open device is error, fd = %d\n",fd);
		return -1;
	}
	refresh_dsp_write_space(fd, &dsp_sharespace);
	refresh_dsp_log_space(fd, &dsp_sharespace);
	//choose_sharespace(fd, (unsigned long)&dsp_sharespace, CHOOSE_DSP_WRITE_SPACE);
	//refresh_log_head_addr(fd);
	//refresh_dsp_write_space(fd, &dsp_sharespace);
	close(fd);
err0:
	return 0;
}
