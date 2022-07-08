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

/* SUNXIFB GAL video driver implementation; this is just enough to make an
 *  GAL-based application THINK it's got a working video driver, for
 *  applications that call GAL_Init(GAL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting GAL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that GAL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 */

/* MiniGUI.cfg
 * [sunxifb]
 * defaultmode=1280x800-32bpp
 * flipbuffer=1
 * cacheflag=1
 * rotate=0
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "common.h"
#include "newgal.h"
#include "sysvideo.h"
#include "pixels_c.h"

#ifdef _MGGAL_SUNXIFB

#include "sunxifb.h"
#include "minigui.h"
#include "memops.h"

#define FBIO_CACHE_SYNC         0x4630
#define FBIO_ENABLE_CACHE       0x4631
#define FBIO_GET_IONFD          0x4632
#define FBIO_GET_PHY_ADDR       0x4633
#define FBIOGET_DMABUF          _IOR('F', 0x21, struct fb_dmabuf_export)
/*#define SUNXIFB_DEBUG   1*/

#if _MGIMAGE_GPU
#if CPU_TARGET_R818
#define MY_FORMAT_ARGB8888 DRM_FORMAT_ARGB8888
#elif CPU_TARGET_R16
#define MY_FORMAT_ARGB8888 DRM_FORMAT_RGBA8888
#else
#define MY_FORMAT_ARGB8888 DRM_FORMAT_ARGB8888
#endif
#endif

/* Initialization/Query functions */
static int SUNXIFB_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect** SUNXIFB_ListModes(_THIS, GAL_PixelFormat *format,
        Uint32 flags);
static GAL_Surface* SUNXIFB_SetVideoMode(_THIS, GAL_Surface *current, int width,
        int height, int bpp, Uint32 flags);
static int SUNXIFB_SetColors(_THIS, int firstcolor, int ncolors,
        GAL_Color *colors);
static void SUNXIFB_VideoQuit(_THIS);
static int SUNXIFB_FlipHWSurface(_THIS, GAL_Surface *surface, GAL_Rect *rects,
        BOOL enable);
static int SUNXIFB_DoubleBufferEnable(_THIS, GAL_Surface *current, BOOL enable);
static int SUNXIFB_SlideHWSurface(_THIS, GAL_Surface *surface);
static void SUNXIFB_SyncDoubleBuffer(_THIS, GAL_Surface *surface,
        GAL_Rect *rects);

/* Hardware surface functions */
static int SUNXIFB_AllocHWSurface(_THIS, GAL_Surface *surface);
static void SUNXIFB_FreeHWSurface(_THIS, GAL_Surface *surface);
#if defined(_MGIMAGE_G2D) || defined(_MGIMAGE_GPU)
static int SUNXIFB_HWAccelBlit(_THIS, GAL_Surface *src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect);
static int SUNXIFB_HWAccelBld(_THIS, GAL_Surface *src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect);
static int SUNXIFB_AllocHWBuffer(_THIS, PBITMAP bmp, int nSize);
static void SUNXIFB_FreeHWBuffer(_THIS, PBITMAP bmp);
static int SUNXIFB_HWPutBoxAlpha(_THIS, PBITMAP src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect);
static int SUNXIFB_FillHWRect(_THIS, GAL_Surface *dst, GAL_Rect *rect,
        Uint32 color);
static int SUNXIFB_SetHWAlpha(_THIS, GAL_Surface *surface, Uint8 alpha);
static int SUNXIFB_HWPutBoxAlphaScaler(_THIS, PBITMAP src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect, GAL_Rect *devrect,
        GAL_Rect *scalrect);
static int SUNXIFB_SetHWColorKey(_THIS, GAL_Surface *surface, Uint32 key);
#endif

#if defined(_MGIMAGE_G2D_ROTATE) || defined(_MGIMAGE_GPU_ROTATE)
static void change_mouseXY_cw(int *x, int *y);
static void change_mouseXY_ccw(int *x, int *y);
static void change_mouseXY_cw180(int *x, int *y);
extern void (*__mg_ial_change_mouse_xy_hook)(int *x, int *y);
#endif

extern GAL_Surface *__gal_screen;

/* SUNXIFB driver bootstrap functions */

static void* task_do_update(void *data);

/* for task_do_update */
static int run_flag = 0;
static int end_flag = 0;

#ifdef SUNXIFB_DEBUG
static void print_vinfo(struct fb_var_screeninfo *vinfo) {
    fprintf(stderr, "Printing vinfo:\n");
    fprintf(stderr, "txres: %d\n", vinfo->xres);
    fprintf(stderr, "tyres: %d\n", vinfo->yres);
    fprintf(stderr, "txres_virtual: %d\n", vinfo->xres_virtual);
    fprintf(stderr, "tyres_virtual: %d\n", vinfo->yres_virtual);
    fprintf(stderr, "txoffset: %d\n", vinfo->xoffset);
    fprintf(stderr, "tyoffset: %d\n", vinfo->yoffset);
    fprintf(stderr, "tbits_per_pixel: %d\n", vinfo->bits_per_pixel);
    fprintf(stderr, "tgrayscale: %d\n", vinfo->grayscale);
    fprintf(stderr, "tnonstd: %d\n", vinfo->nonstd);
    fprintf(stderr, "tactivate: %d\n", vinfo->activate);
    fprintf(stderr, "theight: %d\n", vinfo->height);
    fprintf(stderr, "twidth: %d\n", vinfo->width);
    fprintf(stderr, "taccel_flags: %d\n", vinfo->accel_flags);
    fprintf(stderr, "tpixclock: %d\n", vinfo->pixclock);
    fprintf(stderr, "tleft_margin: %d\n", vinfo->left_margin);
    fprintf(stderr, "tright_margin: %d\n", vinfo->right_margin);
    fprintf(stderr, "tupper_margin: %d\n", vinfo->upper_margin);
    fprintf(stderr, "tlower_margin: %d\n", vinfo->lower_margin);
    fprintf(stderr, "thsync_len: %d\n", vinfo->hsync_len);
    fprintf(stderr, "tvsync_len: %d\n", vinfo->vsync_len);
    fprintf(stderr, "tsync: %d\n", vinfo->sync);
    fprintf(stderr, "tvmode: %d\n", vinfo->vmode);
    fprintf(stderr, "tred: %d/%d\n", vinfo->red.length, vinfo->red.offset);
    fprintf(stderr, "tgreen: %d/%d\n", vinfo->green.length,
            vinfo->green.offset);
    fprintf(stderr, "tblue: %d/%d\n", vinfo->blue.length, vinfo->blue.offset);
    fprintf(stderr, "talpha: %d/%d\n", vinfo->transp.length,
            vinfo->transp.offset);
}

static void print_finfo(struct fb_fix_screeninfo *finfo) {
    fprintf(stderr, "Printing finfo:\n");
    fprintf(stderr, "tsmem_start = %p\n", (char*) finfo->smem_start);
    fprintf(stderr, "tsmem_len = %d\n", finfo->smem_len);
    fprintf(stderr, "ttype = %d\n", finfo->type);
    fprintf(stderr, "ttype_aux = %d\n", finfo->type_aux);
    fprintf(stderr, "tvisual = %d\n", finfo->visual);
    fprintf(stderr, "txpanstep = %d\n", finfo->xpanstep);
    fprintf(stderr, "typanstep = %d\n", finfo->ypanstep);
    fprintf(stderr, "tywrapstep = %d\n", finfo->ywrapstep);
    fprintf(stderr, "tline_length = %d\n", finfo->line_length);
    fprintf(stderr, "tmmio_start = %p\n", (char*) finfo->mmio_start);
    fprintf(stderr, "tmmio_len = %d\n", finfo->mmio_len);
    fprintf(stderr, "taccel = %d\n", finfo->accel);
}
#endif

static int SUNXIFB_Available(void) {
    return (1);
}

