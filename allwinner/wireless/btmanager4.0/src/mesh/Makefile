#INSTALL_DIR := ../../install
API := ./api
#INSTALL_LIBS_DIR := ../../install/usr/lib

subdirs :=
MFLAGS :=
SRC :=
CFG_MEST_TEST_LOG_ENABLE := true
CFG_LIB_MESH_ENABLE := true
CFG_PTS_APP_ENABLE := true
CFG_NORDIC_MODULE_ENABLE := true
CFG_RTK_MODULE_ENABLE := true
CFG_OPENAPI_ENABLE := true

MFLAGS=
INCLUDES=
LDADD=
LDFLAGS=
TARGET_OBJ_DIR=
MFLAGS += -DHAVE_CONFIG_H
target = libmesh-api.a
INCLUDES + = ./
INCLUDES += -I ./bluez/include

ifeq ($(CFG_MEST_TEST_LOG_ENABLE),true)
    MFLAGS  += -DMEST_TEST_LOG_ENABLE
endif

ifeq ($(CFG_LIB_MESH_ENABLE),true)
    subdirs += stack/component.mk
#    INCLUDES += -I $(INSTALL_DIR)/
    INCLUDES += -I $(API)/include/
    INCLUDES += -I ./
    INCLUDES += -I ./stack/src/
endif

ifeq ($(CFG_OPENAPI_ENABLE),true)
    subdirs += $(API)/component.mk
endif

ifeq ($(CFG_NORDIC_MODULE_ENABLE),true)
    subdirs += adapter/models/nordic/component.mk
    MFLAGS  += -DNORDIC_MODELS_ENABLE
endif

ifeq ($(CFG_RTK_MODULE_ENABLE),true)
    subdirs += adapter/models/rtk/component.mk
    MFLAGS  += -DRTK_MODELS
endif

ifeq ($(CFG_PTS_APP_ENABLE),true)
    subdirs += adapter/proj/pts/component.mk
    subdirs += adapter/proj/pts/src/server/component.mk
#    MFLAGS  += -DPTS_APP_ENABLE
endif

subdirs += adapter/proj/model_adaptor/component.mk
include $(subdirs)

SRC += adapter/mesh_arch.c \
	adapter/application.c \
	adapter/mesh_db.c \
	adapter/mesh_gateway.c \
	adapter/mesh_agent.c \
	adapter/mesh_node.c \
	adapter/mesh_element.c \
	adapter/mesh_cfg_server.c \
	adapter/mesh_cfg_client.c \
	adapter/mesh_gap.c \
	adapter/provisioner.c \
	adapter/provisionee.c \
	adapter/dbus.c \
	adapter/bluez/util.c \
	adapter/aw_dbg.c

INCLUDES += -I ./adapter/include/
INCLUDES += -I ./adapter/bluez/include/

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp $(API)/include/*.h $(CONFIG_PREFIX)/usr/include
	@cp adapter/bluez/include/*.h $(CONFIG_PREFIX)/usr/include

all: install $(target)

OBJS = $(patsubst %c, %o, $(SRC))

$(OBJS): %.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $(MFLAGS) $(LDFLAGS)  $< -o $@

$(target): $(OBJS)
	$(AR) -rcs $@ $^
	cp $@ $(CONFIG_PREFIX)/usr/lib

release:
	make clean all RELEASE=1

clean_prompt:
	@echo "-----------------------------------------";
	@echo " make clean                         		";
	@echo "-----------------------------------------";

clean: clean_prompt
	rm -rf $(target)
	rm -rf $(OBJS)
