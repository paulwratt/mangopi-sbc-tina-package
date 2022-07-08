#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "MtpDevHandle.h"
#include "MtpCommon.h"
#include "mtp.h"

#include <linux/usb/ch9.h>
#include <linux/usb/functionfs.h>

#ifdef USE_AIO
#include "aio/libaio.h"
#include <sys/eventfd.h>
#include <poll.h>

#include <aio.h>
#include <sys/mman.h>
#endif

#define MTP_USB			"/dev/mtp_usb"

#define FFS_MTP_EP0		"/dev/usb-ffs/mtp/ep0"
#define FFS_MTP_EP_IN		"/dev/usb-ffs/mtp/ep1"
#define FFS_MTP_EP_OUT		"/dev/usb-ffs/mtp/ep2"
#define FFS_MTP_EP_INTR		"/dev/usb-ffs/mtp/ep3"

struct MtpDevOps {
	int (*open)(struct MtpDevHandle *);
	int (*read)(void *data, size_t len, struct MtpDevHandle *);
	int (*write)(void *data, size_t len, struct MtpDevHandle *);
	int (*receiveFile)(struct mtp_file_range *, bool, struct MtpDevHandle *);
	int (*sendFile)(struct mtp_file_range *, struct MtpDevHandle *);
	int (*sendEvent)(struct mtp_event *, struct MtpDevHandle *);
	int (*close)(struct MtpDevHandle *);
};

#ifdef USE_AIO
#define NUM_IO_BUFS			2

struct io_buffer {
	struct iocb *iocbs;
	struct iocb **iocb;
	unsigned char *bufs;
	unsigned char **buf;
	unsigned int actual;
};

struct ffs_control {
	struct io_buffer mIobuf[NUM_IO_BUFS];
	io_context_t mCtx;
	int mEventFd;
	struct pollfd mPollFds[2];
	bool mCanceled;
};
#endif

struct MtpDevHandle {
	bool is_ffs;
	struct MtpDevOps ops;
	union {
		struct {
			int mFd;
			int reserved;
		} directFd;
		struct {
			int mControl;
			int mBulkIn;
			int mBulkOut;
			int mIntr;
#ifdef USE_AIO
			struct ffs_control *mFfsCtrl;
#else
			void *data1;
			void *data2;
#endif
		} ffsFd;
	} descFd;
};

#define __uint16_identity(x) x
#define __uint32_identity(x) x
#define __uint64_identity(x) x

#ifndef __bswap_constant_16
#define __bswap_constant_16(x)					\
  ((__uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#endif

#ifndef __bswap_16
#define __bswap_16(x) __bswap_constant_16(x)
#endif

#ifndef __bswap_constant_32
#define __bswap_constant_32(x)					\
  ((__uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#endif

#ifndef __bswap_32
#define __bswap_32(x) __bswap_constant_32(x)
#endif

#ifndef __bswap_constant_64
#define __bswap_constant_64(x)					\
  ((__uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#endif

#ifndef __bswap_64
#define __bswap_64(x) __bswap_constant_64(x)
#endif

#if 0

# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define htobe16(x) __bswap_16 (x)
#  define htole16(x) __uint16_identity (x)
#  define be16toh(x) __bswap_16 (x)
#  define le16toh(x) __uint16_identity (x)

#  define htobe32(x) __bswap_32 (x)
#  define htole32(x) __uint32_identity (x)
#  define be32toh(x) __bswap_32 (x)
#  define le32toh(x) __uint32_identity (x)

#  define htobe64(x) __bswap_64 (x)
#  define htole64(x) __uint64_identity (x)
#  define be64toh(x) __bswap_64 (x)
#  define le64toh(x) __uint64_identity (x)

# else
#  define htobe16(x) __uint16_identity (x)
#  define htole16(x) __bswap_16 (x)
#  define be16toh(x) __uint16_identity (x)
#  define le16toh(x) __bswap_16 (x)

#  define htobe32(x) __uint32_identity (x)
#  define htole32(x) __bswap_32 (x)
#  define be32toh(x) __uint32_identity (x)
#  define le32toh(x) __bswap_32 (x)

#  define htobe64(x) __uint64_identity (x)
#  define htole64(x) __bswap_64 (x)
#  define be64toh(x) __uint64_identity (x)
#  define le64toh(x) __bswap_64 (x)
#endif

#endif

#define cpu_to_le16(x)  htole16(x)
#define cpu_to_le32(x)  htole32(x)

/* Direct interface(/dev/mtp_usb) */
static int MtpDevHandleDirectOpen(struct MtpDevHandle *handle)
{
	int fd;

	DLOG("");
	fd = open("/dev/mtp_usb", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "open /dev/mtp_usb failed\n");
		return -1;
	}
	handle->descFd.directFd.mFd = fd;
	return 0;
}

static int MtpDevHandleDirectRead(void *data, size_t len, struct MtpDevHandle *handle)
{
	int fd;

	fd = handle->descFd.directFd.mFd;
	return read(fd, data, len);
}

static int MtpDevHandleDirectWrite(void *data, size_t len, struct MtpDevHandle *handle)
{
	int fd;

	fd = handle->descFd.directFd.mFd;
	return write(fd, data, len);
}

static int MtpDevHandleDirectReceiveFile(struct mtp_file_range *mfr, bool zero_packet,
					struct MtpDevHandle *handle)
{
	int ret, fd;

	fd = handle->descFd.directFd.mFd;
	ret = ioctl(fd, MTP_RECEIVE_FILE, (unsigned long)mfr);
	return ret;
}

static int MtpDevHandleDirectSendFile(struct mtp_file_range *mfr, struct MtpDevHandle *handle)
{
	int ret, fd;

	fd = handle->descFd.directFd.mFd;
	ret = ioctl(fd, MTP_SEND_FILE_WITH_HEADER, (unsigned long)mfr);
	return ret;
}

static int MtpDevHandleDirectSendEvent(struct mtp_event *em, struct MtpDevHandle *handle)
{
	int ret, fd;

	fd = handle->descFd.directFd.mFd;
	ret = ioctl(fd, MTP_SEND_EVENT, (unsigned long)em);
	return ret;
}

static int MtpDevHandleDirectClose(struct MtpDevHandle *handle)
{
	int ret;

	ret = close(handle->descFd.directFd.mFd);
	handle->descFd.directFd.mFd = -1;
	return ret;
}

/* FFS interface(/dev/usb-ffs/mtp/ep0,1,2,3) */

struct mtp_data_header {
	/* length of packet, including this header */
	__le32 length;
	/* container type (2 for data packet) */
	__le16 type;
	/* MTP command code */
	__le16 command;
	/* MTP transaction ID */
	__le32 transaction_id;
};

#define MAX_PACKET_SIZE_FS	64
#define MAX_PACKET_SIZE_HS	512
#define MAX_PACKET_SIZE_SS	1024
#define MAX_PACKET_SIZE_EV	28

struct func_desc {
    struct usb_interface_descriptor intf;
    struct usb_endpoint_descriptor_no_audio sink;
    struct usb_endpoint_descriptor_no_audio source;
    struct usb_endpoint_descriptor_no_audio intr;
} __attribute__((packed));

struct ss_func_desc {
    struct usb_interface_descriptor intf;
    struct usb_endpoint_descriptor_no_audio sink;
    struct usb_ss_ep_comp_descriptor sink_comp;
    struct usb_endpoint_descriptor_no_audio source;
    struct usb_ss_ep_comp_descriptor source_comp;
    struct usb_endpoint_descriptor_no_audio intr;
    struct usb_ss_ep_comp_descriptor intr_comp;
} __attribute__((packed));

struct desc_v1 {
    struct usb_functionfs_descs_head_v1 {
        __le32 magic;
        __le32 length;
        __le32 fs_count;
        __le32 hs_count;
    } __attribute__((packed)) header;
    struct func_desc fs_descs, hs_descs;
} __attribute__((packed));

struct desc_v2 {
    struct usb_functionfs_descs_head_v2 header;
    // The rest of the structure depends on the flags in the header.
    __le32 fs_count;
    __le32 hs_count;
    __le32 ss_count;
    __le32 os_count;
    struct func_desc fs_descs, hs_descs;
    struct ss_func_desc ss_descs;
    struct usb_os_desc_header os_header;
    struct usb_ext_compat_desc os_desc;
} __attribute__((packed));

#define STR_INTERFACE "MTP"
struct functionfs_lang {
    __le16 code;
    char str1[sizeof(STR_INTERFACE)];
} __attribute__((packed));

struct functionfs_strings {
    struct usb_functionfs_strings_head header;
    struct functionfs_lang lang0;
} __attribute__((packed));

const struct desc_v2 mtp_desc_v2 = {
	.header = {
		.magic = cpu_to_le32(FUNCTIONFS_DESCRIPTORS_MAGIC_V2),
		.length = cpu_to_le32(sizeof(struct desc_v2)),
		.flags = FUNCTIONFS_HAS_FS_DESC | FUNCTIONFS_HAS_HS_DESC |
			FUNCTIONFS_HAS_MS_OS_DESC,
	},
	.fs_count = 4,
	.hs_count = 4,
	.os_count = 1,
	.fs_descs = {
		.intf = {
			.bLength = USB_DT_INTERFACE_SIZE,
			.bDescriptorType = USB_DT_INTERFACE,
			.bInterfaceNumber = 0,
			.bNumEndpoints = 3,
			.bInterfaceClass = USB_CLASS_STILL_IMAGE,
			.bInterfaceSubClass = 1,
			.bInterfaceProtocol = 1,
			.iInterface = 1,
		},
		.sink = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 1 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = MAX_PACKET_SIZE_FS,
		},
		.source = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 2 | USB_DIR_OUT,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = MAX_PACKET_SIZE_HS,
		},
		.intr = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 3 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_INT,
			.wMaxPacketSize = MAX_PACKET_SIZE_EV,
			.bInterval = 6,
		},
	},
	.hs_descs = {
		.intf = {
			.bLength = USB_DT_INTERFACE_SIZE,
			.bDescriptorType = USB_DT_INTERFACE,
			.bInterfaceNumber = 0,
			.bNumEndpoints = 3,
			.bInterfaceClass = USB_CLASS_STILL_IMAGE,
			.bInterfaceSubClass = 1,
			.bInterfaceProtocol = 1,
			.iInterface = 1,
		},
		.sink = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 1 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = MAX_PACKET_SIZE_HS,
		},
		.source = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 2 | USB_DIR_OUT,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = MAX_PACKET_SIZE_HS,
		},
		.intr = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 3 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_INT,
			.wMaxPacketSize = MAX_PACKET_SIZE_EV,
			.bInterval = 6,
		},
	},
	.os_header = {
		.interface = cpu_to_le32(1),
		.dwLength = cpu_to_le32(sizeof(struct usb_os_desc_header) + sizeof(struct usb_ext_compat_desc)),
		.bcdVersion = cpu_to_le16(1),
		.wIndex = cpu_to_le16(4),
		.bCount = cpu_to_le16(1),
		.Reserved = cpu_to_le16(0),
	},
	.os_desc = {
		.bFirstInterfaceNumber = 0,
		.Reserved1 = cpu_to_le32(1),
		.CompatibleID = { 'M', 'T', 'P' },
		.SubCompatibleID = {0},
		.Reserved2 = {0},
	},
};