static void SUNXIFB_DeleteDevice(GAL_VideoDevice *device) {
#if defined(_MGIMAGE_G2D) || defined(_MGIMAGE_GPU)
    /* Close ion driver */
    if (device->hidden->pMemops) {
        SunxiMemClose(device->hidden->pMemops);
        device->hidden->pMemops = NULL;
    }
#endif
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice* SUNXIFB_CreateDevice(int devindex) {
    GAL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (GAL_VideoDevice*) malloc(sizeof(GAL_VideoDevice));
    if (device) {
        memset(device, 0, (sizeof *device));
        device->hidden = (struct GAL_PrivateVideoData*) malloc(
                (sizeof *device->hidden));
    }
    if ((device == NULL) || (device->hidden == NULL)) {
        GAL_OutOfMemory();
        if (device) {
            free(device);
        }
        return (0);
    }
    memset(device->hidden, 0, (sizeof *device->hidden));

    /* Set the function pointers */
    device->VideoInit = SUNXIFB_VideoInit;
    device->ListModes = SUNXIFB_ListModes;
    device->SetVideoMode = SUNXIFB_SetVideoMode;
    device->SetColors = SUNXIFB_SetColors;
    device->VideoQuit = SUNXIFB_VideoQuit;
#ifndef _MGRM_THREADS
    device->RequestHWSurface = NULL;
#endif
    device->AllocHWSurface = SUNXIFB_AllocHWSurface;
    device->CheckHWBlit = NULL;
    device->FillHWRect = NULL;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha = NULL;
    device->FreeHWSurface = SUNXIFB_FreeHWSurface;
    device->FlipHWSurface = SUNXIFB_FlipHWSurface;
    device->DoubleBufferEnable = SUNXIFB_DoubleBufferEnable;
    device->SlideHWSurface = SUNXIFB_SlideHWSurface;
    device->SyncDoubleBuffer = SUNXIFB_SyncDoubleBuffer;

    device->free = SUNXIFB_DeleteDevice;

#ifdef _MGIMAGE_G2D
    device->HWAccelBlit = SUNXIFB_HWAccelBld;
    device->AllocHWBuffer = SUNXIFB_AllocHWBuffer;
    device->FreeHWBuffer = SUNXIFB_FreeHWBuffer;
    device->HWPutBoxAlpha = SUNXIFB_HWPutBoxAlpha;
    device->FillHWRect = SUNXIFB_FillHWRect;
    device->SetHWAlpha = SUNXIFB_SetHWAlpha;
    device->HWPutBoxAlphaScaler = SUNXIFB_HWPutBoxAlphaScaler;
    device->SetHWColorKey = SUNXIFB_SetHWColorKey;
#endif

#ifdef _MGIMAGE_GPU
    device->HWAccelBlit = SUNXIFB_HWAccelBld;
    device->AllocHWBuffer = SUNXIFB_AllocHWBuffer;
    device->FreeHWBuffer = SUNXIFB_FreeHWBuffer;
    device->HWPutBoxAlpha = SUNXIFB_HWPutBoxAlpha;
    device->HWPutBoxAlphaScaler = SUNXIFB_HWPutBoxAlphaScaler;
    device->FillHWRect = SUNXIFB_FillHWRect;
    device->SetHWAlpha = SUNXIFB_SetHWAlpha;
    device->SetHWColorKey = SUNXIFB_SetHWColorKey;
#endif

    device->doubleBufferStatus = FALSE;
    device->YOffsetHWSurface = FALSE;

    return device;
}

VideoBootStrap SUNXIFB_bootstrap = { "sunxifb", "sunxifb video driver",
        SUNXIFB_Available, SUNXIFB_CreateDevice };

static int SUNXIFB_VideoInit(_THIS, GAL_PixelFormat *vformat) {

    fprintf(stderr, "NEWGAL>SUNXIFB: Calling init method!\n");

    struct GAL_PrivateVideoData *data = this->hidden;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    const char *GAL_fbdev;
    int i;

    if (GetMgEtcIntValue("sunxifb", "flipbuffer", &data->flipBuffer) < 0)
        data->flipBuffer = 0;

    if (GetMgEtcIntValue("sunxifb", "cacheflag", &data->cacheFlag) < 0)
        data->cacheFlag = 0;

#if defined(_MGIMAGE_G2D) && defined(_MGIMAGE_G2D_ROTATE)
    if (GetMgEtcIntValue("sunxifb", "rotate", &data->rotate) < 0)
        data->rotate = 1024;
    switch (data->rotate) {
    case 0:
        data->rotate = 1024;
        break;
    case 90:
        data->rotate = G2D_ROT_90;
        __mg_ial_change_mouse_xy_hook = change_mouseXY_cw;
        break;
    case 180:
        data->rotate = G2D_ROT_180;
        __mg_ial_change_mouse_xy_hook = change_mouseXY_cw180;
        break;
    case 270:
        data->rotate = G2D_ROT_270;
        __mg_ial_change_mouse_xy_hook = change_mouseXY_ccw;
        break;
    default:
        data->rotate = 1024;
        break;
    }
#elif defined(_MGIMAGE_GPU) && defined(_MGIMAGE_GPU_ROTATE)
    if (GetMgEtcIntValue("sunxifb", "rotate", &data->rotate) < 0)
        data->rotate = SRC_ROTATE_DEGREE_00;
    switch (data->rotate) {
    case 0:
        data->rotate = SRC_ROTATE_DEGREE_00;
        break;
    case 90:
        /* gpu drawing is rotating counterclockwise */
        data->rotate = SRC_ROTATE_DEGREE_270;
        __mg_ial_change_mouse_xy_hook = change_mouseXY_ccw;
        break;
    case 180:
        data->rotate = SRC_ROTATE_DEGREE_180;
        __mg_ial_change_mouse_xy_hook = change_mouseXY_cw180;
        break;
    case 270:
        /* gpu drawing is rotating counterclockwise */
        data->rotate = SRC_ROTATE_DEGREE_90;
        __mg_ial_change_mouse_xy_hook = change_mouseXY_cw;
        break;
    default:
        data->rotate = SRC_ROTATE_DEGREE_00;
        break;
    }
#endif

    pthread_mutex_init(&data->updateLock, NULL);
    pthread_cond_init(&data->drawCond, NULL);

    /* Initialize the library */
    GAL_fbdev = getenv("FRAMEBUFFER");
    if (GAL_fbdev == NULL) {
        GAL_fbdev = "/dev/fb0";
    }
    data->consoleFd = open(GAL_fbdev, O_RDWR, 0);
    if (data->consoleFd < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Unable to open %s\n", GAL_fbdev);
        return (-1);
    }

    /* Get the type of video hardware */
    if (ioctl(data->consoleFd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get console hardware info\n");
        SUNXIFB_VideoQuit(this);
        return (-1);
    }

    /* Determine the current screen depth */
    if (ioctl(data->consoleFd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get console pixel format\n");
        SUNXIFB_VideoQuit(this);
        return (-1);
    }

    /* Memory map the device, compensating for buggy PPC mmap() */
    data->mappedOffSet = (((long) finfo.smem_start)
            - (((long) finfo.smem_start) & ~(getpagesize() - 1)));
    data->mappedMemLen = finfo.smem_len + data->mappedOffSet;
    data->mappedOneBufLen = finfo.line_length * vinfo.yres;
    data->mappedBufNum = data->mappedMemLen / data->mappedOneBufLen;
    /*data->mappedBufNum = 2;*/
    data->w = vinfo.xres;
    data->h = vinfo.yres;

    /* If flip buffer, open the cache */
    if (data->flipBuffer && data->cacheFlag && data->mappedBufNum > 1) {
        unsigned long args[2];
        args[0] = 1;
        if (ioctl(data->consoleFd, FBIO_ENABLE_CACHE, args) < 0) {
            GAL_SetError("NEWGAL>SUNXIFB: FBIO_ENABLE_CACHE failed\n");
        }
    }

    data->mappedMem = mmap(NULL, data->mappedMemLen, PROT_READ | PROT_WRITE,
            MAP_SHARED, data->consoleFd, 0);

    if (data->mappedMem == (char*) -1) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: Unable to memory map the video hardware\n");
        data->mappedMem = NULL;
        SUNXIFB_VideoQuit(this);
        return (-1);
    }

    vformat->BitsPerPixel = vinfo.bits_per_pixel;
    if (vformat->BitsPerPixel < 8) {
        vformat->MSBLeft = !(vinfo.red.msb_right);
        return 0;
    }
    for (i = 0; i < vinfo.red.length; ++i) {
        vformat->Rmask <<= 1;
        vformat->Rmask |= (0x00000001 << vinfo.red.offset);
    }
    for (i = 0; i < vinfo.green.length; ++i) {
        vformat->Gmask <<= 1;
        vformat->Gmask |= (0x00000001 << vinfo.green.offset);
    }
    for (i = 0; i < vinfo.blue.length; ++i) {
        vformat->Bmask <<= 1;
        vformat->Bmask |= (0x00000001 << vinfo.blue.offset);
    }
    for (i = 0; i < vinfo.transp.length; ++i) {
        vformat->Amask <<= 1;
        vformat->Amask |= (0x00000001 << vinfo.transp.offset);
    }

#ifdef SUNXIFB_DEBUG
    print_vinfo(&vinfo);
    print_finfo(&finfo);
#endif

    /* Set flip address */
    if (data->mappedBufNum == 2) {
        data->flipAddress[0] = data->mappedMem + data->mappedOffSet;
        data->flipAddress[1] = data->flipAddress[0] + data->mappedOneBufLen;
    }

#ifdef _MGIMAGE_G2D
    if ((data->g2dFd = open("/dev/g2d", O_RDWR)) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Open /dev/g2d fail!\n");
        return (0);
    }

    /* The sunxifb g2d has an accelerated color fill */
    this->info.blit_fill = 1;
#endif

#ifdef _MGIMAGE_GPU
    unsigned int dstFormat;
    /* Get the fd of framebuffer */
    if (ioctl(data->consoleFd, FBIOGET_DMABUF, &data->videoBucket.fbDmaBuf)
    < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get framebuffer ion fd \n");
        return (0);
    }

    switch (vformat->BitsPerPixel) {
    case 32:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        dstFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        dstFormat = DRM_FORMAT_RGB565;
        break;
    default:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    }

    data->gpuThreadId = pthread_self();
    data->render = renderEngineCreate(false, 0);

    data->videoBucket.target[0] = renderEngineCreateOffScreenTarget(
            data->render, data->w, data->h, dstFormat,
            data->videoBucket.fbDmaBuf.fd, 0);

    data->videoBucket.target[1] = renderEngineCreateOffScreenTarget(
            data->render, data->w, data->h, dstFormat,
            data->videoBucket.fbDmaBuf.fd, data->mappedOneBufLen);

    /* The filling color is not as efficient as the cpu */
    /* this->info.blit_fill = 1; */
#endif

#if defined(_MGIMAGE_G2D) || defined(_MGIMAGE_GPU)
    data->pMemops = GetMemAdapterOpsS();
    if (SunxiMemOpen(data->pMemops) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Open /dev/ion error!\n");
        return (0);
    }
#endif

    /* We're done! */
    return (0);
}

static GAL_Rect** SUNXIFB_ListModes(_THIS, GAL_PixelFormat *format,
        Uint32 flags) {
    if (format->BitsPerPixel < 8) {
        return NULL;
    }

    return (GAL_Rect**) -1;
}

static GAL_Surface* SUNXIFB_SetVideoMode(_THIS, GAL_Surface *current, int width,
        int height, int bpp, Uint32 flags) {
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    struct GAL_PrivateVideoData *data = this->hidden;
    int i;
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;

    /* Set the video mode and get the final screen format */
    if (ioctl(data->consoleFd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get console screen info");
        return (NULL);
    }

    Rmask = 0;
    for (i = 0; i < vinfo.red.length; ++i) {
        Rmask <<= 1;
        Rmask |= (0x00000001 << vinfo.red.offset);
    }
    Gmask = 0;
    for (i = 0; i < vinfo.green.length; ++i) {
        Gmask <<= 1;
        Gmask |= (0x00000001 << vinfo.green.offset);
    }
    Bmask = 0;
    for (i = 0; i < vinfo.blue.length; ++i) {
        Bmask <<= 1;
        Bmask |= (0x00000001 << vinfo.blue.offset);
    }
    Amask = 0;
    for (i = 0; i < vinfo.transp.length; ++i) {
        Amask <<= 1;
        Amask |= (0x00000001 << vinfo.transp.offset);
    }

    if (!GAL_ReallocFormat(current, vinfo.bits_per_pixel, Rmask, Gmask, Bmask,
            Amask)) {
        return (NULL);
    }
    if (vinfo.bits_per_pixel < 8) {
        current->format->MSBLeft = !(vinfo.red.msb_right);
    }

    /* Get the fixed information about the console hardware.
     This is necessary since finfo.line_length changes.
     */
    if (ioctl(data->consoleFd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: Couldn't get console hardware info");
        return (NULL);
    }

    data->cacheVinfo = vinfo;

    /* Set up the new mode framebuffer */
    current->flags = (GAL_FULLSCREEN | GAL_HWSURFACE);
    current->w = vinfo.xres;
    current->h = vinfo.yres;
    current->pitch = finfo.line_length;
    current->pixels = data->mappedMem + data->mappedOffSet;

    if (data->flipBuffer) {
        /* The number of buffers is greater than 1 to flip pages */
        if (data->mappedBufNum > 1) {
            current->flags |= GAL_DOUBLEBUF;
            this->doubleBufferStatus = TRUE;
        }
        if (data->mappedBufNum > 2) {
            /* Calculate drawing and display buffer index */
            data->mappedDispIndex = vinfo.yoffset / data->h;
            data->mappedDrawIndex =
                    data->mappedDispIndex >= (data->mappedBufNum - 1) ?
                            0 : data->mappedDispIndex + 1;
            /* The draw index buffer is used to draw the image */
            current->pixels = data->mappedMem
                    + data->mappedDrawIndex * data->mappedOneBufLen;

            /* UI refresh thread */
            pthread_attr_t new_attr;
            run_flag = 1;
            end_flag = 0;
            pthread_attr_init(&new_attr);
            pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&data->updateTh, &new_attr, task_do_update, this);
            pthread_attr_destroy(&new_attr);
        } else if (data->mappedBufNum == 2) {
            /* Ensure that the last image is not cleared */
            if (vinfo.yoffset == 0) {
                data->flipPage = !data->flipPage;
                current->pixels = data->flipAddress[data->flipPage];
            }
        }
    }

#if defined(_MGIMAGE_G2D) || defined(_MGIMAGE_GPU)
    data->videoBucket.isScreen = TRUE;
    data->videoBucket.w = vinfo.xres_virtual;
    data->videoBucket.h = vinfo.yres_virtual;
    data->videoBucket.addrPhy = finfo.smem_start;
    /* Save frame buffer info on the screen surface*/
    current->hwdata = (void*) &data->videoBucket;
#if defined(_MGIMAGE_G2D_ROTATE) || defined(_MGIMAGE_GPU_ROTATE)
    current->pixels = SunxiMemPalloc(this->hidden->pMemops,
            current->h * current->pitch);

    if (0 == current->pixels)
        return (NULL);

    data->rotateBucket.addrPhy = (unsigned long) SunxiMemGetPhysicAddressCpu(
            this->hidden->pMemops, current->pixels);
    /* Update w/h/pitch/pixels of screen surface */
#ifdef _MGIMAGE_G2D_ROTATE
    if (data->rotate == G2D_ROT_90 || data->rotate == G2D_ROT_270) {
#else _MGIMAGE_GPU_ROTATE
    if (data->rotate == SRC_ROTATE_DEGREE_90
            || data->rotate == SRC_ROTATE_DEGREE_270) {
#endif
        current->w = vinfo.yres;
        current->h = vinfo.xres;
        current->pitch = finfo.line_length / vinfo.xres * vinfo.yres;
    }
    /* Save rotate buffer info on the private video data */
    data->rotateBucket.isScreen = TRUE;
    data->rotateBucket.w = current->w;
    data->rotateBucket.h = current->h;

#if _MGIMAGE_GPU
    unsigned int dstFormat;
    switch (current->format->BitsPerPixel) {
    case 32:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        dstFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        dstFormat = DRM_FORMAT_RGB565;
        break;
    default:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    }
    data->rotateBucket.fbDmaBuf.fd = SunxiMemGetBufferFd(this->hidden->pMemops,
            current->pixels);
    data->rotateBucket.target[0] = renderEngineCreateOffScreenTarget(
            data->render, data->rotateBucket.w, data->rotateBucket.h, dstFormat,
            data->rotateBucket.fbDmaBuf.fd, 0);
#endif
#endif
#endif

    /* We're done */
    return (current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int SUNXIFB_AllocHWSurface(_THIS, GAL_Surface *surface) {
#if defined(_MGIMAGE_G2D) || defined(_MGIMAGE_GPU)
    vidmem_bucket *bucket;

    surface->pixels = SunxiMemPalloc(this->hidden->pMemops,
            surface->h * surface->pitch);
    if (0 == surface->pixels)
        return (-1);

    bucket = (vidmem_bucket*) malloc(sizeof(*bucket));
    if (bucket == NULL) {
        SunxiMemPfree(this->hidden->pMemops, surface->pixels);
        GAL_OutOfMemory();
        return (-1);
    }

    bucket->isScreen = FALSE;
    bucket->w = surface->w;
    bucket->h = surface->h;
    surface->flags |= GAL_HWSURFACE;
#if _MGIMAGE_GPU
    unsigned int dstFormat;
    switch (surface->format->BitsPerPixel) {
    case 32:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        dstFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        dstFormat = DRM_FORMAT_RGB565;
        break;
    default:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    }
    bucket->fbDmaBuf.fd = SunxiMemGetBufferFd(this->hidden->pMemops,
            surface->pixels);
    if (this->hidden->gpuThreadId == pthread_self())
        bucket->target[0] = renderEngineCreateOffScreenTarget(
                this->hidden->render, bucket->w, bucket->h, dstFormat,
                bucket->fbDmaBuf.fd, 0);
    else
        bucket->target[0] = 0;
#endif
    bucket->addrPhy = (unsigned long) SunxiMemGetPhysicAddressCpu(
            this->hidden->pMemops, surface->pixels);
    surface->hwdata = (void*) bucket;

#ifdef SUNXIFB_DEBUG
#if _MGIMAGE_GPU
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_AllocHWSurface fd=%d ddrPhy=%p ddr=%p, w=%d h=%d target=%p\n",
            bucket->fbDmaBuf.fd, bucket->addrPhy, surface->pixels, bucket->w,
            bucket->h, bucket->target[0]);
#else
    GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_AllocHWSurface ddrPhy=%p ddr=%p\n",
            bucket->addrPhy, surface->pixels);
#endif
#endif
    return 0;
#else
    return (-1);
#endif
}

static void SUNXIFB_FreeHWSurface(_THIS, GAL_Surface *surface) {
#if defined(_MGIMAGE_G2D) || defined(_MGIMAGE_GPU)
    vidmem_bucket *bucket = (vidmem_bucket*) surface->hwdata;
    if (bucket && !bucket->isScreen) {
        if (surface->pixels) {
#ifdef SUNXIFB_DEBUG
#if _MGIMAGE_GPU
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_FreeHWSurface fd=%d ddrPhy=%p ddr=%p, w=%d h=%d target=%p\n",
            bucket->fbDmaBuf.fd, bucket->addrPhy, surface->pixels, bucket->w,
            bucket->h, bucket->target[0]);
#else
    GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_FreeHWSurface ddrPhy=%p ddr=%p\n",
            bucket->addrPhy, surface->pixels);
#endif
#endif
#if _MGIMAGE_GPU
            if (bucket->target[0])
                renderEngineDestroyOffScreenTarget(this->hidden->render,
                        bucket->target[0]);
#endif
            SunxiMemPfree(this->hidden->pMemops, surface->pixels);
            surface->pixels = NULL;
        }
        free(surface->hwdata);
        surface->hwdata = NULL;
    }
#else
    surface->pixels = NULL;
#endif
}

static int SUNXIFB_SetColors(_THIS, int firstcolor, int ncolors,
        GAL_Color *colors) {
    /* do nothing of note. */
    return (1);
}

/* Note:  If we are terminated, this could be called in the middle of
 * another video routine -- notably UpdateRects.
 */
static void SUNXIFB_VideoQuit(_THIS) {
    struct GAL_PrivateVideoData *data = this->hidden;

    if (data->updateTh > 0) {
        run_flag = 0;
        /* waiting task_do_update end */
        for (;;) {
            if (end_flag != 0) {
                break;
            }
        }
    }

    pthread_mutex_destroy(&data->updateLock);
    pthread_cond_destroy(&data->drawCond);

    if (this->screen) {
        if (this->screen->pixels) {
            memset(this->screen->pixels, 0,
                    this->screen->h * this->screen->pitch);
        }

#if _MGIMAGE_GPU
        renderEngineDestroyOffScreenTarget(data->render,
                data->videoBucket.target[0]);
        renderEngineDestroyOffScreenTarget(data->render,
                data->videoBucket.target[1]);
#if _MGIMAGE_GPU_ROTATE
        renderEngineDestroyOffScreenTarget(data->render,
                data->rotateBucket.target[0]);
#endif
#endif

#if (defined(_MGIMAGE_G2D) && defined(_MGIMAGE_G2D_ROTATE)) \
    || (defined(_MGIMAGE_GPU) && defined(_MGIMAGE_GPU_ROTATE))
        if (this->screen->pixels) {
            SunxiMemPfree(data->pMemops, this->screen->pixels);
            this->screen->pixels = NULL;
        }
#else
        /* This test fails when using the VGA16 shadow memory */
        if (((char*) this->screen->pixels >= data->mappedMem)
                && ((char*) this->screen->pixels
                        < (data->mappedMem + data->mappedMemLen))) {
            this->screen->pixels = NULL;
        }
#endif
    }

    /* Close console and input file descriptors */
    if (data->consoleFd > 0) {
        if (data->flipBuffer && data->cacheFlag && data->mappedBufNum > 1) {
            unsigned long args[2];
            args[0] = 0;
            if (ioctl(data->consoleFd, FBIO_ENABLE_CACHE, args) < 0) {
                GAL_SetError(
                        "NEWGAL>SUNXIFB: FBIO_ENABLE_CACHE disable failed\n");
            }
        }

        /* Unmap the video framebuffer and I/O registers */
        if (data->mappedMem) {
            munmap(data->mappedMem, data->mappedMemLen);
            data->mappedMem = NULL;
        }

        /* We're all done with the framebuffer */
        close(data->consoleFd);
        data->consoleFd = -1;
    }
#ifdef _MGIMAGE_G2D
    /* Close g2d driver */
    if (data->g2dFd > 0) {
        close(data->g2dFd);
        data->g2dFd = -1;
    }
#endif

#ifdef _MGIMAGE_GPU
    renderEngineDestroy(data->render);
#endif
}

static int SUNXIFB_FlipHWSurface(_THIS, GAL_Surface *surface, GAL_Rect *rects,
        BOOL enable) {

    pthread_mutex_lock(&this->hidden->updateLock);

    struct GAL_PrivateVideoData *data = this->hidden;

    if (this->doubleBufferStatus) {
        if (data->mappedBufNum > 2) {
            int tempIndex = data->mappedDrawIndex + 1;
            if (tempIndex > data->mappedBufNum - 1)
                tempIndex = 0;
            /* Draw buffer full */
            if (tempIndex == data->mappedDispIndex)
                pthread_cond_wait(&data->drawCond, &data->updateLock);

            /* Refresh the cache */
            if (data->cacheFlag) {
                unsigned long args[2];
                args[0] = (unsigned long) data->mappedMem
                        + data->mappedDrawIndex * data->mappedOneBufLen;
                args[1] = data->mappedOneBufLen;
                if (ioctl(data->consoleFd, FBIO_CACHE_SYNC, args) < 0) {
                    GAL_SetError("NEWGAL>SUNXIFB: FBIO_CACHE_SYNC failed\n");
                }
            }

            /* Copy the drawn image to the buffer to be displayed */
            GAL_memcpy(data->mappedMem + tempIndex * data->mappedOneBufLen,
                    data->mappedMem
                            + data->mappedDrawIndex * data->mappedOneBufLen,
                    data->mappedOneBufLen);
            data->mappedDrawIndex = tempIndex;
            /* Set minigui drawing address */
            this->screen->pixels = data->mappedMem
                    + data->mappedDrawIndex * data->mappedOneBufLen;
        } else if (data->mappedBufNum == 2) {

#if defined(_MGIMAGE_G2D) && defined(_MGIMAGE_G2D_ROTATE)
            /* rotate buffer info */
            GAL_Rect srcRects = { 0, 0, surface->w, surface->h };
            /* frame buffer info */
            GAL_Rect dstRects =
                    { 0, data->flipPage * data->h, data->w, data->h };
            SunxiMemFlushCache(data->pMemops, surface->pixels,
                    surface->h * surface->pitch);
            SUNXIFB_HWAccelBlit(this, surface, &srcRects, surface, &dstRects);
#elif defined(_MGIMAGE_GPU) && defined(_MGIMAGE_GPU_ROTATE)
            /* rotate buffer info */
            GAL_Rect srcRects = { 0, 0, surface->w, surface->h };
            /* frame buffer info */
            GAL_Rect dstRects = { 0, 0, data->w, data->h };
            SunxiMemFlushCache(data->pMemops, surface->pixels,
                    surface->h * surface->pitch);
            SUNXIFB_HWAccelBlit(this, surface, &srcRects, surface, &dstRects);
#endif

            /* Refresh the cache */
            if (data->cacheFlag) {
                unsigned long args[2];
                args[0] = (unsigned long) data->flipAddress[data->flipPage];
                args[1] = data->mappedOneBufLen;
                if (ioctl(data->consoleFd, FBIO_CACHE_SYNC, args) < 0) {
                    GAL_SetError("NEWGAL>SUNXIFB: FBIO_CACHE_SYNC failed\n");
                }
            }

            data->cacheVinfo.yoffset = data->flipPage * data->h;
            if (ioctl(data->consoleFd, FBIOPAN_DISPLAY, &data->cacheVinfo)
                    < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
                pthread_mutex_unlock(&data->updateLock);
                return (-1);
            }

#if defined(_MGIMAGE_G2D) && !defined(_MGIMAGE_G2D_ROTATE)
            GAL_Rect srcRects = { 0, data->flipPage * surface->h, surface->w, surface->h };
            GAL_Rect dstRects = { 0, !data->flipPage * surface->h, surface->w, surface->h };
            SUNXIFB_HWAccelBlit(this, surface, &srcRects, surface, &dstRects);
#elif defined(_MGIMAGE_GPU) && !defined(_MGIMAGE_GPU_ROTATE) && 0
            /* The gpu copy efficiency is lower than the fb after the cache is turned on */
            GAL_Rect srcRects = { 0, 0, surface->w, surface->h };
            GAL_Rect dstRects = { 0, 0, surface->w, surface->h };
            SUNXIFB_HWAccelBlit(this, surface, &srcRects, surface, &dstRects);
#elif !defined(_MGIMAGE_G2D_ROTATE) && !defined(_MGIMAGE_GPU_ROTATE)
            GAL_memcpy(data->flipAddress[!data->flipPage],
                    data->flipAddress[data->flipPage], data->mappedOneBufLen);
#endif

            data->flipPage = !data->flipPage;
#if !defined(_MGIMAGE_G2D_ROTATE) && !defined(_MGIMAGE_GPU_ROTATE)
            this->screen->pixels = data->flipAddress[data->flipPage];
#endif

#ifdef SUNXIFB_DEBUG
            static struct timeval new, old;
            static int fps;
            gettimeofday(&new, NULL);
            if (new.tv_sec * 1000 - old.tv_sec * 1000 >= 1000) {
                GAL_SetError(
                        "NEWGAL>SUNXIFB: Flip double buffer fps is %d, current drawing page is %d\n",
                        fps, data->flipPage);
                old = new;
                fps = 0;
            } else {
                fps++;
            }
#endif
        }
    }
    pthread_mutex_unlock(&this->hidden->updateLock);
    return 0;
}

static int SUNXIFB_DoubleBufferEnable(_THIS, GAL_Surface *current, BOOL enable) {
    pthread_mutex_lock(&this->hidden->updateLock);
    struct GAL_PrivateVideoData *data = this->hidden;

    /* Refresh the cache */
    if (data->cacheFlag) {
        unsigned long args[2];
        args[0] = enable;
        if (ioctl(data->consoleFd, FBIO_ENABLE_CACHE, args) < 0) {
            GAL_SetError("NEWGAL>SUNXIFB: FBIO_ENABLE_CACHE disable failed\n");
            pthread_mutex_unlock(&data->updateLock);
            return (-1);
        }

        if (data->mappedMem) {
            munmap(data->mappedMem, data->mappedMemLen);
            data->mappedMem = NULL;
        }

        data->mappedMem = mmap(NULL, data->mappedMemLen, PROT_READ | PROT_WRITE,
                MAP_SHARED, data->consoleFd, 0);
        if (data->mappedMem == (char*) -1) {
            GAL_SetError(
                    "NEWGAL>SUNXIFB: Unable to memory map the video hardware\n");
            data->mappedMem = NULL;
            pthread_mutex_unlock(&data->updateLock);
            return (-1);
        }

#if !defined(_MGIMAGE_G2D_ROTATE) && !defined(_MGIMAGE_GPU_ROTATE)
        current->pixels = data->mappedMem + data->mappedOffSet;
#endif
    }

    /* Set flip address */
    if (data->mappedBufNum > 2) {
        /* The last buffer is used to draw the image */
        this->screen->pixels = data->mappedMem
                + data->mappedDrawIndex * data->mappedOneBufLen;

        if (!enable) {
            /* Must be assigned first, otherwise task_do_update may still be running,
             * resulting in yoffset value is not normal */
            this->doubleBufferStatus = enable;

            data->cacheVinfo.yoffset = data->mappedDrawIndex * data->h;
            if (ioctl(data->consoleFd, FBIOPAN_DISPLAY, &data->cacheVinfo)
                    < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
            }
        } else {
            /* Copy the current draw image to the disp buffer */
            GAL_memcpy(
                    data->mappedMem
                            + data->mappedDispIndex * data->mappedOneBufLen,
                    data->mappedMem
                            + data->mappedDrawIndex * data->mappedOneBufLen,
                    data->mappedOneBufLen);

            data->cacheVinfo.yoffset = data->mappedDispIndex * data->h;
            if (ioctl(data->consoleFd, FBIOPAN_DISPLAY, &data->cacheVinfo)
                    < 0) {
                GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
            }
        }

        pthread_mutex_unlock(&data->updateLock);
    } else if (data->mappedBufNum == 2) {
        data->flipAddress[0] = data->mappedMem + this->hidden->mappedOffSet;
        data->flipAddress[1] = data->flipAddress[0] + data->mappedOneBufLen;

        pthread_mutex_unlock(&data->updateLock);

        if (!enable) {
            data->flipPage = !data->flipPage;
#if !defined(_MGIMAGE_G2D_ROTATE) && !defined(_MGIMAGE_GPU_ROTATE)
            this->screen->pixels = data->flipAddress[data->flipPage];
#endif
        } else {
            GAL_Rect rc;
            rc.x = 0;
            rc.y = 0;
            rc.w = data->w;
            rc.h = data->h;
            /* Must be assigned first, otherwise Flip cannot be */
            this->doubleBufferStatus = enable;
            SUNXIFB_FlipHWSurface(this, current, &rc, TRUE);
        }
    }
    return 0;
}

/*
 * Do not copy the buffer, directly change the yoffset
 */
static int SUNXIFB_SlideHWSurface(_THIS, GAL_Surface *surface) {
    struct GAL_PrivateVideoData *data = this->hidden;

    if (data->mappedBufNum > 2) {
        GAL_SetError("NEWGAL>SUNXIFB: SlideHWSurface does not support\n");
        return (-1);
    }

    pthread_mutex_lock(&data->updateLock);

#if defined(_MGIMAGE_G2D) && defined(_MGIMAGE_G2D_ROTATE)
    /* rotate buffer info */
    GAL_Rect srcRects = { 0, 0, surface->w, surface->h };
    /* frame buffer info */
    GAL_Rect dstRects = { 0, data->flipPage * data->h, data->w, data->h };
    SunxiMemFlushCache(data->pMemops, surface->pixels,
            surface->h * surface->pitch);
    SUNXIFB_HWAccelBlit(this, surface, &srcRects, surface, &dstRects);
#elif defined(_MGIMAGE_GPU) && defined(_MGIMAGE_GPU_ROTATE)
    if (this->hidden->gpuThreadId != pthread_self()) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_SlideHWSurface cannot be called outside gpuThreadId\n");
        pthread_mutex_unlock(&data->updateLock);
        return (-1);
    }
    /* rotate buffer info */
    GAL_Rect srcRects = { 0, 0, surface->w, surface->h };
    /* frame buffer info */
    GAL_Rect dstRects = { 0, 0, surface->w, surface->h };
    SunxiMemFlushCache(data->pMemops, surface->pixels,
            surface->h * surface->pitch);
    SUNXIFB_HWAccelBlit(this, surface, &srcRects, surface, &dstRects);
#endif

    data->cacheVinfo.yoffset = data->flipPage * data->h;
    if (ioctl(data->consoleFd, FBIOPAN_DISPLAY, &data->cacheVinfo) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
        pthread_mutex_unlock(&data->updateLock);
        return (-1);
    }

    data->flipPage = !data->flipPage;

#if !defined(_MGIMAGE_G2D_ROTATE) && !defined(_MGIMAGE_GPU_ROTATE)
    this->screen->pixels = data->flipAddress[data->flipPage];
#endif

    pthread_mutex_unlock(&data->updateLock);

#ifdef SUNXIFB_DEBUG
    static struct timeval new, old;
    static int fps;
    gettimeofday(&new, NULL);
    if (new.tv_sec * 1000 - old.tv_sec * 1000 >= 1000) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: Slide buffer fps is %d, current drawing page is %d\n",
                fps, data->flipPage);
        old = new;
        fps = 0;
    } else {
        fps++;
    }
