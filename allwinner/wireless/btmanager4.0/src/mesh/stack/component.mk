#
# Component Makefile
#
MESH_SRC := stack/src
MESH_INCLUDE := stack/include

ifeq ($(CFG_LIB_MESH_ENABLE),true)
INCLUDES += -I $(MESH_INCLUDE)
SRC += $(MESH_SRC)/mesh.c \
    $(MESH_SRC)/net-keys.c \
	$(MESH_SRC)/mesh-io.c \
	$(MESH_SRC)/mesh-mgmt.c \
	$(MESH_SRC)/mesh-io-generic.c \
	$(MESH_SRC)/net.c \
	$(MESH_SRC)/crypto.c \
	$(MESH_SRC)/friend.c \
	$(MESH_SRC)/appkey.c \
	$(MESH_SRC)/node.c \
	$(MESH_SRC)/model.c \
	$(MESH_SRC)/cfgmod-server.c \
	$(MESH_SRC)/cfgmod-client.c  \
	$(MESH_SRC)/mesh-config-json.c \
	$(MESH_SRC)/dbus.c \
	$(MESH_SRC)/agent.c \
	$(MESH_SRC)/prov-acceptor.c \
	$(MESH_SRC)/prov-initiator.c \
	$(MESH_SRC)/manager.c \
	$(MESH_SRC)/pb-adv.c \
	$(MESH_SRC)/keyring.c \
	$(MESH_SRC)/xr829-patch.c \
	$(MESH_SRC)/rpl.c  \
	$(MESH_SRC)/util.c \
	$(MESH_SRC)/main.c  \
	$(MESH_SRC)/shared_hci.c  \
	$(MESH_SRC)/shared_mgmt.c  \
	$(MESH_SRC)/shared_ecc.c  \
	$(MESH_SRC)/shared_queue.c  \
	$(MESH_SRC)/shared_io-ell.c \
	$(MESH_SRC)/shared_util.c 
endif
#	$(MESH_SRC)/shared_io-ell.c \
