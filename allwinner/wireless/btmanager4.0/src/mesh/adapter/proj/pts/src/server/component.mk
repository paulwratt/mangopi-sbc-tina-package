#
# Component Makefile
#
SERVER_SRC := adapter/proj/pts/src/server

#ifeq ($(CFG_PTS_APP_ENABLE),true)
INCLUDES += -I $(PTS_INCLUDE)
SRC += $(SERVER_SRC)/app_generic_on_off_server_rtk.c \
	$(SERVER_SRC)/app_generic_level_server.c \
	$(SERVER_SRC)/app_generic_power_level_server.c \
	$(SERVER_SRC)/app_generic_ponoff_setup_server.c \
	$(SERVER_SRC)/app_generic_default_transition_time_server.c \
	$(SERVER_SRC)/app_battery_server.c \
	$(SERVER_SRC)/app_light_lightness_server.c \
	$(SERVER_SRC)/app_light_ctl_server.c \
	$(SERVER_SRC)/app_light_hsl_server.c
#endif