#endif
    return (0);
}

static void SUNXIFB_BlitCopy(GAL_BlitInfo *info) {
    Uint8 *src, *dst;
    int w, h;
    int srcskip, dstskip;

    w = info->d_width * info->dst->BytesPerPixel;
    h = info->d_height;
    src = info->s_pixels;
    dst = info->d_pixels;
    srcskip = w + info->s_skip;
    dstskip = w + info->d_skip;

    while (h--) {
        if (((DWORD) dst & 0x03) == 0 && ((DWORD) src & 0x03) == 0
                && (w & 0x03) == 0)
            GAL_memcpy4(dst, src, w >> 2);
        else
            GAL_memcpy(dst, src, w);
        src += srcskip;
        dst += dstskip;
    }
}

static void SUNXIFB_SyncDoubleBuffer(_THIS, GAL_Surface *surface,
        GAL_Rect *rects) {
    struct GAL_PrivateVideoData *data = this->hidden;
    pthread_mutex_lock(&data->updateLock);

#if defined(_MGIMAGE_G2D) && !defined(_MGIMAGE_G2D_ROTATE)
    GAL_Rect srcRects = { rects->x, data->flipPage * rects->y, rects->w,
            rects->h };
    GAL_Rect dstRects = { rects->x, !data->flipPage * rects->y, rects->w,
            rects->h };
    SUNXIFB_HWAccelBlit(this, surface, &srcRects, surface, &dstRects);
#elif defined(_MGIMAGE_GPU) && !defined(_MGIMAGE_GPU_ROTATE) && 0
    if (this->hidden->gpuThreadId != pthread_self()) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_SyncDoubleBuffer cannot be called outside gpuThreadId\n");
        pthread_mutex_unlock(&data->updateLock);
        return (-1);
    }
    /* The gpu copy efficiency is lower than the fb after the cache is turned on */
    /* TODO dstIndex should be !data->flipPage */
    GAL_Rect srcRects = { rects->x, rects->y, rects->w, rects->h };
    GAL_Rect dstRects = { rects->x, rects->y, rects->w, rects->h };
    SUNXIFB_HWAccelBlit(this, surface, &srcRects, surface, &dstRects);
