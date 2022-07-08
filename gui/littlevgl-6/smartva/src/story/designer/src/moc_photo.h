#ifndef __MOC_PHOTO_H__
#define __MOC_PHOTO_H__
#include "media_mixture.h"
typedef struct
{
    int  is_auto_play;
    int  index;
    char mountpoint[MOUNT_PATH_MAX_LENGTH];
    char filename[FILE_PATH_LEN];
}photo_user_data_t;

typedef struct
{
    int  effect;
    int  speed;
    char filename[FILE_PATH_LEN];
    char mountpoint[MOUNT_PATH_MAX_LENGTH];
}photo_app_data_t;
#endif /*__MOC_PHOTO_H__*/