const struct desc_v1 mtp_desc_v1 = {
	.header = {
		.magic = cpu_to_le32(FUNCTIONFS_DESCRIPTORS_MAGIC),
		.length = cpu_to_le32(sizeof(struct desc_v1)),
		.fs_count = 4,
		.hs_count = 4,
	},
	.fs_descs = {
		.intf = {
			.bLength = USB_DT_INTERFACE_SIZE,
			.bDescriptorType = USB_DT_INTERFACE,
			.bInterfaceNumber = 0,
			.bNumEndpoints = 3,
			.bInterfaceClass = USB_CLASS_STILL_IMAGE,
			.bInterfaceSubClass = 1,
			.bInterfaceProtocol = 1,
			.iInterface = 1,
		},
		.sink = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 1 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = MAX_PACKET_SIZE_FS,
		},
		.source = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 2 | USB_DIR_OUT,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = MAX_PACKET_SIZE_HS,
		},
		.intr = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 3 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_INT,
			.wMaxPacketSize = MAX_PACKET_SIZE_EV,
			.bInterval = 6,
		},
	},
	.hs_descs = {
		.intf = {
			.bLength = USB_DT_INTERFACE_SIZE,
			.bDescriptorType = USB_DT_INTERFACE,
			.bInterfaceNumber = 0,
			.bNumEndpoints = 3,
			.bInterfaceClass = USB_CLASS_STILL_IMAGE,
			.bInterfaceSubClass = 1,
			.bInterfaceProtocol = 1,
			.iInterface = 1,
		},
		.sink = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 1 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = MAX_PACKET_SIZE_HS,
		},
		.source = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 2 | USB_DIR_OUT,
			.bmAttributes = USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize = MAX_PACKET_SIZE_HS,
		},
		.intr = {
			.bLength = USB_DT_ENDPOINT_SIZE,
			.bDescriptorType = USB_DT_ENDPOINT,
			.bEndpointAddress = 3 | USB_DIR_IN,
			.bmAttributes = USB_ENDPOINT_XFER_INT,
			.wMaxPacketSize = MAX_PACKET_SIZE_EV,
			.bInterval = 6,
		},
	},
};

const struct functionfs_strings mtp_strings = {
    .header = {
        .magic = cpu_to_le32(FUNCTIONFS_STRINGS_MAGIC),
        .length = cpu_to_le32(sizeof(struct functionfs_strings)),
        .str_count = cpu_to_le32(1),
        .lang_count = cpu_to_le32(1),
    },
    .lang0 = {
        .code = cpu_to_le16(0x0409),
        .str1 = STR_INTERFACE,
    },
};

#if 0
static void set_udc(void)
{
	static char g_udc_controller[64];
	const char *udc_config_path = "/sys/kernel/config/usb_gadget/g1/UDC";
	int fd;

	DLOG("\n");
	if (strlen(g_udc_controller) <= 0) {
		const char *udc_path = "/sys/class/udc";
		DIR *dir;
		struct dirent *ent;
		dir = opendir(udc_path);
		if (!dir) {
			DLOG("[%s opendir %s failed]\n", __func__, udc_path);
			return;
		}
		while((ent = readdir(dir)) != NULL) {
			if (ent->d_type & DT_REG) {
				if (!strcmp(".", ent->d_name))
					continue;
				else if (!strcmp("..", ent->d_name))
					continue;
				else
					break;
			}
		}
		if (!ent) {
			DLOG("cannot found udc dir, udc driver maybe not select\n");
			exit(-1);
		}
		DLOG("%s get udc controller: %s \n", __func__, ent->d_name);
		strncpy(g_udc_controller, ent->d_name, sizeof(g_udc_controller));
		closedir(dir);
	}

	DLOG("\n");
	fd = open(udc_config_path, O_RDWR);
	if (fd<0) {
		DLOG("[%s open %s failed]\n", __func__, udc_config_path);
		return;
	}
	write(fd, g_udc_controller, strlen(g_udc_controller));

	DLOG("\n");
	close(fd);
	return;
}
#endif