#else
    GAL_BlitInfo info;

    //Set up the blit information
    info.s_pixels = (Uint8*) data->flipAddress[data->flipPage] + surface->offset
            + (Uint16) rects->y * surface->pitch
            + (Uint16) rects->x * surface->format->BytesPerPixel;
    info.s_width = rects->w;
    info.s_height = rects->h;
    info.s_skip = surface->pitch
            - info.s_width * surface->format->BytesPerPixel;
    info.d_pixels = (Uint8*) data->flipAddress[!data->flipPage]
            + surface->offset + (Uint16) rects->y * surface->pitch
            + (Uint16) rects->x * surface->format->BytesPerPixel;
    info.d_width = rects->w;
    info.d_height = rects->h;
    info.d_skip = surface->pitch
            - info.d_width * surface->format->BytesPerPixel;
    info.aux_data = surface->map->sw_data->aux_data;
    info.src = surface->format;
    info.table = surface->map->table;
    info.dst = surface->format;

    SUNXIFB_BlitCopy(&info);
#endif

    pthread_mutex_unlock(&data->updateLock);
}

#ifdef _MGIMAGE_G2D
static int SUNXIFB_HWAccelBlit(_THIS, GAL_Surface *src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *srcBucket = (vidmem_bucket*) src->hwdata;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    g2d_blt_h info;

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBlit srcSddrPhy=%p src=[%d, %d, %d, %d] srcrect=[%d, %d, %d, %d] "
                    "dstAddrPhy=%p dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            srcBucket->addrPhy, src->w, src->h, srcBucket->w, srcBucket->h,
            srcrect->x, srcrect->y, srcrect->w, srcrect->h, dstBucket->addrPhy,
            dst->w, dst->h, dstBucket->w, dstBucket->h, dstrect->x, dstrect->y,
            dstrect->w, dstrect->h);
#endif

    memset(&info, 0, sizeof(g2d_blt_h));

    if (srcrect->w == 0 || dstrect->w == 0 || !srcBucket || !dstBucket) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_HWAccelBlit parameter error\n");
        return -1;
    }

#ifdef _MGIMAGE_G2D_ROTATE
    info.flag_h = data->rotate;
    info.src_image_h.width = data->rotateBucket.w;
    info.src_image_h.height = data->rotateBucket.h;
    info.src_image_h.laddr[0] = data->rotateBucket.addrPhy;
#else
    info.flag_h = 1024;
    info.src_image_h.width = srcBucket->w;
    info.src_image_h.height = srcBucket->h;
    info.src_image_h.laddr[0] = srcBucket->addrPhy;
#endif

    switch (src->format->BitsPerPixel) {
    case 32:
        info.src_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.src_image_h.format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.src_image_h.format = G2D_FORMAT_RGB565;
        break;
    default:
        info.src_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.src_image_h.clip_rect.x = srcrect->x;
    info.src_image_h.clip_rect.y = srcrect->y;
    info.src_image_h.clip_rect.w = srcrect->w;
    info.src_image_h.clip_rect.h = srcrect->h;

    info.src_image_h.mode = G2D_GLOBAL_ALPHA;
    info.src_image_h.alpha = 255;
    info.src_image_h.color = 0xee8899;
    info.src_image_h.align[0] = 0;
    info.src_image_h.align[1] = info.src_image_h.align[0];
    info.src_image_h.align[2] = info.src_image_h.align[0];
    info.src_image_h.laddr[1] = (unsigned long) 0;
    info.src_image_h.laddr[2] = (unsigned long) 0;
    info.src_image_h.use_phy_addr = 1;

    switch (dst->format->BitsPerPixel) {
    case 32:
        info.dst_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.dst_image_h.format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.dst_image_h.format = G2D_FORMAT_RGB565;
        break;
    default:
        info.dst_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.dst_image_h.width = dstBucket->w;
    info.dst_image_h.height = dstBucket->h;

    info.dst_image_h.clip_rect.x = dstrect->x;
    info.dst_image_h.clip_rect.y = dstrect->y;
    info.dst_image_h.clip_rect.w = dstrect->w;
    info.dst_image_h.clip_rect.h = dstrect->h;

    info.dst_image_h.mode = G2D_GLOBAL_ALPHA;
    info.dst_image_h.alpha = 255;
    info.dst_image_h.color = 0xee8899;
    info.dst_image_h.align[0] = 0;
    info.dst_image_h.align[1] = info.dst_image_h.align[0];
    info.dst_image_h.align[2] = info.dst_image_h.align[0];
    info.dst_image_h.laddr[0] = dstBucket->addrPhy;
    info.dst_image_h.laddr[1] = (unsigned long) 0;
    info.dst_image_h.laddr[2] = (unsigned long) 0;
    info.dst_image_h.use_phy_addr = 1;

    if (ioctl(data->g2dFd, G2D_CMD_BITBLT_H, (unsigned long) (&info)) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: G2D_CMD_BITBLT_H failed\n");

        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBlit "
                        "src_addr=%p src.format=%d src_wh=[%d %d] src_clip=[%d %d %d %d] "
                        "dst_addr=%p dst.format=%d dst_wh=[%d %d] dst_clip=[%d %d %d %d]\n",
                info.src_image_h.laddr[0], info.src_image_h.format,
                info.src_image_h.width, info.src_image_h.height,
                info.src_image_h.clip_rect.x, info.src_image_h.clip_rect.y,
                info.src_image_h.clip_rect.w, info.src_image_h.clip_rect.h,
                info.dst_image_h.laddr[0], info.dst_image_h.format,
                info.dst_image_h.width, info.dst_image_h.height,
                info.dst_image_h.clip_rect.x, info.dst_image_h.clip_rect.y,
                info.dst_image_h.clip_rect.w, info.dst_image_h.clip_rect.h);
        return -1;
    }
    return 0;
}

static int SUNXIFB_HWAccelBld(_THIS, GAL_Surface *src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *srcBucket = (vidmem_bucket*) src->hwdata;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    g2d_bld info;

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBld srcSddrPhy=%p src=[%d, %d, %d, %d] srcrect=[%d, %d, %d, %d] "
                    "dstAddrPhy=%p dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            srcBucket->addrPhy, src->w, src->h, srcBucket->w, srcBucket->h,
            srcrect->x, srcrect->y, srcrect->w, srcrect->h, dstBucket->addrPhy,
            dst->w, dst->h, dstBucket->w, dstBucket->h, dstrect->x, dstrect->y,
            dstrect->w, dstrect->h);
#endif

    memset(&info, 0, sizeof(g2d_bld));

    if (srcrect->w == 0 || dstrect->w == 0 || !srcBucket || !dstBucket) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_HWAccelBld parameter error\n");
        return -1;
    }

    pthread_mutex_lock(&this->hidden->updateLock);

    SunxiMemFlushCache(data->pMemops, src->pixels, src->h * src->pitch);
    SunxiMemFlushCache(data->pMemops, dst->pixels, dst->h * dst->pitch);

    info.src_image[1].clip_rect.y = srcrect->y;
    info.dst_image.clip_rect.y = dstrect->y;

    info.src_image[1].width = srcBucket->w;
    info.src_image[1].height = srcBucket->h;
    info.src_image[1].laddr[0] = srcBucket->addrPhy;

    info.dst_image.width = dstBucket->w;
    info.dst_image.height = dstBucket->h;
    info.dst_image.laddr[0] = dstBucket->addrPhy;

#ifdef _MGIMAGE_G2D_ROTATE
    if (srcBucket->isScreen) {
        info.src_image[1].width = data->rotateBucket.w;
        info.src_image[1].height = data->rotateBucket.h;
        info.src_image[1].laddr[0] = data->rotateBucket.addrPhy;
    }
    if (dstBucket->isScreen) {
        info.dst_image.width = data->rotateBucket.w;
        info.dst_image.height = data->rotateBucket.h;
        info.dst_image.laddr[0] = data->rotateBucket.addrPhy;
    }
#else
    if (srcBucket->isScreen)
        info.src_image[1].clip_rect.y = data->flipPage * src->h + srcrect->y;
    if (dstBucket->isScreen)
        info.dst_image.clip_rect.y = data->flipPage * dst->h + dstrect->y;
