/*
 *   This file is part of MiniGUI, a mature cross-platform windowing
 *   and Graphics User Interface (GUI) support system for embedded systems
 *   and smart IoT devices.
 *
 *   Copyright (C) 2002~2018, Beijing FMSoft Technologies Co., Ltd.
 *   Copyright (C) 1998~2002, WEI Yongming
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Or,
 *
 *   As this program is a library, any link to this program must follow
 *   GNU General Public License version 3 (GPLv3). If you cannot accept
 *   GPLv3, you need to be licensed from FMSoft.
 *
 *   If you have got a commercial license of this program, please use it
 *   under the terms and conditions of the commercial license.
 *
 *   For more information about the commercial license, please refer to
 *   <http://www.minigui.com/en/about/licensing-policy/>.
 */

#ifndef _GAL_sunxifb_h
#define _GAL_sunxifb_h

#ifdef _MGGAL_SUNXIFB

#include <linux/fb.h>
#include "sysvideo.h"

#ifdef _MGIMAGE_G2D
#include "g2d_driver_enh.h"
#include <ion_mem_alloc.h>
#endif

#ifdef _MGIMAGE_GPU
#include <stdbool.h>
#include <renderengine_uapi.h>
#include <drm_fourcc.h>
#include <ion_mem_alloc.h>
#endif

/* Hidden "this" pointer for the video functions */
#define _THIS   GAL_VideoDevice *this

#if defined(_MGIMAGE_G2D) || defined(_MGIMAGE_GPU)
struct fb_dmabuf_export {
    int fd;
    __u32 flags;
};

/* This is the structure we use to keep track of video memory */
/* g2d use addrPhy, gpu use fd */
typedef struct vidmem_bucket {
    int w, h;
    bool isScreen;
    unsigned long addrPhy;
#if _MGIMAGE_GPU
    struct fb_dmabuf_export fbDmaBuf;
    /* frame buffer is 2, other is 1 */
    unsigned long long target[2];
#endif
} vidmem_bucket;
#endif

/* Private display data */

struct GAL_PrivateVideoData {
    int w, h;
    int consoleFd;
    int flipPage; /* 0~1, 0 is current drawing */
    int flipBuffer; /* 0 does not turn pages, 1 page turns */
    int cacheFlag; /* 0 disable cache buffer, 1 enable cache buffer */
    char *flipAddress[2];
    int mappedOneBufLen;
    int mappedBufNum;
    int mappedMemLen;
    int mappedOffSet;
    int mappedDrawIndex;
    int mappedDispIndex;
    char *mappedMem;
    pthread_t updateTh;
    pthread_mutex_t updateLock;
    pthread_cond_t drawCond;
    struct fb_var_screeninfo cacheVinfo;
#ifdef _MGIMAGE_G2D
    int g2dFd;
    vidmem_bucket videoBucket;
    struct SunxiMemOpsS *pMemops;
#ifdef _MGIMAGE_G2D_ROTATE
    int rotate;
    vidmem_bucket rotateBucket;
#endif
#endif
#ifdef _MGIMAGE_GPU
    RenderEngine_t render;
    pthread_t gpuThreadId;
    vidmem_bucket videoBucket;
    struct SunxiMemOpsS *pMemops;
#ifdef _MGIMAGE_GPU_ROTATE
    int rotate;
    vidmem_bucket rotateBucket;
#endif
#endif
};

#endif /* _MGGAL_SUNXIFB */
#endif /* _GAL_sunxifb_h */