int writeDescroptors(int fd)
{
	int ret;

	ret = write(fd, &mtp_desc_v2, sizeof(struct desc_v2));
	if (ret < 0) {
		fprintf(stdout, "Switching to V1 descriptor format\n");
		ret = write(fd, &mtp_desc_v1, sizeof(struct desc_v1));
		if (ret < 0) {
			fprintf(stderr, "write descriptors failed\n");
			return -1;
		}
	}

	ret = write(fd, &mtp_strings, sizeof(struct functionfs_strings));
	if (ret < 0) {
		fprintf(stderr, "write strings failed\n");
		return -1;
	}

	/*set_udc();*/
	return 0;
}

#define MAX_MTP_FILE_SIZE	(0xFFFFFFFF)


#ifdef USE_AIO
/*#define AIO_BUFS_MAX			128*/
#define AIO_BUFS_MAX			64
#define AIO_BUF_LEN			16384
#define MAX_FILE_CHUNK_SIZE		(AIO_BUFS_MAX * AIO_BUF_LEN)

#define POLL_TIMEOUT_MS			500

struct mtp_device_status {
	uint16_t wLength;
	uint16_t wCode;
};

static int ffs_control_init(struct ffs_control *ctrl, int mControl)
{
	struct io_buffer *iobuf = ctrl->mIobuf;
	unsigned int i, j;

	for (i = 0; i < NUM_IO_BUFS; i++) {
		iobuf[i].bufs = malloc(MAX_FILE_CHUNK_SIZE);
		iobuf[i].iocbs = malloc(AIO_BUFS_MAX * sizeof(struct iocb));
		iobuf[i].iocb = malloc(AIO_BUFS_MAX * sizeof(struct iocb *));
		iobuf[i].buf = malloc(AIO_BUFS_MAX * sizeof(unsigned char *));
		for (j = 0; j < AIO_BUFS_MAX; j++) {
			iobuf[i].buf[j] = iobuf[i].bufs + j * AIO_BUF_LEN;
			iobuf[i].iocb[j] = &iobuf[i].iocbs[j];
		}
	}

	memset(&ctrl->mCtx, 0, sizeof(ctrl->mCtx));
	if (io_setup(AIO_BUFS_MAX, &ctrl->mCtx) < 0) {
		fprintf(stderr, "unable to setup aio, errno=%d\n", errno);
		goto failed;
	}

	ctrl->mEventFd = eventfd(0, EFD_NONBLOCK);
	ctrl->mPollFds[0].fd = mControl;
	ctrl->mPollFds[0].events = POLLIN;
	ctrl->mPollFds[1].fd = ctrl->mEventFd;
	ctrl->mPollFds[1].events = POLLIN;

	return 0;
failed:
	for (i = 0; i < NUM_IO_BUFS; i++) {
		if (iobuf[i].bufs)
			free(iobuf[i].bufs);
		if (iobuf[i].iocbs)
			free(iobuf[i].iocbs);
		if (iobuf[i].iocb)
			free(iobuf[i].iocb);
		if (iobuf[i].buf)
			free(iobuf[i].buf);
		memset(&iobuf[i], 0, sizeof(struct io_buffer));
	}

	return -1;
}

static void ffs_control_deinit(struct ffs_control *ctrl)
{
	struct io_buffer *iobuf = ctrl->mIobuf;
	unsigned int i;

	io_destroy(ctrl->mCtx);
	for (i = 0; i < NUM_IO_BUFS; i++) {
		if (iobuf[i].bufs)
			free(iobuf[i].bufs);
		if (iobuf[i].iocbs)
			free(iobuf[i].iocbs);
		if (iobuf[i].iocb)
			free(iobuf[i].iocb);
		if (iobuf[i].buf)
			free(iobuf[i].buf);
	}
	free(ctrl);
	return;
}
#else
#define MAX_FILE_CHUNK_SIZE	(16*1024)
#endif


static int MtpDevHandleFfsOpen(struct MtpDevHandle *handle)
{
	int fd;
	void *data = NULL;

	DLOG("");
#if 0
	fd = open(FFS_MTP_EP0, O_RDWR);
	if (fd < 0)
		goto err_open_ep;
	handle->descFd.ffsFd.mControl = fd;

	/* write descriptors */
	writeDescroptors(handle->descFd.ffsFd.mControl);
#else
	if (handle->descFd.ffsFd.mControl < 0) {
		fprintf(stderr, "ffs control fd is invalid\n");
		return -1;
	}
#endif

	fd = open(FFS_MTP_EP_IN, O_RDWR);
	if (fd < 0)
		goto err_open_ep;
	handle->descFd.ffsFd.mBulkIn = fd;
	fd = open(FFS_MTP_EP_OUT, O_RDWR);
	if (fd < 0)
		goto err_open_ep;
	handle->descFd.ffsFd.mBulkOut =  fd;
	fd = open(FFS_MTP_EP_INTR, O_RDWR);
	if (fd < 0)
		goto err_open_ep;
	handle->descFd.ffsFd.mIntr = fd;

	DLOG("");

#if 1
	DLOG("clear all msg");
	struct usb_functionfs_event events[1];
	int bytes;
	bytes = read(handle->descFd.ffsFd.mControl, events, sizeof(events));
	DLOG("ERROR bytes=%d", bytes);
#endif

#ifdef USE_AIO
	handle->descFd.ffsFd.mFfsCtrl = calloc(1, sizeof(struct ffs_control));

	if (ffs_control_init(handle->descFd.ffsFd.mFfsCtrl, handle->descFd.ffsFd.mControl) < 0)
		goto err_ffs_control_init;
#else
	DLOG("");
	data = malloc(MAX_FILE_CHUNK_SIZE);
	if (!data) {
		fprintf(stderr, "%s no memory\n", __func__);
		goto err_no_memory;
	}
	handle->descFd.ffsFd.data1 = data;

	data = malloc(MAX_FILE_CHUNK_SIZE);
	if (!data) {
		fprintf(stderr, "%s no memory\n", __func__);
		goto err_no_memory;
	}
	handle->descFd.ffsFd.data2 = data;
#endif
	DLOG("");
	return 0;

#ifdef USE_AIO
err_ffs_control_init:
#else
err_no_memory:
	if (handle->descFd.ffsFd.data1) {
		free(handle->descFd.ffsFd.data1);
		handle->descFd.ffsFd.data1 = NULL;
	}
	if (handle->descFd.ffsFd.data2) {
		free(handle->descFd.ffsFd.data2);
		handle->descFd.ffsFd.data2 = NULL;
	}
#endif
err_open_ep:
	DLOG("");
	fprintf(stderr, "open ep failed\n");
	if (handle->descFd.ffsFd.mControl > 0)
		close(handle->descFd.ffsFd.mControl);
	if (handle->descFd.ffsFd.mBulkIn > 0)
		close(handle->descFd.ffsFd.mBulkIn);
	if (handle->descFd.ffsFd.mBulkOut > 0)
		close(handle->descFd.ffsFd.mBulkOut);
	if (handle->descFd.ffsFd.mIntr > 0)
		close(handle->descFd.ffsFd.mIntr);
	handle->descFd.ffsFd.mControl = -1;
	handle->descFd.ffsFd.mBulkIn = -1;
	handle->descFd.ffsFd.mBulkOut = -1;
	handle->descFd.ffsFd.mIntr = -1;
	return -1;
}