#endif

    switch (src->format->BitsPerPixel) {
    case 32:
        info.src_image[1].format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.src_image[1].format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.src_image[1].format = G2D_FORMAT_RGB565;
        break;
    default:
        info.src_image[1].format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.src_image[1].clip_rect.x = srcrect->x;
    info.src_image[1].clip_rect.w = srcrect->w;
    info.src_image[1].clip_rect.h = srcrect->h;

    if (src->flags == 257) {
        /* SetMemDCAlpha(hdc, MEMDC_FLAG_NONE, 0) */
        /* Solve the problem that the UI cannot be cleared when the video is in the background */
        info.bld_cmd = G2D_BLD_COPY;
    } else if (src->flags & GAL_SRCCOLORKEY) {
        info.bld_cmd = G2D_CK_DST;
        info.ck_para.match_rule = 0;
        info.ck_para.max_color = src->format->colorkey;
        info.ck_para.min_color = src->format->colorkey;
    } else {
        info.bld_cmd = G2D_BLD_SRCOVER;
    }

    if (src->flags & GAL_SRCALPHA) {
        info.src_image[1].mode = G2D_GLOBAL_ALPHA;
        info.src_image[1].alpha = src->format->alpha;
    } else {
        info.src_image[1].mode = G2D_PIXEL_ALPHA;
        info.src_image[1].alpha = 255;
    }
    info.src_image[1].color = 0xee8899;
    info.src_image[1].align[0] = 0;
    info.src_image[1].align[1] = info.src_image[0].align[0];
    info.src_image[1].align[2] = info.src_image[0].align[0];
    info.src_image[1].laddr[1] = (unsigned long) 0;
    info.src_image[1].laddr[2] = (unsigned long) 0;
    info.src_image[1].use_phy_addr = 1;

    switch (dst->format->BitsPerPixel) {
    case 32:
        info.dst_image.format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.dst_image.format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.dst_image.format = G2D_FORMAT_RGB565;
        break;
    default:
        info.dst_image.format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.dst_image.clip_rect.x = dstrect->x;
    info.dst_image.clip_rect.w = dstrect->w;
    info.dst_image.clip_rect.h = dstrect->h;

    info.dst_image.mode = G2D_PIXEL_ALPHA;
    info.dst_image.alpha = 255;
    info.dst_image.color = 0xee8899;
    info.dst_image.align[0] = 0;
    info.dst_image.align[1] = info.dst_image.align[0];
    info.dst_image.align[2] = info.dst_image.align[0];
    info.dst_image.laddr[1] = (unsigned long) 0;
    info.dst_image.laddr[2] = (unsigned long) 0;
    info.dst_image.use_phy_addr = 1;

    /* src_image[1] is the top, src_image[0] is the bottom */
    /* src_image[0] is used as dst_image, no need to malloc a buffer */
    info.src_image[0] = info.dst_image;

    if (ioctl(data->g2dFd, G2D_CMD_BLD_H, (unsigned long) (&info)) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: G2D_CMD_BLD_H failed\n");
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBld "
                        "src_addr=%p src.format=%d src_wh=[%d %d] src_clip=[%d %d %d %d] "
                        "dst_addr=%p dst.format=%d dst_wh=[%d %d] dst_clip=[%d %d %d %d]\n",
                info.src_image[1].laddr[0], info.src_image[1].format,
                info.src_image[1].width, info.src_image[1].height,
                info.src_image[1].clip_rect.x, info.src_image[1].clip_rect.y,
                info.src_image[1].clip_rect.w, info.src_image[1].clip_rect.h,
                info.dst_image.laddr[0], info.dst_image.format,
                info.dst_image.width, info.dst_image.height,
                info.dst_image.clip_rect.x, info.dst_image.clip_rect.y,
                info.dst_image.clip_rect.w, info.dst_image.clip_rect.h);

        pthread_mutex_unlock(&this->hidden->updateLock);
        return -1;
    }

    pthread_mutex_unlock(&this->hidden->updateLock);
    return 0;
}

static int SUNXIFB_HWPutBoxAlpha(_THIS, PBITMAP src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    g2d_bld info;

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWBoxAlpha srcSddrPhy=%p src=[%d, %d] srcrect=[%d, %d, %d, %d] "
                    "dstAddrPhy=%p dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            src->bmHwInfo.bmBitsPhy, src->bmWidth, src->bmHeight, srcrect->x,
            srcrect->y, srcrect->w, srcrect->h, dstBucket->addrPhy, dst->w,
            dst->h, dstBucket->w, dstBucket->h, dstrect->x, dstrect->y,
            dstrect->w, dstrect->h);
#endif

    memset(&info, 0, sizeof(g2d_bld));

    if (srcrect->w == 0 || dstrect->w == 0 || !src || !dstBucket) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_HWBoxAlpha parameter error\n");
        return -1;
    }

    pthread_mutex_lock(&this->hidden->updateLock);

    SunxiMemFlushCache(data->pMemops, src->bmBits,
            src->bmHeight * src->bmPitch);
    SunxiMemFlushCache(data->pMemops, dst->pixels, dst->h * dst->pitch);

    info.src_image[1].clip_rect.y = srcrect->y;
    info.dst_image.clip_rect.y = dstrect->y;

    info.dst_image.width = dstBucket->w;
    info.dst_image.height = dstBucket->h;
    info.dst_image.laddr[0] = dstBucket->addrPhy;

#ifdef _MGIMAGE_G2D_ROTATE
    if (dstBucket->isScreen) {
        info.dst_image.width = data->rotateBucket.w;
        info.dst_image.height = data->rotateBucket.h;
        info.dst_image.laddr[0] = data->rotateBucket.addrPhy;
    }
#else
    if (dstBucket->isScreen)
        info.dst_image.clip_rect.y = data->flipPage * dst->h + dstrect->y;
#endif

    switch (src->bmBitsPerPixel) {
    case 32:
        info.src_image[1].format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.src_image[1].format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.src_image[1].format = G2D_FORMAT_RGB565;
        break;
    default:
        info.src_image[1].format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.src_image[1].width = src->bmWidth;
    info.src_image[1].height = src->bmHeight;

    info.src_image[1].clip_rect.x = srcrect->x;
    info.src_image[1].clip_rect.w = srcrect->w;
    info.src_image[1].clip_rect.h = srcrect->h;

    if (src->bmType & BMP_TYPE_ALPHA)
        info.bld_cmd = G2D_BLD_SRCOVER;
    else
        info.bld_cmd = G2D_BLD_COPY;

    info.src_image[1].mode = G2D_PIXEL_ALPHA;
    info.src_image[1].alpha = 255;
    info.src_image[1].color = 0xee8899;
    info.src_image[1].align[0] = 0;
    info.src_image[1].align[1] = info.src_image[0].align[0];
    info.src_image[1].align[2] = info.src_image[0].align[0];
    info.src_image[1].laddr[0] = src->bmHwInfo.bmBitsPhy;
    info.src_image[1].laddr[1] = (unsigned long) 0;
    info.src_image[1].laddr[2] = (unsigned long) 0;
    info.src_image[1].use_phy_addr = 1;

    switch (dst->format->BitsPerPixel) {
    case 32:
        info.dst_image.format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.dst_image.format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.dst_image.format = G2D_FORMAT_RGB565;
        break;
    default:
        info.dst_image.format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.dst_image.clip_rect.x = dstrect->x;
    info.dst_image.clip_rect.w = dstrect->w;
    info.dst_image.clip_rect.h = dstrect->h;

    info.dst_image.mode = G2D_PIXEL_ALPHA;
    info.dst_image.alpha = 255;
    info.dst_image.color = 0xee8899;
    info.dst_image.align[0] = 0;
    info.dst_image.align[1] = info.dst_image.align[0];
    info.dst_image.align[2] = info.dst_image.align[0];
    info.dst_image.laddr[1] = (unsigned long) 0;
    info.dst_image.laddr[2] = (unsigned long) 0;
    info.dst_image.use_phy_addr = 1;

    /* src_image[1] is the top, src_image[0] is the bottom */
    /* src_image[0] is used as dst_image, no need to malloc a buffer */
    info.src_image[0] = info.dst_image;

    if (ioctl(data->g2dFd, G2D_CMD_BLD_H, (unsigned long) (&info)) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: G2D_CMD_BLD_H failed\n");
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWBoxAlpha "
                        "src_addr=%p src.format=%d src_wh=[%d %d] src_clip=[%d %d %d %d] "
                        "dst_addr=%p dst.format=%d dst_wh=[%d %d] dst_clip=[%d %d %d %d]\n",
                info.src_image[1].laddr[0], info.src_image[1].format,
                info.src_image[1].width, info.src_image[1].height,
                info.src_image[1].clip_rect.x, info.src_image[1].clip_rect.y,
                info.src_image[1].clip_rect.w, info.src_image[1].clip_rect.h,
                info.dst_image.laddr[0], info.dst_image.format,
                info.dst_image.width, info.dst_image.height,
                info.dst_image.clip_rect.x, info.dst_image.clip_rect.y,
                info.dst_image.clip_rect.w, info.dst_image.clip_rect.h);

        pthread_mutex_unlock(&this->hidden->updateLock);
        return -1;
    }

    pthread_mutex_unlock(&this->hidden->updateLock);
    return 0;
}

static int SUNXIFB_FillHWRect(_THIS, GAL_Surface *dst, GAL_Rect *rect,
        Uint32 color) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    g2d_fillrect_h info;

#ifdef SUNXIFB_DEBUG
    GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_FillHWRect dstAddrPhy=%p "
            "dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d] color=%u\n",
            dstBucket->addrPhy, dst->w, dst->h, dstBucket->w, dstBucket->h,
            rect->x, rect->y, rect->w, rect->h, color);
#endif

    memset(&info, 0, sizeof(g2d_fillrect_h));

    if (rect->w == 0 || rect->h == 0 || !dstBucket) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_FillHWRect parameter error\n");
        return -1;
    }

    pthread_mutex_lock(&this->hidden->updateLock);

    SunxiMemFlushCache(data->pMemops, dst->pixels, dst->h * dst->pitch);

    switch (dst->format->BitsPerPixel) {
    case 32:
        info.dst_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.dst_image_h.format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.dst_image_h.format = G2D_FORMAT_RGB565;
        break;
    default:
        info.dst_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.dst_image_h.clip_rect.y = rect->y;

    info.dst_image_h.width = dstBucket->w;
    info.dst_image_h.height = dstBucket->h;
    info.dst_image_h.laddr[0] = dstBucket->addrPhy;

#ifdef _MGIMAGE_G2D_ROTATE
    if (dstBucket->isScreen) {
        info.dst_image_h.width = data->rotateBucket.w;
        info.dst_image_h.height = data->rotateBucket.h;
        info.dst_image_h.laddr[0] = data->rotateBucket.addrPhy;
    }
#else
    if (dstBucket->isScreen)
        info.dst_image_h.clip_rect.y = data->flipPage * dst->h + rect->y;
#endif

    info.dst_image_h.mode = G2D_PIXEL_ALPHA;
    info.dst_image_h.alpha = 255;
    info.dst_image_h.color = color;
    info.dst_image_h.clip_rect.x = rect->x;
    info.dst_image_h.clip_rect.w = rect->w;
    info.dst_image_h.clip_rect.h = rect->h;
    info.dst_image_h.align[0] = 0;
    info.dst_image_h.align[1] = info.dst_image_h.align[0];
    info.dst_image_h.align[2] = info.dst_image_h.align[0];
    info.dst_image_h.laddr[1] = (unsigned long) 0;
    info.dst_image_h.laddr[2] = (unsigned long) 0;
    info.dst_image_h.use_phy_addr = 1;

    if (ioctl(data->g2dFd, G2D_CMD_FILLRECT_H, (unsigned long) (&info)) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: G2D_CMD_FILLRECT_H failed\n");
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_FillHWRect "
                        "color=%u dst_addr=%p dst.format=%d dst_wh=[%d %d] dst_clip=[%d %d %d %d]\n",
                color, info.dst_image_h.laddr[0], info.dst_image_h.format,
                info.dst_image_h.width, info.dst_image_h.height,
                info.dst_image_h.clip_rect.x, info.dst_image_h.clip_rect.y,
                info.dst_image_h.clip_rect.w, info.dst_image_h.clip_rect.h);

        pthread_mutex_unlock(&this->hidden->updateLock);
        return -1;
    }

    pthread_mutex_unlock(&this->hidden->updateLock);

    return 0;
}

static int SUNXIFB_SetHWAlpha(_THIS, GAL_Surface *surface, Uint8 alpha) {
    surface->format->alpha = alpha;
    surface->flags |= GAL_SRCALPHA;
    return 0;
}

/*
 * @srcrect The rect of the src image to be cropped
 * @dstrect The rect of the dst image that needs to be rendered
 * @devrect The rect of the border
 * @scalrect The rect that the image needs to use after scaler
 */
static int SUNXIFB_HWPutBoxAlphaScaler(_THIS, PBITMAP src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect, GAL_Rect *devrect,
        GAL_Rect *scalrect) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWAlphaScaler srcSddrPhy=%p src=[%d, %d] srcrect=[%d, %d, %d, %d] "
                    "dstAddrPhy=%p dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d] devrect=[%d, %d, %d, %d] "
                    "scalrect=[%d, %d, %d, %d]\n", src->bmHwInfo.bmBitsPhy,
            src->bmWidth, src->bmHeight, srcrect->x, srcrect->y, srcrect->w,
            srcrect->h, dstBucket->addrPhy, dst->w, dst->h, dstBucket->w,
            dstBucket->h, dstrect->x, dstrect->y, dstrect->w, dstrect->h,
            devrect->x, devrect->y, devrect->w, devrect->h, scalrect->x,
            scalrect->y, scalrect->w, scalrect->h);
