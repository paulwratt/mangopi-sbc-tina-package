#
# Component Makefile
#
NORDIC_SRC := adapter/models/nordic/src
NORDIC_INCLUDE := adapter/models/nordic/include

ifeq ($(CFG_NORDIC_MODULE_ENABLE),true)
INCLUDES += -I $(NORDIC_INCLUDE)
SRC += $(NORDIC_SRC)/model_mgr.c \
	$(NORDIC_SRC)/model_common.c \
	$(NORDIC_SRC)/generic_onoff_server.c \
	$(NORDIC_SRC)/generic_onoff_client.c \
	$(NORDIC_SRC)/generic_level_client.c \
	$(NORDIC_SRC)/generic_level_server.c \
	$(NORDIC_SRC)/generic_ponoff_client.c \
	$(NORDIC_SRC)/generic_ponoff_setup_server.c \
	$(NORDIC_SRC)/generic_dtt_client.c \
	$(NORDIC_SRC)/generic_dtt_server.c
endif