static int MtpDevHandleFfsRead(void *data, size_t len, struct MtpDevHandle *handle)
{
	int fd;

	fd = handle->descFd.ffsFd.mBulkOut;
	return read(fd, data, len);
}

static int MtpDevHandleFfsWrite(void *data, size_t len, struct MtpDevHandle *handle)
{
	int fd;

	fd = handle->descFd.ffsFd.mBulkIn;
	return write(fd, data, len);

}
#if 0
static inline uint32_t min(uint32_t a, uint32_t b)
{
	if (a < b)
		return a;
	return b;
}
#else
#define min(a,b)	((a) < (b))  ? (a) : (b);
#endif

#ifdef USE_AIO
int handleControlRequest(const struct usb_ctrlrequest *setup, struct MtpDevHandle *handle)
{
	uint8_t type = setup->bRequestType;
	uint8_t code = setup->bRequest;
	uint16_t length = setup->wLength;
	uint16_t index = setup->wIndex;
	uint16_t value = setup->wValue;
	char *buf;
	int mControl = handle->descFd.ffsFd.mControl;

	DLOG("");
	buf = alloca(length);
	if (!(type & USB_DIR_IN)) {
		if (read(mControl, buf, length) != length) {
			fprintf(stderr, "Mtp error ctrlreq read data\n");
			return -1;
		}
	}

	if ((type & USB_TYPE_MASK) == USB_TYPE_CLASS && index == 0 && value == 0) {
		switch(code) {
		case MTP_REQ_RESET:
		case MTP_REQ_CANCEL:
			errno = ECANCELED;
			return -1;
			//    break;
		case MTP_REQ_GET_DEVICE_STATUS: {
			if (length < sizeof(struct mtp_device_status) + 4) {
				errno = EINVAL;
				return -1;
			}
			struct mtp_device_status *st = (struct mtp_device_status*)(buf);
			st->wLength = htole16(sizeof(st));
			DLOG("");
			if (handle->descFd.ffsFd.mFfsCtrl->mCanceled) {
				st->wLength += 4;
				st->wCode = MTP_RESPONSE_TRANSACTION_CANCELLED;
				uint16_t *endpoints = (uint16_t*)(st + 1);
				endpoints[0] = ioctl(handle->descFd.ffsFd.mBulkIn, FUNCTIONFS_ENDPOINT_REVMAP);
				endpoints[1] = ioctl(handle->descFd.ffsFd.mBulkOut, FUNCTIONFS_ENDPOINT_REVMAP);
				handle->descFd.ffsFd.mFfsCtrl->mCanceled = false;
			} else {
				st->wCode = MTP_RESPONSE_OK;
			}
			length = st->wLength;
			break;
		}
		default:
			fprintf(stderr, "Unrecognized Mtp class request! code:%d\n", code);
		}
	} else {
		fprintf(stderr, "Unrecognized request type: %d\n", type);
	}

	if (type & USB_DIR_IN) {
		if (write(mControl, buf, length) != length) {
			fprintf(stderr, "Mtp error ctrlreq write data\n");
		}
	}
	return 0;
}

#if 0
#define HEXDUMP(ptr, size) \
do { \
	int i; \
	char *p = (char *)ptr; \
	for (i = 0; i < size; i++) { \
		printf("0x%x ", *(p+i)); \
		if ((i+1)%16 == 0) \
			printf("\n"); \
	} \
	printf("\n"); \
} while (0)
#else
#define HEXDUMP(ptr, size)
#endif

#define FFS_NUM_EVENTS		5
static int handleEvent(struct MtpDevHandle *handle)
{
	int mControl = handle->descFd.ffsFd.mControl;
	struct usb_functionfs_event events[FFS_NUM_EVENTS];
	struct usb_functionfs_event *event = events;
	int nbytes;
	int ret = 0;
	size_t n;

	DLOG("");
	nbytes = read(mControl, event, FFS_NUM_EVENTS * sizeof(struct usb_functionfs_event));
	if (nbytes == -1)
		return -1;
	HEXDUMP(event, nbytes);
#if 0
	if (nbytes == 12 && event->type == 4) {
		nbytes = read(mControl, event, FFS_NUM_EVENTS * sizeof(struct usb_functionfs_event));
		DLOG("read again, nbytes=%d", nbytes);
		HEXDUMP(event, nbytes);
	}
#endif
#if 0
struct usb_functionfs_event {
	union {
		/* SETUP: packet; DATA phase i/o precedes next event
		 *(setup.bmRequestType & USB_DIR_IN) flags direction */
		struct usb_ctrlrequest	setup;
	} __attribute__((packed)) u;

	/* enum usb_functionfs_event_type */
	__u8				type;
	__u8				_pad[3];
} __attribute__((packed));
struct usb_ctrlrequest {
	__u8 bRequestType;
	__u8 bRequest;
	__le16 wValue;
	__le16 wIndex;
	__le16 wLength;
} __attribute__ ((packed));
#endif

	DLOG("nbytes=%d, size usb_functionfs_event =%d", nbytes, sizeof(struct usb_functionfs_event));
	HEXDUMP(event, nbytes);
	for (n = nbytes / sizeof(*event); n; --n, ++event) {
		DLOG("n=%d, event type=0x%x", n, event->type);
		switch (event->type) {
		case FUNCTIONFS_BIND:
		case FUNCTIONFS_ENABLE:
			ret = 0;
			errno = 0;
			break;
		case FUNCTIONFS_UNBIND:
		case FUNCTIONFS_DISABLE:
			errno = ESHUTDOWN;
			ret = -1;
			break;
		case FUNCTIONFS_SETUP:
			if (handleControlRequest(&event->u.setup, handle) == -1)
				ret = -1;
			break;
		case FUNCTIONFS_SUSPEND:
		case FUNCTIONFS_RESUME:
			break;
		default:
			fprintf(stderr, "Mtp Event:%d (unknown)\n", event->type);
			return -1;
		}
	}
	return ret;
}

static int waitEvents(struct io_buffer *buf, int min_events,
			struct io_event *events, int *counter,
			struct MtpDevHandle *handle)
{
	struct ffs_control *ctrl = handle->descFd.ffsFd.mFfsCtrl;
	struct pollfd *mPollFds = ctrl->mPollFds;
	int mEventFd = ctrl->mEventFd;
	int num_events = 0;
	int ret = 0;
	int error = 0;
	struct timespec ZERO_TIMEOUT = { 0, 0 };

