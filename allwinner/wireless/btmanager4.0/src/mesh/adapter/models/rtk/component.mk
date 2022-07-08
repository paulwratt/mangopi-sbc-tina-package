#
# Component Makefile
#
RTK_SRC := adapter/models/rtk/src
RTK_INCLUDE := adapter/models/rtk/include

ifeq ($(CFG_RTK_MODULE_ENABLE),true)
INCLUDES += -I $(RTK_INCLUDE)
SRC += $(RTK_SRC)/generic_default_transition_time_server.c \
    $(RTK_SRC)/generic_on_off_server.c \
	$(RTK_SRC)/generic_power_level_server.c \
	$(RTK_SRC)/generic_power_level_setup_server.c \
	$(RTK_SRC)/generic_transition_time.c \
	$(RTK_SRC)/generic_power_on_off_server.c \
	$(RTK_SRC)/generic_battery_server.c \
	$(RTK_SRC)/scene_server.c \
	$(RTK_SRC)/scene_setup_server.c \
	$(RTK_SRC)/sensor_server.c \
	$(RTK_SRC)/sensor_setup_server.c \
	$(RTK_SRC)/time_server.c \
	$(RTK_SRC)/time_setup_server.c \
	$(RTK_SRC)/scheduler_server.c \
	$(RTK_SRC)/scheduler_setup_server.c \
	$(RTK_SRC)/light_lightness_server.c \
	$(RTK_SRC)/light_lightness_setup_server.c \
	$(RTK_SRC)/light_ctl_server.c \
	$(RTK_SRC)/light_ctl_setup_server.c \
	$(RTK_SRC)/light_ctl_temperature_server.c \
	$(RTK_SRC)/light_hsl_server.c \
	$(RTK_SRC)/light_hsl_setup_server.c \
	$(RTK_SRC)/light_hsl_saturation_server.c \
	$(RTK_SRC)/light_hsl_hue_server.c \
	$(RTK_SRC)/generic_on_off_client.c \
	$(RTK_SRC)/generic_level_client.c \
	$(RTK_SRC)/generic_default_transition_time_client.c \
	$(RTK_SRC)/generic_power_on_off_client.c \
	$(RTK_SRC)/generic_power_level_client.c \
	$(RTK_SRC)/generic_battery_client.c \
	$(RTK_SRC)/light_lightness_client.c \
	$(RTK_SRC)/light_ctl_client.c \
	$(RTK_SRC)/light_hsl_client.c \
	$(RTK_SRC)/health_client.c \
	$(RTK_SRC)/mesh_access.c
endif
