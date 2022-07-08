#
# Component Makefile
#
PTS_SRC := adapter/proj/pts/src
PTS_CLIENT_SRC := adapter/proj/pts/src/client
PTS_INCLUDE := adapter/proj/pts/include

ifeq ($(CFG_PTS_APP_ENABLE),true)
INCLUDES += -I $(PTS_INCLUDE)
SRC += $(PTS_SRC)/pts_app.c \
	$(PTS_SRC)/app_timer_mesh.c \
	$(PTS_CLIENT_SRC)/app_generic_on_off_client.c \
	$(PTS_CLIENT_SRC)/app_generic_level_client.c \
	$(PTS_CLIENT_SRC)/app_generic_default_transition_time_client.c \
	$(PTS_CLIENT_SRC)/app_generic_power_onoff_client.c \
	$(PTS_CLIENT_SRC)/app_generic_power_level_client.c \
	$(PTS_CLIENT_SRC)/app_generic_battery_client.c \
	$(PTS_CLIENT_SRC)/app_light_lightness_client.c \
	$(PTS_CLIENT_SRC)/app_light_ctl_client.c \
	$(PTS_CLIENT_SRC)/app_light_hsl_client.c \
	$(PTS_CLIENT_SRC)/app_health_client.c \
	$(PTS_CLIENT_SRC)/app_pts_ts.c
endif