	while (num_events < min_events) {
		if (poll(mPollFds, 2, POLL_TIMEOUT_MS) == -1) {
			fprintf(stderr, "Mtp error during poll()\n");
			return -1;
		}
		if (mPollFds[0].revents & POLLIN) {
			mPollFds[0].revents = 0;
			DLOG("ERROR handle ep0");
			if (handleEvent(handle) == -1) {
				error = errno;
			}
		}
		if (mPollFds[1].revents & POLLIN) {
			/*DLOG("ERROR handle event");	*/
			mPollFds[1].revents = 0;
			uint64_t ev_cnt = 0;

			if (read(mEventFd, &ev_cnt, sizeof(ev_cnt)) == -1) {
				fprintf(stderr, "Mtp unable to read eventfd");
				error = errno;
				continue;
			}

			// It's possible that io_getevents will return more events than the eventFd reported,
			// since events may appear in the time between the calls. In this case, the eventFd will
			// show up as readable next iteration, but there will be fewer or no events to actually
			// wait for. Thus we never want io_getevents to block.
			int this_events = io_getevents(ctrl->mCtx, 0, AIO_BUFS_MAX, events, &ZERO_TIMEOUT);
			if (this_events == -1) {
				fprintf(stderr, "Mtp error getting events");
				error = errno;
			}
			// Add up the total amount of data and find errors on the way.
			for (unsigned j = 0; j < (unsigned int)(this_events); j++) {
				if (events[j].res < 0) {
					errno = -events[j].res;
					fprintf(stderr, "Mtp got error event at %d\n", j);
					error = errno;
				}
				ret += events[j].res;
			}
			num_events += this_events;
			if (counter)
				*counter += this_events;
		}
		if (error) {
			errno = error;
			ret = -1;
			break;
		}
	}
	return ret;
}

static void cancelTransaction(struct MtpDevHandle *handle)
{
	struct ffs_control *ctrl = handle->descFd.ffsFd.mFfsCtrl;

	// Device cancels by stalling both bulk endpoints.
	if (read(handle->descFd.ffsFd.mBulkIn, NULL, 0) != -1 || errno != EBADMSG)
		fprintf(stderr, "Mtp stall failed on bulk in\n");
	if (write(handle->descFd.ffsFd.mBulkOut, NULL, 0) != -1 || errno != EBADMSG)
		fprintf(stderr, "Mtp stall failed on bulk out\n");
	ctrl->mCanceled = true;
	errno = ECANCELED;
}


static int cancelEvents(struct iocb **iocb, struct io_event *events, unsigned start,
        unsigned end, struct MtpDevHandle *handle)
{
	// Some manpages for io_cancel are out of date and incorrect.
	// io_cancel will return -EINPROGRESS on success and does
	// not place the event in the given memory. We have to use
	// io_getevents to wait for all the events we cancelled.
	int ret = 0;
	unsigned num_events = 0;
	int save_errno = 0;
	struct ffs_control *ctrl = handle->descFd.ffsFd.mFfsCtrl;

	save_errno = errno;
	errno = 0;

	DLOG("save_errno=%d", save_errno);
	for (unsigned j = start; j < end; j++) {
#if 1
		if (io_cancel(ctrl->mCtx, iocb[j], NULL) != -1 || errno != EINPROGRESS) {
			fprintf(stderr, "Mtp couldn't cancel request:%d\n", j);
		} else {
			num_events++;
		}
#else
		while (1) {
			ret = io_cancel(ctrl->mCtx, iocb[j], NULL);
			DLOG("ret=%d, errno=%d", ret, errno);
			if (ret != EINPROGRESS)
				break;
			usleep(50000);
		}
		DLOG("ret=%d, errno=%d", ret, errno);
		if (ret != EINPROGRESS) {
		/*if (ret != -1 || errno != EINPROGRESS) {*/
			fprintf(stderr, "Mtp couldn't cancel request:%d\n", j);
		} else {
			num_events++;
		}
#endif
	}
	DLOG("ret=%d", ret);
	if (num_events != end - start) {
		ret = -1;
		errno = EIO;
	}
	DLOG("ret=%d", ret);
	int evs = io_getevents(ctrl->mCtx, num_events, AIO_BUFS_MAX, events, NULL);
	if ((unsigned)(evs) != num_events) {
		fprintf(stderr, "Mtp couldn't cancel all requests, got :%u\n", evs);
		ret = -1;
	}

	uint64_t ev_cnt = 0;
	if (num_events && read(ctrl->mEventFd, &ev_cnt, sizeof(ev_cnt)) == -1)
		fprintf(stderr, "Mtp Unable to read event fd");

	DLOG("ret=%d", ret);
	if (ret == 0) {
		// Restore errno since it probably got overriden with EINPROGRESS.
		errno = save_errno;
	}

	return ret;
}

static int iobufSubmit(struct io_buffer *buf, int fd, unsigned length, bool read, struct MtpDevHandle *handle)
{
	int ret = 0;

	buf->actual = AIO_BUFS_MAX;
	for (unsigned j = 0; j < AIO_BUFS_MAX; j++) {
		unsigned rq_length = min(AIO_BUF_LEN, length - AIO_BUF_LEN * j);
#if 0
		io_prep(buf->iocb[j], fd, buf->buf[j], rq_length, 0, read);
#else
		if (read)
			io_prep_pread(buf->iocb[j], fd, buf->buf[j], rq_length, 0);
		else
			io_prep_pwrite(buf->iocb[j], fd, buf->buf[j], rq_length, 0);
#endif
		io_set_eventfd(buf->iocb[j], handle->descFd.ffsFd.mFfsCtrl->mEventFd);

		// Not enough data, so table is truncated.
		if (rq_length < AIO_BUF_LEN || length == AIO_BUF_LEN * (j + 1)) {
			buf->actual = j + 1;
			break;
		}
	}

	ret = io_submit(handle->descFd.ffsFd.mFfsCtrl->mCtx, buf->actual, buf->iocb);
	if (ret != (int)(buf->actual)) {
		fprintf(stderr, "Mtp io_submit got :%d, expected :%d\n", ret, buf->actual);
		if (ret != -1) {
			errno = EIO;
		}
		ret = -1;
	}
	return ret;
}

static int getPacketSize(int ffs_fd) {
    struct usb_endpoint_descriptor desc;
    if (ioctl(ffs_fd, FUNCTIONFS_ENDPOINT_DESC, (unsigned long)(&desc))) {
        fprintf(stderr, "Could not get FFS bulk-in descriptor\n");
        return MAX_PACKET_SIZE_HS;
    } else {
        return desc.wMaxPacketSize;
    }
}

static int doAsync(void* data, size_t len, bool read, bool zero_packet, struct MtpDevHandle *handle)
{
	struct io_event ioevs[AIO_BUFS_MAX];
	size_t total = 0;
	struct ffs_control *ctrl = handle->descFd.ffsFd.mFfsCtrl;
	int mBulkOut = handle->descFd.ffsFd.mBulkOut;
	int mBulkIn = handle->descFd.ffsFd.mBulkIn;

	while (total < len) {
		size_t this_len = min(len - total, (size_t)(AIO_BUF_LEN * AIO_BUFS_MAX));
		int num_bufs = this_len / AIO_BUF_LEN + (this_len % AIO_BUF_LEN == 0 ? 0 : 1);
		for (int i = 0; i < num_bufs; i++) {
			ctrl->mIobuf[0].buf[i] = (unsigned char*)(data) + total + i * AIO_BUF_LEN;
		}
		int ret = iobufSubmit(&ctrl->mIobuf[0], read ? mBulkOut : mBulkIn, this_len, read, handle);
		if (ret < 0)
			return -1;
		ret = waitEvents(&ctrl->mIobuf[0], ret, ioevs, NULL, handle);
		if (ret < 0)
			return -1;
		total += ret;
		if ((size_t)(ret) < this_len)
			break;
	}

	int packet_size = getPacketSize(read ? mBulkOut : mBulkIn);
	if (len % packet_size == 0 && zero_packet) {
		int ret = iobufSubmit(&ctrl->mIobuf[0], read ? mBulkOut : mBulkIn, 0, read, handle);
		if (ret < 0)
			return -1;
		ret = waitEvents(&ctrl->mIobuf[0], ret, ioevs, NULL, handle);
		if (ret < 0)
			return -1;
	}

	for (unsigned i = 0; i < AIO_BUFS_MAX; i++) {
		ctrl->mIobuf[0].buf[i] = ctrl->mIobuf[0].bufs + i * AIO_BUF_LEN;
	}
	return total;
}