#endif

    g2d_blt_h info;
    BITMAP tmpBmp;
    BYTE *scalerBmBits;
    unsigned long scalerBmBitsPhy;
    Uint32 scalerBmPitch;
    int srcx = 0, srcy = 0, dstx = 0, dsty = 0, dstw = 0, dsth = 0;

    memset(&info, 0, sizeof(g2d_blt_h));

    if (srcrect->w == 0 || dstrect->w == 0 || !src || !dstBucket) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_HWAlphaScaler parameter error\n");
        return -1;
    }

    pthread_mutex_lock(&this->hidden->updateLock);

    /* StretchBlt function in NULL*/
    if (!src->bmHwInfo.bmBitsPhy)
        src->bmHwInfo.bmBitsPhy = (unsigned long) SunxiMemGetPhysicAddressCpu(
                this->hidden->pMemops, src->bmBits);

    SunxiMemFlushCache(data->pMemops, src->bmBits,
            src->bmHeight * src->bmPitch);
    SunxiMemFlushCache(data->pMemops, dst->pixels, dst->h * dst->pitch);

    scalerBmPitch = scalrect->w * src->bmBytesPerPixel;
    scalerBmPitch = (scalerBmPitch + 3) & ~3; /* 4-byte aligning */

    scalerBmBits = (BYTE*) SunxiMemPalloc(this->hidden->pMemops, scalrect->h * scalerBmPitch);
    if (0 == scalerBmBits) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWAlphaScaler Failed to apply for scaler buffer\n");
        pthread_mutex_lock(&this->hidden->updateLock);
        return (-1);
    }

    scalerBmBitsPhy = (unsigned long) SunxiMemGetPhysicAddressCpu(
            this->hidden->pMemops, scalerBmBits);

    switch (src->bmBitsPerPixel) {
    case 32:
        info.src_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.src_image_h.format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.src_image_h.format = G2D_FORMAT_RGB565;
        break;
    default:
        info.src_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.flag_h = G2D_BLT_NONE_H;

    info.src_image_h.width = src->bmWidth;
    info.src_image_h.height = src->bmHeight;
    info.src_image_h.clip_rect.x = srcrect->x;
    info.src_image_h.clip_rect.y = srcrect->y;
    info.src_image_h.clip_rect.w = srcrect->w;
    info.src_image_h.clip_rect.h = srcrect->h;
    info.src_image_h.mode = G2D_PIXEL_ALPHA;
    info.src_image_h.alpha = 255;
    info.src_image_h.color = 0xee8899;
    info.src_image_h.align[0] = 0;
    info.src_image_h.align[1] = info.src_image_h.align[0];
    info.src_image_h.align[2] = info.src_image_h.align[0];
    info.src_image_h.laddr[0] = src->bmHwInfo.bmBitsPhy;
    info.src_image_h.laddr[1] = (unsigned long) 0;
    info.src_image_h.laddr[2] = (unsigned long) 0;
    info.src_image_h.use_phy_addr = 1;

    switch (src->bmBitsPerPixel) {
    case 32:
        info.dst_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    case 24:
        info.dst_image_h.format = G2D_FORMAT_RGB888;
        break;
    case 16:
        info.dst_image_h.format = G2D_FORMAT_RGB565;
        break;
    default:
        info.dst_image_h.format = G2D_FORMAT_ARGB8888;
        break;
    }

    info.dst_image_h.width = scalrect->w;
    info.dst_image_h.height = scalrect->h;
    info.dst_image_h.clip_rect.x = 0;
    info.dst_image_h.clip_rect.y = 0;
    info.dst_image_h.clip_rect.w = scalrect->w;
    info.dst_image_h.clip_rect.h = scalrect->h;
    info.dst_image_h.mode = G2D_PIXEL_ALPHA;
    info.dst_image_h.alpha = 255;
    info.dst_image_h.color = 0xee8899;
    info.dst_image_h.align[0] = 0;
    info.dst_image_h.align[1] = info.dst_image_h.align[0];
    info.dst_image_h.align[2] = info.dst_image_h.align[0];
    info.dst_image_h.laddr[0] = scalerBmBitsPhy;
    info.dst_image_h.laddr[1] = (unsigned long) 0;
    info.dst_image_h.laddr[2] = (unsigned long) 0;
    info.dst_image_h.use_phy_addr = 1;

    /* Scale the image to the required size */
    if (ioctl(data->g2dFd, G2D_CMD_BITBLT_H, (unsigned long) (&info)) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: G2D_CMD_BITBLT_H failed\n");

        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWAlphaScaler "
                        "src_addr=%p src.format=%d src_wh=[%d %d] src_clip=[%d %d %d %d] "
                        "dst_addr=%p dst.format=%d dst_wh=[%d %d] dst_clip=[%d %d %d %d]\n",
                info.src_image_h.laddr[0], info.src_image_h.format,
                info.src_image_h.width, info.src_image_h.height,
                info.src_image_h.clip_rect.x, info.src_image_h.clip_rect.y,
                info.src_image_h.clip_rect.w, info.src_image_h.clip_rect.h,
                info.dst_image_h.laddr[0], info.dst_image_h.format,
                info.dst_image_h.width, info.dst_image_h.height,
                info.dst_image_h.clip_rect.x, info.dst_image_h.clip_rect.y,
                info.dst_image_h.clip_rect.w, info.dst_image_h.clip_rect.h);

        SunxiMemPfree(this->hidden->pMemops, (void*) scalerBmBits);
        scalerBmBits = NULL;
        pthread_mutex_unlock(&this->hidden->updateLock);
        return -1;
    }

    /* Build a tmp image */
    memcpy(&tmpBmp, src, sizeof(BITMAP));
    tmpBmp.bmType |= BMP_TYPE_ALPHA;
    tmpBmp.bmBits = scalerBmBits;
    tmpBmp.bmHwInfo.bmBitsPhy = scalerBmBitsPhy;
    tmpBmp.bmWidth = scalrect->w;
    tmpBmp.bmHeight = scalrect->h;
    tmpBmp.bmPitch = scalerBmPitch;

    dstw = dstrect->w;
    dsth = dstrect->h;

    /* E.g dstrect=[96, -44, 618, 618] devrect=[5, 25, 790, 450] scalrect=[0, 0, 618, 618] */
    if (dstrect->x < devrect->x) {
        srcx = -dstrect->x + devrect->x + scalrect->x;
        dstx = devrect->x;
    } else {
        srcx = scalrect->x;
        dstx = dstrect->x;
    }

    if (dstrect->y < devrect->y) {
        srcy = -dstrect->y + devrect->y + scalrect->y;
        dsty = devrect->y;
    } else {
        srcy = scalrect->y;
        dsty = dstrect->y;
    }

    /* If the width exceeds the width of the border */
    /* (dst->w - devrect->w - devrect->x) is the width of the right border */
    if (dstx + dstrect->w > devrect->w)
        dstw = dst->w - dstx - (dst->w - devrect->w - devrect->x);
    /* If the height exceeds the width of the border */
    /* (dst->h - devrect->h - devrect->y) is the height of the bottom border */
    if (dsty + dstrect->h > devrect->h)
        dsth = dst->h - dsty - (dst->h - devrect->h - devrect->y);

    /* Ensure that the final range does not exceed the actual buffer size,
     * especially the width and height of the created window and
     * the rotating buffer may not correspond */
#ifdef _MGIMAGE_G2D_ROTATE
    if (dstx + dstw > data->rotateBucket.w)
        dstw = data->rotateBucket.w - dstx;
    if (dsty + dsth > data->rotateBucket.h)
        dsth = data->rotateBucket.h - dsty;
#else
    if (dstx + dstw > data->w)
        dstw = data->w - dstx;
    if (dsty + dsth > data->h)
        dsth = data->h - dsty;
#endif

    GAL_Rect bmpSrcrect = { srcx, srcy, dstw, dsth };
    GAL_Rect bmpDstrect = { dstx, dsty, dstw, dsth };

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWAlphaScaler srcrect=[%d, %d, %d, %d] "
                    "dstrect=[%d, %d, %d, %d]\n", srcx, srcy, dstw, dsth, dstx,
            dsty, dstw, dsth);
#endif

    pthread_mutex_unlock(&this->hidden->updateLock);

    SUNXIFB_HWPutBoxAlpha(this, &tmpBmp, &bmpSrcrect, dst, &bmpDstrect);

    SunxiMemPfree(this->hidden->pMemops, (void*) scalerBmBits);
    scalerBmBits = NULL;
    return 0;
}

static int SUNXIFB_SetHWColorKey(_THIS, GAL_Surface *src, Uint32 key) {
    src->format->colorkey = key;
    src->flags |= GAL_SRCCOLORKEY;
    return 0;
}
#endif

#if defined(_MGIMAGE_G2D) || defined(_MGIMAGE_GPU)
static int SUNXIFB_AllocHWBuffer(_THIS, PBITMAP bmp, int nSize) {
    bmp->bmBits = (BYTE*) SunxiMemPalloc(this->hidden->pMemops, nSize);
    if (0 == bmp->bmBits)
        return (-1);

#if _MGIMAGE_GPU
    bmp->bmHwInfo.bmFd = SunxiMemGetBufferFd(this->hidden->pMemops,
            bmp->bmBits);
#endif
    bmp->bmHwInfo.bmBitsPhy = (unsigned long) SunxiMemGetPhysicAddressCpu(
            this->hidden->pMemops, bmp->bmBits);

#ifdef SUNXIFB_DEBUG
#if _MGIMAGE_GPU
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_AllocHWBuffer fd=%d ddrPhy=%p ddr=%p\n",
            bmp->bmHwInfo.bmFd, bmp->bmHwInfo.bmBitsPhy, bmp->bmBits);
#else
    GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_AllocHWBuffer ddrPhy=%p ddr=%p\n",
            bmp->bmHwInfo.bmBitsPhy, bmp->bmBits);
#endif
#endif
    return 0;
}

static void SUNXIFB_FreeHWBuffer(_THIS, PBITMAP bmp) {
    if (bmp->bmBits) {
#ifdef SUNXIFB_DEBUG
#if _MGIMAGE_GPU
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_FreeHWBuffer fd=%d ddrPhy=%p ddr=%p\n",
                bmp->bmHwInfo.bmFd, bmp->bmHwInfo.bmBitsPhy, bmp->bmBits);
#else
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_FreeHWBuffer ddrPhy=%p ddr=%p\n",
                bmp->bmHwInfo.bmBitsPhy, bmp->bmBits);
#endif
#endif
        SunxiMemPfree(this->hidden->pMemops, (void*) bmp->bmBits);
        bmp->bmBits = NULL;
    }
}
#endif

#if defined(_MGIMAGE_G2D_ROTATE) || defined(_MGIMAGE_GPU_ROTATE)
static void change_mouseXY_cw(int *x, int *y) {
    int tmp;
    if (*x > (__gal_screen->h - 1))
        *x = (__gal_screen->h - 1);
    if (*y > (__gal_screen->w - 1))
        *y = (__gal_screen->w - 1);
    tmp = *x;
    *x = *y;
    *y = (__gal_screen->h - 1) - tmp;
}

static void change_mouseXY_ccw(int *x, int *y) {
    int tmp;
    if (*x > (__gal_screen->h - 1))
        *x = (__gal_screen->h - 1);
    if (*y > (__gal_screen->w - 1))
        *y = (__gal_screen->w - 1);
    tmp = *y;
    *y = *x;
    *x = (__gal_screen->w - 1) - tmp;
}

static void change_mouseXY_cw180(int *x, int *y) {
    if (*x > (__gal_screen->w - 1))
        *x = (__gal_screen->w - 1);
    if (*y > (__gal_screen->h - 1))
        *y = (__gal_screen->h - 1);
    *x = (__gal_screen->w - 1) - *x;
    *y = (__gal_screen->h - 1) - *y;
}
#endif

#ifdef _MGIMAGE_GPU
static int SUNXIFB_HWAccelBlit(_THIS, GAL_Surface *src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *srcBucket = (vidmem_bucket*) src->hwdata;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    unsigned int dstFormat;
    int dstIndex;
    struct RESurface surface0;
    struct RE_rect crop0 = { srcrect->x, srcrect->y, srcrect->w, srcrect->h };
    struct RE_rect dstWinRect0 = { dstrect->x, dstrect->y, dstrect->w,
            dstrect->h };

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBlit srcFd=%d src=[%d, %d, %d, %d] srcrect=[%d, %d, %d, %d] "
                    "dstFd=%d dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            srcBucket->fbDmaBuf.fd, src->w, src->h, srcBucket->w, srcBucket->h,
            srcrect->x, srcrect->y, srcrect->w, srcrect->h,
            dstBucket->fbDmaBuf.fd, dst->w, dst->h, dstBucket->w, dstBucket->h,
            dstrect->x, dstrect->y, dstrect->w, dstrect->h);
#endif

    memset(&surface0, 0, sizeof(surface0));

    switch (src->format->BitsPerPixel) {
    case 32:
        surface0.srcFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        surface0.srcFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        surface0.srcFormat = DRM_FORMAT_RGB565;
        break;
    default:
        surface0.srcFormat = MY_FORMAT_ARGB8888;
        break;
    }

    switch (dst->format->BitsPerPixel) {
    case 32:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        dstFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        dstFormat = DRM_FORMAT_RGB565;
        break;
    default:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    }

    surface0.globalAlpha = 1.0f;
    dstIndex = !data->flipPage;

#ifdef _MGIMAGE_GPU_ROTATE
    surface0.srcFd = data->rotateBucket.fbDmaBuf.fd;
    surface0.srcDataOffset = 0;
    surface0.srcWidth = data->rotateBucket.w;
    surface0.srcHeight = data->rotateBucket.h;
    surface0.srcRotate = data->rotate;
    dstIndex = data->flipPage;
