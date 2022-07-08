#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include "download_curl.h"
#define DL_DEFAULT_RETRIES	(3)
#define DL_LOWSPEED_TIME	(300)
int download_from_url(const char *url, const char *dest_path, bool wait);
#endif