static int async_read(void *data, size_t len, struct MtpDevHandle *handle)
{
    // Zero packets are handled by receiveFile()
    return doAsync(data, len, true, false, handle);
}

static int async_write(void *data, size_t len, struct MtpDevHandle *handle)
{
    return doAsync(data, len, false, true, handle);
}

static void advise(int fd, struct MtpDevHandle *handle)
{
	struct ffs_control *ctrl = handle->descFd.ffsFd.mFfsCtrl;

	for (unsigned i = 0; i < NUM_IO_BUFS; i++) {
		if (posix_madvise(ctrl->mIobuf[i].bufs, MAX_FILE_CHUNK_SIZE,
			POSIX_MADV_SEQUENTIAL | POSIX_MADV_WILLNEED) < 0)
			fprintf(stderr, "Failed to madvise\n");
	}
	if (posix_fadvise(fd, 0, 0,
		POSIX_FADV_SEQUENTIAL | POSIX_FADV_NOREUSE | POSIX_FADV_WILLNEED) < 0)
		fprintf(stderr, "Failed to fadvise\n");
}

static int MtpDevHandleFfsReceiveFile(struct mtp_file_range *mfr, bool zero_packet,
					struct MtpDevHandle *handle)
{
	// When receiving files, the incoming length is given in 32 bits.
	// A >=4G file is given as 0xFFFFFFFF
	uint32_t file_length = mfr->length;
	off_t offset = mfr->offset;
	int mBulkOut = handle->descFd.ffsFd.mBulkOut;
	struct ffs_control *ctrl = handle->descFd.ffsFd.mFfsCtrl;

	struct aiocb aio;
	aio.aio_fildes = mfr->fd;
	aio.aio_buf = NULL;
	const struct aiocb *aiol[] = {&aio};

	int ret = -1;
	unsigned i = 0;
	size_t length;
	struct io_event ioevs[AIO_BUFS_MAX];
	bool has_write = false;
	bool error = false;
	bool write_error = false;
	int packet_size = getPacketSize(mBulkOut);
	bool short_packet = false;
	advise(mfr->fd, handle);

	// Break down the file into pieces that fit in buffers
	while (file_length > 0 || has_write) {
		// Queue an asynchronous read from USB.
		if (file_length > 0) {
			length = min((uint32_t)(MAX_FILE_CHUNK_SIZE), file_length);
			if (iobufSubmit(&ctrl->mIobuf[i], mBulkOut, length, true, handle) == -1)
				error = true;
		}

		// Get the return status of the last write request.
		if (has_write) {
			aio_suspend(aiol, 1, NULL);
			int written = aio_return(&aio);
			/*DLOG("written=%d, aio_nbytes=%u", written, aio.aio_nbytes);*/
			if ((size_t)(written) < aio.aio_nbytes) {
				errno = written == -1 ? aio_error(&aio) : EIO;
				fprintf(stderr, "Mtp error writing to disk\n");
				write_error = true;
			}
			has_write = false;
			fsync(mfr->fd);
		}

		if (error) {
			return -1;
		}

		// Get the result of the read request, and queue a write to disk.
		if (file_length > 0) {
			unsigned num_events = 0;
			ret = 0;
			unsigned short_i = ctrl->mIobuf[i].actual;
			while (num_events < short_i) {
				// Get all events up to the short read, if there is one.
				// We must wait for each event since data transfer could end at any time.
				int this_events = 0;
				int event_ret = waitEvents(&ctrl->mIobuf[i], 1, ioevs, &this_events, handle);
				num_events += this_events;

				/*printf("[%s] line:%d num_events=%d, short_i=%d, event_ret=%d, ret=%d, this_events=%d, file_length=%d, offset=%d\n", __func__, __LINE__,*/
					/*num_events, short_i, event_ret, ret, this_events, file_length, offset);*/
				if (event_ret == -1) {
					DLOG("event_ret=%d, errno=%d, num_events=%d, actual=%d", ret, errno, num_events, ctrl->mIobuf[i].actual);
					cancelEvents(ctrl->mIobuf[i].iocb, ioevs, num_events, ctrl->mIobuf[i].actual, handle);
					DLOG("errno=%d", ret, errno);
					return -1;
				}
				ret += event_ret;
				for (int j = 0; j < this_events; j++) {
					// struct io_event contains a pointer to the associated struct iocb as a __u64.
					if ((uint64_t)(ioevs[j].res) <
						(uint64_t)(((struct iocb*)(ioevs[j].obj))->u.c.nbytes)) {
						// We've found a short event. Store the index since
						// events won't necessarily arrive in the order they are queued.
						short_i = (unsigned)((unsigned long)ioevs[j].obj - (unsigned long)(ctrl->mIobuf[i].iocbs))
						/ sizeof(struct iocb) + 1;
						short_packet = true;
					}
				}
			}
			if (short_packet) {
				if (cancelEvents(ctrl->mIobuf[i].iocb, ioevs, short_i, ctrl->mIobuf[i].actual, handle)) {
					write_error = true;
				}
			}
			if (file_length == MAX_MTP_FILE_SIZE) {
				// For larger files, receive until a short packet is received.
				if ((size_t)(ret) < length) {
					file_length = 0;
				}
			} else if (ret < (int)(length)) {
				// If file is less than 4G and we get a short packet, it's an error.
				errno = EIO;
				fprintf(stderr, "Mtp got unexpected short packet\n");
				return -1;
			} else {
				file_length -= ret;
			}

			if (write_error) {
				cancelTransaction(handle);
				return -1;
			}
			// Enqueue a new write request
			#if 0
			aio_prepare(&aio, ctrl->mIobuf[i].bufs, ret, offset);
			#else
			aio.aio_buf = ctrl->mIobuf[i].bufs;
			aio.aio_offset = offset;
			aio.aio_nbytes = ret;
			#endif
			aio_write(&aio);

			offset += ret;
			i = (i + 1) % NUM_IO_BUFS;
			has_write = true;
			/*DLOG("[%s] line:%d file_length=%u, offset=%lu, ret=%d", __func__, __LINE__,*/
				/*file_length, offset, ret);*/
		}
	}
	DLOG("[%s] line:%d file_length=%u, offset=%lu\n", __func__, __LINE__,
		file_length, offset);
	DLOG("ret=%d, packet_size=%d\n", ret, packet_size);
	if ((ret % packet_size == 0 && !short_packet) || zero_packet) {
		// Receive an empty packet if size is a multiple of the endpoint size
		// and we didn't already get an empty packet from the header or large file.
		if (async_read(ctrl->mIobuf[0].bufs, packet_size, handle) != 0) {
			return -1;
		}
	}
	return 0;
}
#else
static int MtpDevHandleFfsReceiveFile(struct mtp_file_range *mfr, bool zero_packet,
					struct MtpDevHandle *handle)
{
	int ret = -1;
	int fd = mfr->fd;
	uint32_t file_length = mfr->length;
	off_t offset = mfr->offset;
	size_t length;
	void *data1 = handle->descFd.ffsFd.data1;
	void *data2 = handle->descFd.ffsFd.data2;
	void *data = data1;
	struct {
		void *data;
		uint32_t len;
	} wData;