#else
    surface0.srcFd = srcBucket->fbDmaBuf.fd;
    surface0.srcDataOffset = data->flipPage * data->mappedOneBufLen;
    surface0.srcWidth = src->w;
    surface0.srcHeight = src->h;
    surface0.srcRotate = SRC_ROTATE_DEGREE_00;
#endif

    memcpy(&surface0.srcCrop, &crop0, sizeof(crop0));
    memcpy(&surface0.dstWinRect, &dstWinRect0, sizeof(dstWinRect0));

    renderEngineSetSurface(data->render, &surface0);

    if (renderEngineDrawOffScreen(data->render,
            data->videoBucket.target[dstIndex], 0, 0.0f, NULL) < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_HWAccelBlit drawLayer failed\n");
        return -1;
    }

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBlit srcFd=%d srcOffset=%d src=[%d, %d, %d, %d] "
                    "srcrect=[%d, %d, %d, %d] dstFd=%d dstOffset=%d dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            surface0.srcFd, surface0.srcDataOffset, surface0.srcWidth,
            surface0.srcHeight, srcBucket->w, srcBucket->h,
            surface0.srcCrop.left, surface0.srcCrop.top, surface0.srcCrop.right,
            surface0.srcCrop.bottom, dstBucket->fbDmaBuf.fd,
            dstBucket->isScreen ? data->flipPage * data->mappedOneBufLen : 0,
            dst->w, dst->h, dstBucket->w, dstBucket->h,
            surface0.dstWinRect.left, surface0.dstWinRect.top,
            surface0.dstWinRect.right, surface0.dstWinRect.bottom);
#endif

    return 0;
}

static int SUNXIFB_HWAccelBld(_THIS, GAL_Surface *src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *srcBucket = (vidmem_bucket*) src->hwdata;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    unsigned int dstFormat;
    unsigned char blendEnable;
    unsigned long long target;
    struct RESurface surface0;
    struct RE_rect crop0 = { srcrect->x, srcrect->y, srcrect->x + srcrect->w,
            srcrect->y + srcrect->h };
    struct RE_rect dstWinRect0 = { dstrect->x, dstrect->y, dstrect->x
            + dstrect->w, dstrect->y + dstrect->h };

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBld srcFd=%d src=[%d, %d, %d, %d] srcrect=[%d, %d, %d, %d] "
                    "dstFd=%d dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            srcBucket->fbDmaBuf.fd, src->w, src->h, srcBucket->w, srcBucket->h,
            srcrect->x, srcrect->y, srcrect->w, srcrect->h,
            dstBucket->fbDmaBuf.fd, dst->w, dst->h, dstBucket->w, dstBucket->h,
            dstrect->x, dstrect->y, dstrect->w, dstrect->h);
#endif

    if (data->gpuThreadId != pthread_self()) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBld cannot be called outside gpuThreadId\n");
        return -1;
    }

    if (srcrect->w == 0 || dstrect->w == 0 || !srcBucket || !dstBucket) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_HWAccelBld parameter error\n");
        return -1;
    }

    memset(&surface0, 0, sizeof(surface0));

    pthread_mutex_lock(&this->hidden->updateLock);

    SunxiMemFlushCache(data->pMemops, src->pixels, src->h * src->pitch);
    SunxiMemFlushCache(data->pMemops, dst->pixels, dst->h * dst->pitch);

    switch (src->format->BitsPerPixel) {
    case 32:
        surface0.srcFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        surface0.srcFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        surface0.srcFormat = DRM_FORMAT_RGB565;
        break;
    default:
        surface0.srcFormat = MY_FORMAT_ARGB8888;
        break;
    }

    switch (dst->format->BitsPerPixel) {
    case 32:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        dstFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        dstFormat = DRM_FORMAT_RGB565;
        break;
    default:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    }

    blendEnable = 1;
    /* SetMemDCAlpha(hdc, MEMDC_FLAG_NONE, 0) */
    /* Solve the problem that the UI cannot be cleared when the video is in the background */
    if (src->flags == 257)
        blendEnable = 0;
    else if (src->flags & GAL_SRCCOLORKEY) {
        unsigned rv, gv, bv;
        surface0.colorkey.enable = true;
        rv = (src->format->colorkey & src->format->Rmask)
                >> src->format->Rshift;
        surface0.colorkey.r = (rv << src->format->Rloss)
                + (rv >> (8 - src->format->Rloss));
        gv = (src->format->colorkey & src->format->Gmask)
                >> src->format->Gshift;
        surface0.colorkey.g = (gv << src->format->Gloss)
                + (gv >> (8 - src->format->Gloss));
        bv = (src->format->colorkey & src->format->Bmask)
                >> src->format->Bshift;
        surface0.colorkey.b = (bv << src->format->Bloss)
                + (bv >> (8 - src->format->Bloss));
    }

    if (src->flags & GAL_SRCALPHA) {
        surface0.globalAlpha = (float) src->format->alpha / 255.0f;
    } else {
        surface0.globalAlpha = 1.0f;
    }

    surface0.srcFd = srcBucket->fbDmaBuf.fd;
    surface0.srcDataOffset =
            srcBucket->isScreen ? data->flipPage * data->mappedOneBufLen : 0;
    surface0.srcWidth = src->w;
    surface0.srcHeight = src->h;
    surface0.srcRotate = SRC_ROTATE_DEGREE_00;

#ifdef _MGIMAGE_GPU_ROTATE
    if (srcBucket->isScreen) {
        surface0.srcFd = data->rotateBucket.fbDmaBuf.fd;
        surface0.srcDataOffset = 0;
        surface0.srcWidth = data->rotateBucket.w;
        surface0.srcHeight = data->rotateBucket.h;
        surface0.srcRotate = SRC_ROTATE_DEGREE_00;
    }
#endif

    memcpy(&surface0.srcCrop, &crop0, sizeof(crop0));
    memcpy(&surface0.dstWinRect, &dstWinRect0, sizeof(dstWinRect0));

    renderEngineSetSurface(data->render, &surface0);

#ifdef _MGIMAGE_GPU_ROTATE
    if (dstBucket->isScreen)
        target = data->rotateBucket.target[0];
    else
        target = dstBucket->target[0];
#else
    target = dstBucket->target[dstBucket->isScreen ? data->flipPage : 0];
#endif

    if (target == NULL) {
        dstBucket->target[0] = renderEngineCreateOffScreenTarget(data->render,
                dstBucket->w, dstBucket->h, dstFormat, dstBucket->fbDmaBuf.fd,
                0);
        target = dstBucket->target[0];
    }

    if (renderEngineDrawOffScreen(data->render, target, blendEnable, 0.0f, NULL)
            < 0) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_HWAccelBld drawLayer failed\n");
        pthread_mutex_unlock(&this->hidden->updateLock);
        return -1;
    }

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWAccelBld srcFd=%d srcOffset=%d src=[%d, %d, %d, %d] "
                    "srcrect=[%d, %d, %d, %d] dstFd=%d dstOffset=%d dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            surface0.srcFd, surface0.srcDataOffset, surface0.srcWidth,
            surface0.srcHeight, srcBucket->w, srcBucket->h,
            surface0.srcCrop.left, surface0.srcCrop.top, surface0.srcCrop.right,
            surface0.srcCrop.bottom, dstBucket->fbDmaBuf.fd,
            dstBucket->isScreen ? data->flipPage * data->mappedOneBufLen : 0,
            dst->w, dst->h, dstBucket->w, dstBucket->h,
            surface0.dstWinRect.left, surface0.dstWinRect.top,
            surface0.dstWinRect.right, surface0.dstWinRect.bottom);
#endif

    pthread_mutex_unlock(&this->hidden->updateLock);
    return 0;
}

static int SUNXIFB_HWPutBoxAlpha(_THIS, PBITMAP src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    unsigned int dstFormat;
    unsigned char blendEnable;
    unsigned long long target;
    struct RESurface surface0;
    struct RE_rect crop0 = { srcrect->x, srcrect->y, srcrect->x + srcrect->w,
            srcrect->y + srcrect->h };
    struct RE_rect dstWinRect0 = { dstrect->x, dstrect->y, dstrect->x
            + dstrect->w, dstrect->y + dstrect->h };

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlpha srcFd=%d src=[%d, %d] srcrect=[%d, %d, %d, %d] "
                    "dstFd=%d dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            src->bmHwInfo.bmFd, src->bmWidth, src->bmHeight, srcrect->x,
            srcrect->y, srcrect->w, srcrect->h, dstBucket->fbDmaBuf.fd, dst->w,
            dst->h, dstBucket->w, dstBucket->h, dstrect->x, dstrect->y,
            dstrect->w, dstrect->h);
#endif

    if (data->gpuThreadId != pthread_self()) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlpha cannot be called outside gpuThreadId\n");
        return -1;
    }

    if (srcrect->w == 0 || dstrect->w == 0 || !src || !dstBucket) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlpha parameter error\n");
        return -1;
    }

    memset(&surface0, 0, sizeof(surface0));

    pthread_mutex_lock(&this->hidden->updateLock);

    SunxiMemFlushCache(data->pMemops, src->bmBits,
            src->bmHeight * src->bmPitch);
    SunxiMemFlushCache(data->pMemops, dst->pixels, dst->h * dst->pitch);

    switch (src->bmBitsPerPixel) {
    case 32:
        surface0.srcFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        surface0.srcFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        surface0.srcFormat = DRM_FORMAT_RGB565;
        break;
    default:
        surface0.srcFormat = MY_FORMAT_ARGB8888;
        break;
    }

    switch (dst->format->BitsPerPixel) {
    case 32:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        dstFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        dstFormat = DRM_FORMAT_RGB565;
        break;
    default:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    }

    if (src->bmType & BMP_TYPE_ALPHA)
        blendEnable = 1;
    else
        blendEnable = 0;

    surface0.srcFd = src->bmHwInfo.bmFd;
    surface0.srcDataOffset = 0;
    surface0.srcWidth = src->bmWidth;
    surface0.srcHeight = src->bmHeight;
    surface0.globalAlpha = 1.0f;
    memcpy(&surface0.srcCrop, &crop0, sizeof(crop0));
    memcpy(&surface0.dstWinRect, &dstWinRect0, sizeof(dstWinRect0));

    renderEngineSetSurface(data->render, &surface0);

#ifdef _MGIMAGE_GPU_ROTATE
    if (dstBucket->isScreen)
        target = data->rotateBucket.target[0];
    else
        target = dstBucket->target[0];
#else
    target = dstBucket->target[dstBucket->isScreen ? data->flipPage : 0];
#endif

    if (target == NULL) {
        dstBucket->target[0] = renderEngineCreateOffScreenTarget(data->render,
                dstBucket->w, dstBucket->h, dstFormat, dstBucket->fbDmaBuf.fd,
                0);
        target = dstBucket->target[0];
    }

    if (renderEngineDrawOffScreen(data->render, target, blendEnable, 0.0f, NULL)
            < 0) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlpha drawLayer failed\n");
        pthread_mutex_unlock(&this->hidden->updateLock);
        return -1;
    }

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlpha srcFd=%d srcOffset=%d src=[%d, %d, %d, %d] "
                    "srcrect=[%d, %d, %d, %d] dstFd=%d dstOffset=%d dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            surface0.srcFd, surface0.srcDataOffset, surface0.srcWidth,
            surface0.srcHeight, src->bmWidth, src->bmHeight,
            surface0.srcCrop.left, surface0.srcCrop.top, surface0.srcCrop.right,
            surface0.srcCrop.bottom, dstBucket->fbDmaBuf.fd,
            dstBucket->isScreen ? data->flipPage * data->mappedOneBufLen : 0,
            dst->w, dst->h, dstBucket->w, dstBucket->h,
            surface0.dstWinRect.left, surface0.dstWinRect.top,
            surface0.dstWinRect.right, surface0.dstWinRect.bottom);
#endif

    pthread_mutex_unlock(&this->hidden->updateLock);
    return 0;
}

/*
 * @srcrect The rect of the src image to be cropped
 * @dstrect The rect of the dst image that needs to be rendered
 * @devrect The rect of the border
 * @scalrect The rect that the image needs to use after scaler
 */
static int SUNXIFB_HWPutBoxAlphaScaler(_THIS, PBITMAP src, GAL_Rect *srcrect,
        GAL_Surface *dst, GAL_Rect *dstrect, GAL_Rect *devrect,
        GAL_Rect *scalrect) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    unsigned int dstFormat;
    unsigned long long target;
    struct RESurface surface0;
    BYTE *scalerBmBits;
    int scalerBmFd;
    Uint32 scalerBmPitch;
    BITMAP tmpBmp;
    struct RE_rect crop0 = { srcrect->x, srcrect->y, srcrect->x + srcrect->w,
            srcrect->y + srcrect->h };
    struct RE_rect dstWinRect0 = { 0, 0, scalrect->w, scalrect->h };
    int srcx = 0, srcy = 0, dstx = 0, dsty = 0, dstw = 0, dsth = 0;

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlphaScaler srcSddrPhy=%p src=[%d, %d] srcrect=[%d, %d, %d, %d] "
                    "dstAddrPhy=%p dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d] devrect=[%d, %d, %d, %d] "
                    "scalrect=[%d, %d, %d, %d]\n", src->bmHwInfo.bmBitsPhy,
            src->bmWidth, src->bmHeight, srcrect->x, srcrect->y, srcrect->w,
            srcrect->h, dstBucket->addrPhy, dst->w, dst->h, dstBucket->w,
            dstBucket->h, dstrect->x, dstrect->y, dstrect->w, dstrect->h,
            devrect->x, devrect->y, devrect->w, devrect->h, scalrect->x,
            scalrect->y, scalrect->w, scalrect->h);
