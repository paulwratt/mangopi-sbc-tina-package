#ifndef __APP_CONFIG_PARAM_H__
#define __APP_CONFIG_PARAM_H__
#include "rat_common.h"
/* user data save by sence */
#define VA_SAVE_PATH			"/etc/param_save.cfg"
/* major key */
#define PUBLIC_SCENE			"public"
#define MUSIC_SCENE			"music"
#define MOVIE_SCENE			"movie"
#define EXT_MUSIC_SCENE			"ext_music"
#define WLAN_SCENE			"WLAN"
#define OTA_SCENE				"OTA"
#define PHOTO_SCENE			"photo"

/*PUBLIC_SCENE*/
#define VOLUME					"volume"
#define DEFAULT_VOLUME			(15)
#define AUTO_SLEEP				"auto_sleep"
#define DEFAULT_AUTO_SLEEP		(5)
#define AUTO_CLOSE_SCREEN		"auto_close_screen"
#define DEFAULT_CLOSE_SCREEN	(5)
#define AUTO_POWEROFF			"auto_power_off_time"
#define DEFAULT_AUTO_POWEROFF	(5)
#define BACKLIGHT				"backlight"
#define DEFAULT_BACKLIGHT		(50)
#define ENHANCE_MODE			"enhance_mode"
#define DEFAULT_ENHANCE_MODE	(0)
#define ENHANCE_BRIGHT			"enhance_bright"
#define DEFAULT_ENHANCE_BRIGHT	(50)
#define ENHANCE_STATUTION			"enhance_statution"
#define DEFAULT_ENHANCE_STATUTION	(50)
#define ENHANCE_CONTRAST			"enhance_contrast"
#define DEFAULT_ENHANCE_CONTRAST	(50)
#define	LANGUAGE					"language"
#define DEFAULT_LANGUAGE			(1)

#define BREAK_TAG_ROOT_PATH			 "root_path"
/*MUSIC_SCENE*/
/*MOVIE_SCENE*/
/*EXT_MUSIC_SCENE*/
#define	PLAYMODE				"playmode"
#define DEFAULT_PLAYMODE		(RAT_PLAY_MODE_ROTATE_ALL)
#define	LOOP					"loop"
#define DEFAULT_LOOP			(0)
#define	PATH					"path"
#define DEFAULT_PATH			(NULL)
#define	FILENAME				"filename"
#define DEFAULT_FILENAME		(NULL)
#define	FILETYPE				"fileType"
#define DEFAULT_FILETYPE		(NULL)
#define	OFFSET					"offset"
#define DEFAULT_OFFSET			(0)
#define	INDEX					"index"
#define DEFAULT_INDEX			(0)
#define	DURATION				"nDurationSec"
#define DEFAULT_DURATION		(0)


/*WLAN_SCENE*/
#define	WLAN_MANU_ON				"manu_on"
#define DEFAULT_WLAN_MANU_ON		(1)
#define	WLAN_MANU_CONNECTED			"manu_connected"
#define DEFAULT_MANU_CONNECTED		(1)
#define	WLAN_MANU_SSID				"manu_ssid"
#define DEFAULT_MANU_SSID			(NULL)
#define	WLAN_MANU_PASSWORD			"manu_password"
#define DEFAULT_MANU_PASSWORD		(NULL)
#endif
