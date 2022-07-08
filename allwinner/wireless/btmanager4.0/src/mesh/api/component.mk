#
# Component Makefile
#
API_SRC := api/src
API_INCLUDE := api/include

ifeq ($(CFG_OPENAPI_ENABLE),true)
INCLUDES += -I $(API_INCLUDE)
SRC += $(API_SRC)/AWCBAdapter.c \
	$(API_SRC)/AWmeshNodeApi.c \
	$(API_SRC)/AWDbg.c
endif