#endif

    if (data->gpuThreadId != pthread_self()) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlphaScaler cannot be called outside gpuThreadId\n");
        return -1;
    }

    if (srcrect->w == 0 || dstrect->w == 0 || !src || !dstBucket) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlphaScaler parameter error\n");
        return -1;
    }

    memset(&surface0, 0, sizeof(surface0));

    pthread_mutex_lock(&this->hidden->updateLock);

    /* StretchBlt function in NULL*/
    if (!src->bmHwInfo.bmBitsPhy)
        src->bmHwInfo.bmFd = SunxiMemGetBufferFd(data->pMemops, src->bmBits);

    SunxiMemFlushCache(data->pMemops, src->bmBits,
            src->bmHeight * src->bmPitch);
    SunxiMemFlushCache(data->pMemops, dst->pixels, dst->h * dst->pitch);

    scalerBmPitch = ((scalrect->w + 15) & ~15) * src->bmBytesPerPixel; /* 16-byte aligning */
    scalerBmPitch = (scalerBmPitch + 3) & ~3; /* 4-byte aligning */

    scalerBmBits = (BYTE*) SunxiMemPalloc(data->pMemops, scalrect->h * scalerBmPitch);
    if (0 == scalerBmBits) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlphaScaler Failed to apply for scaler buffer\n");
        pthread_mutex_lock(&this->hidden->updateLock);
        return (-1);
    }

    scalerBmFd = SunxiMemGetBufferFd(data->pMemops, scalerBmBits);

    switch (src->bmBitsPerPixel) {
    case 32:
        surface0.srcFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        surface0.srcFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        surface0.srcFormat = DRM_FORMAT_RGB565;
        break;
    default:
        surface0.srcFormat = MY_FORMAT_ARGB8888;
        break;
    }

    switch (dst->format->BitsPerPixel) {
    case 32:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    case 24:
        dstFormat = DRM_FORMAT_RGB888;
        break;
    case 16:
        dstFormat = DRM_FORMAT_RGB565;
        break;
    default:
        dstFormat = MY_FORMAT_ARGB8888;
        break;
    }

    surface0.srcFd = src->bmHwInfo.bmFd;
    surface0.srcDataOffset = 0;
    surface0.srcWidth = src->bmWidth;
    surface0.srcHeight = src->bmHeight;
    surface0.globalAlpha = 1.0f;
    memcpy(&surface0.srcCrop, &crop0, sizeof(crop0));
    memcpy(&surface0.dstWinRect, &dstWinRect0, sizeof(dstWinRect0));

    renderEngineSetSurface(data->render, &surface0);

    target = renderEngineCreateOffScreenTarget(data->render, scalrect->w,
            scalrect->h, dstFormat, scalerBmFd, 0);

    if (renderEngineDrawOffScreen(data->render, target, 1, 0.0f, NULL) < 0) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlphaScaler drawLayer failed\n");
        pthread_mutex_unlock(&this->hidden->updateLock);
        return -1;
    }

    renderEngineDestroyOffScreenTarget(data->render, target);

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlphaScaler srcFd=%d srcOffset=%d src=[%d, %d, %d, %d] "
                    "srcrect=[%d, %d, %d, %d] dstFd=%d dstOffset=%d dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d]\n",
            surface0.srcFd, surface0.srcDataOffset, surface0.srcWidth,
            surface0.srcHeight, src->bmWidth, src->bmHeight,
            surface0.srcCrop.left, surface0.srcCrop.top, surface0.srcCrop.right,
            surface0.srcCrop.bottom, dstBucket->fbDmaBuf.fd,
            dstBucket->isScreen ? data->flipPage * data->mappedOneBufLen : 0,
            dst->w, dst->h, dstBucket->w, dstBucket->h,
            surface0.dstWinRect.left, surface0.dstWinRect.top,
            surface0.dstWinRect.right, surface0.dstWinRect.bottom);
#endif

    /* Build a tmp image */
    memcpy(&tmpBmp, src, sizeof(BITMAP));
    tmpBmp.bmType |= BMP_TYPE_ALPHA;
    tmpBmp.bmBits = scalerBmBits;
    tmpBmp.bmHwInfo.bmFd = scalerBmFd;
    tmpBmp.bmWidth = scalrect->w;
    tmpBmp.bmHeight = scalrect->h;
    tmpBmp.bmPitch = scalerBmPitch;

    dstw = dstrect->w;
    dsth = dstrect->h;

    /* E.g dstrect=[96, -44, 618, 618] devrect=[5, 25, 790, 450] scalrect=[0, 0, 618, 618] */
    if (dstrect->x < devrect->x) {
        srcx = -dstrect->x + devrect->x + scalrect->x;
        dstx = devrect->x;
    } else {
        srcx = scalrect->x;
        dstx = dstrect->x;
    }

    if (dstrect->y < devrect->y) {
        srcy = -dstrect->y + devrect->y + scalrect->y;
        dsty = devrect->y;
    } else {
        srcy = scalrect->y;
        dsty = dstrect->y;
    }

    /* If the width exceeds the width of the border */
    /* (dst->w - devrect->w - devrect->x) is the width of the right border */
    if (dstx + dstrect->w > devrect->w)
        dstw = dst->w - dstx - (dst->w - devrect->w - devrect->x);
    /* If the height exceeds the width of the border */
    /* (dst->h - devrect->h - devrect->y) is the height of the bottom border */
    if (dsty + dstrect->h > devrect->h)
        dsth = dst->h - dsty - (dst->h - devrect->h - devrect->y);

    /* Ensure that the final range does not exceed the actual buffer size,
     * especially the width and height of the created window and
     * the rotating buffer may not correspond */
#ifdef _MGIMAGE_GPU_ROTATE
    if (dstx + dstw > data->rotateBucket.w)
        dstw = data->rotateBucket.w - dstx;
    if (dsty + dsth > data->rotateBucket.h)
        dsth = data->rotateBucket.h - dsty;
#else
    if (dstx + dstw > data->w)
        dstw = data->w - dstx;
    if (dsty + dsth > data->h)
        dsth = data->h - dsty;
#endif

    GAL_Rect bmpSrcrect = { srcx, srcy, dstw, dsth };
    GAL_Rect bmpDstrect = { dstx, dsty, dstw, dsth };

#ifdef SUNXIFB_DEBUG
    GAL_SetError(
            "NEWGAL>SUNXIFB: SUNXIFB_HWPutBoxAlphaScaler srcrect=[%d, %d, %d, %d] "
                    "dstrect=[%d, %d, %d, %d]\n", srcx, srcy, dstw, dsth, dstx,
            dsty, dstw, dsth);
#endif

    pthread_mutex_unlock(&this->hidden->updateLock);

    SUNXIFB_HWPutBoxAlpha(this, &tmpBmp, &bmpSrcrect, dst, &bmpDstrect);

    SunxiMemPfree(data->pMemops, (void*) scalerBmBits);
    scalerBmBits = NULL;

    return 0;
}

static int SUNXIFB_FillHWRect(_THIS, GAL_Surface *dst, GAL_Rect *rect,
        Uint32 color) {
    struct GAL_PrivateVideoData *data = this->hidden;
    vidmem_bucket *dstBucket = (vidmem_bucket*) dst->hwdata;
    unsigned int dstFormat;
    unsigned long long target;
    unsigned rv, gv, bv, av;
    float r, g, b, a;

#ifdef SUNXIFB_DEBUG
    GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_FillHWRect dstAddrPhy=%p "
            "dst=[%d, %d, %d, %d] dstrect=[%d, %d, %d, %d] color=%u\n",
            dstBucket->addrPhy, dst->w, dst->h, dstBucket->w, dstBucket->h,
            rect->x, rect->y, rect->w, rect->h, color);
#endif

    if (data->gpuThreadId != pthread_self()) {
        GAL_SetError(
                "NEWGAL>SUNXIFB: SUNXIFB_FillHWRect cannot be called outside gpuThreadId\n");
        return -1;
    }

    if (rect->w == 0 || rect->h == 0 || !dstBucket) {
        GAL_SetError("NEWGAL>SUNXIFB: SUNXIFB_FillHWRect parameter error\n");
        return -1;
    }

    pthread_mutex_lock(&this->hidden->updateLock);

    SunxiMemFlushCache(data->pMemops, dst->pixels, dst->h * dst->pitch);

#ifdef _MGIMAGE_GPU_ROTATE
    if (dstBucket->isScreen)
        target = data->rotateBucket.target[0];
    else
        target = dstBucket->target[0];
#else
    target = dstBucket->target[dstBucket->isScreen ? data->flipPage : 0];
#endif

    if (target == NULL) {
        switch (dst->format->BitsPerPixel) {
        case 32:
            dstFormat = MY_FORMAT_ARGB8888;
            break;
        case 24:
            dstFormat = DRM_FORMAT_RGB888;
            break;
        case 16:
            dstFormat = DRM_FORMAT_RGB565;
            break;
        default:
            dstFormat = MY_FORMAT_ARGB8888;
            break;
        }

        dstBucket->target[0] = renderEngineCreateOffScreenTarget(data->render,
                dstBucket->w, dstBucket->h, dstFormat, dstBucket->fbDmaBuf.fd,
                0);
        target = dstBucket->target[0];
    }

    rv = (color & dst->format->Rmask) >> dst->format->Rshift;
    r = (float) ((rv << dst->format->Rloss) + (rv >> (8 - dst->format->Rloss)))
            / 255.0f;
    gv = (color & dst->format->Gmask) >> dst->format->Gshift;
    g = (float) ((gv << dst->format->Gloss) + (gv >> (8 - dst->format->Gloss)))
            / 255.0f;
    bv = (color & dst->format->Bmask) >> dst->format->Bshift;
    b = (float) ((bv << dst->format->Bloss) + (bv >> (8 - dst->format->Bloss)))
            / 255.0f;
    av = (color & dst->format->Amask) >> dst->format->Ashift;
    a = (float) ((av << dst->format->Aloss) + (av >> (8 - dst->format->Aloss)))
            / 255.0f;

    renderEngineSetTargetColor(data->render, target, rect->x, rect->y,
            rect->x + rect->w, rect->y + rect->h, r, g, b, a);

    pthread_mutex_unlock(&this->hidden->updateLock);

    return 0;
}

static int SUNXIFB_SetHWAlpha(_THIS, GAL_Surface *surface, Uint8 alpha) {
    surface->format->alpha = alpha;
    surface->flags |= GAL_SRCALPHA;
    return 0;
}

static int SUNXIFB_SetHWColorKey(_THIS, GAL_Surface *src, Uint32 key) {
    src->format->colorkey = key;
    src->flags |= GAL_SRCCOLORKEY;
    return 0;
}
#endif

static void* task_do_update(void *data) {
    _THIS;
    this = data;

    /* waiting for __gal_screen */
    for (;;) {
        if (__gal_screen != NULL) {
            break;
        }
    }

    while (run_flag) {
        /* After flip off, not in flip */
        if (this->doubleBufferStatus) {
            int tempIndex = this->hidden->mappedDispIndex + 1;
            if (tempIndex > this->hidden->mappedBufNum - 1)
                tempIndex = 0;
            if (tempIndex != this->hidden->mappedDrawIndex) {
                /* Already drawn one frame and can display */
                if (this->hidden->cacheFlag) {
                    unsigned long args[2];
                    args[0] = (unsigned long) this->hidden->mappedMem
                            + tempIndex * this->hidden->mappedOneBufLen;
                    args[1] = this->hidden->mappedOneBufLen;
                    if (ioctl(this->hidden->consoleFd, FBIO_CACHE_SYNC, args)
                            < 0) {
                        GAL_SetError(
                                "NEWGAL>SUNXIFB: FBIO_CACHE_SYNC failed\n");
                    }
                }

                this->hidden->cacheVinfo.yoffset = tempIndex * this->hidden->h;
                if (ioctl(this->hidden->consoleFd, FBIOPAN_DISPLAY,
                        &this->hidden->cacheVinfo) < 0) {
                    GAL_SetError("NEWGAL>SUNXIFB: FBIOPAN_DISPLAY failed\n");
                }

                this->hidden->mappedDispIndex = tempIndex;
                pthread_cond_signal(&this->hidden->drawCond);
#ifdef SUNXIFB_DEBUG
                static struct timeval new, old;
                static int fps;
                gettimeofday(&new, NULL);
                if (new.tv_sec * 1000 - old.tv_sec * 1000 >= 1000) {
                    GAL_SetError("NEWGAL>SUNXIFB: Task do update fps is %d\n",
                            fps);
                    old = new;
                    fps = 0;
                } else {
                    fps++;
                }
#endif
            } else {
                /* Avoid deadlocks */
                pthread_cond_signal(&this->hidden->drawCond);
            }
        }

        /* Waiting for the screen refresh cycle */
        /*        if (ioctl(this->hidden->consoleFd, FBIO_WAITFORVSYNC, NULL) < 0) {
         GAL_SetError("NEWGAL>SUNXIFB: FBIO_WAITFORVSYNC failed\n");
         }*/

        usleep(1000 * 1);
    }

    end_flag = 1;
    return NULL;
}

#endif /* _MGGAL_SUNXIFB */