	bool iread = false;
	bool iwrite = false;

	wData.len = 0;
	DLOG("file_length=%u, ret=%d, wData len:%d, offset=%u", file_length, ret, wData.len, offset);
	while (file_length > 0 || iwrite) {
		/*DLOG("file_length=%u, ret=%d, wData len:%d, offset=%u", file_length, ret, wData.len, offset);*/
		if (file_length > 0) {
			length = min(MAX_FILE_CHUNK_SIZE, file_length);
			ret = MtpDevHandleFfsRead(data, length, handle);
			if (ret != MAX_MTP_FILE_SIZE && ret < length) {
				ret = -1;
				errno = EIO;
			}
			iread = true;
		}
		if (iwrite) {
			ssize_t written;
			/*written = pwrite(fd, wData.data, wData.len, offset);*/
			written = write(fd, wData.data, wData.len);
			fsync(fd);
			/*DLOG("written=%ld", written);*/
			if (written < 0) {
				ret = -1;
				errno = EIO;
			}
			iwrite = false;
		}
		if (ret == -1) {
			return -1;
		}
		if (iread) {
			if (file_length == MAX_MTP_FILE_SIZE) {
				if (ret < length) {
					file_length = 0;
				}
			} else {
				file_length -= ret;
			}
			offset += ret;
			wData.data = data;
			wData.len = ret;
			if (data == data1) {
				data = data2;
			} else {
				data = data1;
			}
			iwrite = true;
			iread = false;
		}
	}
	DLOG("file_length=%u, ret=%d, wData len:%d, offset=%u", file_length, ret, wData.len, offset);
	return 0;
}
#endif

#ifdef USE_AIO
static int MtpDevHandleFfsSendFile(struct mtp_file_range *mfr, struct MtpDevHandle *handle)
{
	uint64_t file_length = mfr->length;
	uint32_t given_length = min((uint64_t)(MAX_MTP_FILE_SIZE),
		file_length + sizeof(struct mtp_data_header));
	off_t offset = mfr->offset;
	int mBulkIn = handle->descFd.ffsFd.mBulkIn;
	int packet_size = getPacketSize(mBulkIn);
	struct ffs_control *ctrl = handle->descFd.ffsFd.mFfsCtrl;

	// If file_length is larger than a size_t, truncating would produce the wrong comparison.
	// Instead, promote the left side to 64 bits, then truncate the small result.
	int init_read_len = min((uint64_t)(packet_size - sizeof(struct mtp_data_header)), file_length);

	advise(mfr->fd, handle);

	struct aiocb aio;
	aio.aio_fildes = mfr->fd;
	const struct aiocb *aiol[] = {&aio};
	int ret = 0;
	int length, num_read;
	unsigned i = 0;
	struct io_event ioevs[AIO_BUFS_MAX];
	bool error = false;
	bool has_write = false;

	// Send the header data
	struct mtp_data_header *header = (struct mtp_data_header *)(ctrl->mIobuf[0].bufs);
	header->length = htole32(given_length);
	header->type = htole16(2); // data packet
	header->command = htole16(mfr->command);
	header->transaction_id = htole32(mfr->transaction_id);

	// Some hosts don't support header/data separation even though MTP allows it
	// Handle by filling first packet with initial file data
	if ((pread(mfr->fd, ctrl->mIobuf[0].bufs +
		sizeof(struct mtp_data_header), init_read_len, offset))
		!= init_read_len)
		return -1;
	if (doAsync(ctrl->mIobuf[0].bufs, sizeof(struct mtp_data_header) + init_read_len,
		false, false, handle) == -1)
		return -1;
	file_length -= init_read_len;
	offset += init_read_len;
	ret = init_read_len + sizeof(struct mtp_data_header);

	DLOG("file_length:%"PRIu64", offset:%lu, init_read_len=%d, header:%u",
			file_length, offset, init_read_len, sizeof(struct mtp_data_header));
	// Break down the file into pieces that fit in buffers
	while(file_length > 0 || has_write) {
		if (file_length > 0) {
			// Queue up a read from disk.
			length = min((uint64_t)(MAX_FILE_CHUNK_SIZE), file_length);
			#if 0
			aio_prepare(&aio, mIobuf[i].bufs, length, offset);
			#else
			aio.aio_buf = ctrl->mIobuf[i].bufs;
			aio.aio_offset = offset;
			aio.aio_nbytes = length;
			#endif
			aio_read(&aio);
			/*DLOG("length:%d, file_length:%"PRIu64", ret=%d", length, file_length, ret);*/
		}

		if (has_write) {
			// Wait for usb write. Cancel unwritten portion if there's an error.
			int num_events = 0;
			if (waitEvents(&ctrl->mIobuf[(i-1)%NUM_IO_BUFS], ctrl->mIobuf[(i-1)%NUM_IO_BUFS].actual, ioevs, &num_events, handle) != ret) {
				error = true;
				cancelEvents(ctrl->mIobuf[(i-1)%NUM_IO_BUFS].iocb, ioevs, num_events,
				ctrl->mIobuf[(i-1)%NUM_IO_BUFS].actual, handle);
			}
			has_write = false;
		}

		if (file_length > 0) {
			// Wait for the previous read to finish
			aio_suspend(aiol, 1, NULL);
			num_read = aio_return(&aio);
			if ((size_t)(num_read) < aio.aio_nbytes) {
				errno = num_read == -1 ? aio_error(&aio) : EIO;
				DLOG("num_read=%d, aio_nbytes=%d", num_read, aio.aio_nbytes);
				fprintf(stderr, "Mtp error reading from disk\n");
				cancelTransaction(handle);
				return -1;
			}

			file_length -= num_read;
			offset += num_read;

			if (error) {
				return -1;
			}

			// Queue up a write to usb.
			if (iobufSubmit(&ctrl->mIobuf[i], mBulkIn, num_read, false, handle) == -1) {
				return -1;
			}
			has_write = true;
			ret = num_read;
			/*DLOG("file_length:%"PRIu64", offset:%ld, ret:%u", file_length, offset, ret);*/
		}

		i = (i + 1) % NUM_IO_BUFS;
	}

	DLOG("");
	if (ret % packet_size == 0) {
		// If the last packet wasn't short, send a final empty packet
		if (async_write(ctrl->mIobuf[0].bufs, 0, handle) != 0) {
			return -1;
		}
	}

	return 0;
}
#else
static int MtpDevHandleFfsSendFile(struct mtp_file_range *mfr, struct MtpDevHandle *handle)
{
	int ret = -1;
	int fd = mfr->fd;
	uint32_t file_length = mfr->length;
	off_t offset = mfr->offset;
	size_t length;
	void *data1 = handle->descFd.ffsFd.data1;
	void *data2 = handle->descFd.ffsFd.data2;
	void *data = data1;
	struct {
		void *data;
		uint32_t len;
	} wData;

	bool iread = false;
	bool iwrite = false;
	int error = 0;

	int packet_size = 512;
	int init_read_len = min(packet_size - sizeof(struct mtp_data_header), file_length);
	struct mtp_data_header *header = NULL;
	uint32_t given_length = file_length + sizeof(struct mtp_data_header);

	if (given_length > MAX_MTP_FILE_SIZE)
		given_length = MAX_MTP_FILE_SIZE;

	DLOG("given_length=%u, offset=%d\n", given_length, offset);
	header = (struct mtp_data_header *)data;
	header->length = cpu_to_le32(given_length);
	header->type = cpu_to_le16(2); /*  data packet */
	header->command = cpu_to_le16(mfr->command);
	header->transaction_id = cpu_to_le32(mfr->transaction_id);

	int size = 0;
	/*size = pread(fd, data + sizeof(struct mtp_data_header), init_read_len, offset);*/
	size = read(fd, data + sizeof(struct mtp_data_header), init_read_len);
	DLOG("[%s] line:%d mtp_data_header=%d, init_read_len=%d, offset=%d, size=%d\n", __func__, __LINE__,
			sizeof(struct mtp_data_header), init_read_len, offset, size);
	MtpDevHandleFfsWrite(data, sizeof(struct mtp_data_header) + init_read_len, handle);

	file_length -= init_read_len;
	offset += init_read_len;
	ret = init_read_len + sizeof(struct mtp_data_header);

	while (file_length > 0) {
		if (iread) {
			file_length -= ret;
			offset += ret;
			wData.data = data;
			wData.len = ret;
			/*DLOG("[%s] line:%d set iwrite, len=%u, offset=%u, file_length=%u", __func__, __LINE__, ret, offset, file_length);*/
			if (data == data1) {
				data = data2;
			} else {
				data = data1;
			}
			iwrite = true;
			iread = false;
		}
		if (error == -1)
			return -1;
		if (file_length > 0) {
			length = min(MAX_FILE_CHUNK_SIZE, file_length);
			ret = read(fd, data, length);
			iread = true;
		}
		if (iwrite) {
			if (MtpDevHandleWrite(wData.data, wData.len, handle) < 0)
				error = -1;
			iwrite = false;
		}
	}
#if 0
	if (ret % packet_size == 0) {
		MtpDevHandleFfsWrite(data, packet_size, handle);
	}
#endif
	return 0;
}
#endif

static int MtpDevHandleFfsSendEvent(struct mtp_event *em, struct MtpDevHandle *handle)
{
	int fd;

	fd = handle->descFd.ffsFd.mIntr;
	return write(fd, em->data, em->length);
}

static int MtpDevHandleFfsClose(struct MtpDevHandle *handle)
{
#if 0
	if (handle->descFd.ffsFd.mControl > 0)
		close(handle->descFd.ffsFd.mControl);
#endif

#ifdef USE_AIO
	ffs_control_deinit(handle->descFd.ffsFd.mFfsCtrl);
#endif

	if (handle->descFd.ffsFd.mBulkIn > 0)
		close(handle->descFd.ffsFd.mBulkIn);
	if (handle->descFd.ffsFd.mBulkOut > 0)
		close(handle->descFd.ffsFd.mBulkOut);
	if (handle->descFd.ffsFd.mIntr > 0)
		close(handle->descFd.ffsFd.mIntr);
	/*handle->descFd.ffsFd.mControl = -1;*/
	handle->descFd.ffsFd.mBulkIn = -1;
	handle->descFd.ffsFd.mBulkOut = -1;
	handle->descFd.ffsFd.mIntr = -1;
#ifndef USE_AIO
	if (handle->descFd.ffsFd.data1) {
		free(handle->descFd.ffsFd.data1);
		handle->descFd.ffsFd.data1 = NULL;
	}
	if (handle->descFd.ffsFd.data2) {
		free(handle->descFd.ffsFd.data2);
		handle->descFd.ffsFd.data2 = NULL;
	}
#endif
	return 0;
}

/* MtpDevHandle Interface */
struct MtpDevHandle *MtpDevHandleInit(int fd)
{
	int ret = 0;
	struct MtpDevHandle *handle;
	bool ffs_ok;


	handle = (struct MtpDevHandle *)calloc_wrapper(1, sizeof(struct MtpDevHandle));
	if (!handle) {
		fprintf(stderr, "no memory\n");
		return NULL;
	}

	ffs_ok = access(FFS_MTP_EP0, W_OK) == 0;
	DLOG("ffs_ok is %s\n", ffs_ok ? "true" : "false");
	if (ffs_ok) {
		handle->ops.open = MtpDevHandleFfsOpen;
		handle->ops.write = MtpDevHandleFfsWrite;
		handle->ops.read = MtpDevHandleFfsRead;
		handle->ops.receiveFile = MtpDevHandleFfsReceiveFile;
		handle->ops.sendFile = MtpDevHandleFfsSendFile;
		handle->ops.sendEvent = MtpDevHandleFfsSendEvent;
		handle->ops.close = MtpDevHandleFfsClose;
	} else {
		handle->ops.open = MtpDevHandleDirectOpen;
		handle->ops.write = MtpDevHandleDirectWrite;
		handle->ops.read = MtpDevHandleDirectRead;
		handle->ops.receiveFile = MtpDevHandleDirectReceiveFile;
		handle->ops.sendFile = MtpDevHandleDirectSendFile;
		handle->ops.sendEvent = MtpDevHandleDirectSendEvent;
		handle->ops.close = MtpDevHandleDirectClose;
	}
	handle->is_ffs = ffs_ok;

	if (ffs_ok) {
		handle->descFd.ffsFd.mControl = fd;
	}
	ret = handle->ops.open(handle);
	if (ret != 0) {
		fprintf(stderr, "%s ops open failed\n", ffs_ok ?
						"ffs" : "direct");
		free_wrapper(handle);
		return NULL;
	}

	return handle;
}

int MtpDevHandleRelease(struct MtpDevHandle *handle)
{
	int ret;

	ret = handle->ops.close(handle);
	free_wrapper(handle);
	return 0;
}

int MtpDevHandleRead(void *data, size_t len, struct MtpDevHandle *handle)
{
	if (!handle || !handle->ops.read)
		return -1;
	return handle->ops.read(data, len, handle);
}

int MtpDevHandleWrite(void *data, size_t len, struct MtpDevHandle *handle)
{
	if (!handle || !handle->ops.write)
		return -1;
	return handle->ops.write(data, len, handle);
}

int MtpDevHandleSendFile(struct mtp_file_range *mfr, struct MtpDevHandle *handle)
{
	if (!handle || !handle->ops.sendFile)
		return -1;
	return handle->ops.sendFile(mfr, handle);
}

int MtpDevHandleReceiveFile(struct mtp_file_range *mfr, bool zero_packet, struct MtpDevHandle *handle)
{
	if (!handle || !handle->ops.receiveFile)
		return -1;
	return handle->ops.receiveFile(mfr, zero_packet, handle);
}

int MtpDevHandleSendEvent(struct mtp_event *em, struct MtpDevHandle *handle)
{
	if (!handle || !handle->ops.sendEvent)
		return -1;
	return handle->ops.sendEvent(em, handle);
}
