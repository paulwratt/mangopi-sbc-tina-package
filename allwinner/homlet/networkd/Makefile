#
# Copyright (C) 2018-2019  Chipsenfuture Technology Co.,Ltd. All rights reserved.
#
# Authors:
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#
#
##############################################
include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=networkd
PKG_VERSION:=1
PKG_RELEASE:=1

HAVE_SRC := yes

SRC_CODE_DIR := $(TOPDIR)/external/networkd
PKG_FILE_DEPENDS := $(SRC_CODE_DIR)

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)


include $(BUILD_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SUBMENU:=Homlet
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Tina Homlet Netwrok Daemon
  DEPENDS:=+libubus +libuci +libjson-c +ubus +wifimanager +libnl-tiny
endef


# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
	Tina Homlet Netwrok Daemon
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)

ifeq ($(HAVE_SRC),yes)
	$(CP) -r $(SRC_CODE_DIR)/src/* $(PKG_BUILD_DIR)/
endif

endef

define Build/Configure
endef

#TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include \

TARGET_LDFLAGS += -lrt -ldl -lm -lubus -lubox \
		-lpthread -luci -lblobmsg_json -ljson-c -lwifimg\
		-L$(STAGING_DIR)/usr/lib


TARGET_LDFLAGS += -lrt -ldl -lm \
		-lpthread \
		-L$(STAGING_DIR)/usr/lib

TARGET_CFLAGS += -Iinclude -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/libnl-tiny -O2 -Wall
define Build/Compile

ifeq ($(HAVE_SRC),yes)
	#complie networkd
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		KERNEL_DIR="$(LINUX_DIR)"
endif

endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one


# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/networkd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib

	$(INSTALL_BIN) ./files/clear_net_config.sh $(1)/usr/bin/clear_net_config.sh
	$(INSTALL_BIN) ./files/disable_softap.sh $(1)/usr/bin/disable_softap.sh
	$(INSTALL_BIN) ./files/enable_softap.sh $(1)/usr/bin/enable_softap.sh
	$(INSTALL_BIN) ./files/miracast_start_p2p0.sh $(1)/usr/bin/miracast_start_p2p0.sh
	$(INSTALL_BIN) ./files/miracast_stop_p2p0.sh $(1)/usr/bin/miracast_stop_p2p0.sh
	$(INSTALL_BIN) ./files/softap.sh $(1)/usr/bin/softap.sh
	$(INSTALL_BIN) ./files/wifi_manager_init.sh $(1)/usr/bin/wifi_manager_init.sh

ifeq ($(HAVE_SRC),yes)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/networkd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/networkd_client $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libnetworkd.so $(1)/usr/lib/
endif

	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DATA) ./files/dnsmasq.conf $(1)/etc/dnsmasq.conf

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/networkd.init $(1)/etc/init.d/networkd
	$(INSTALL_BIN) ./files/starthostapd.init $(1)/etc/init.d/starthostapd
	$(INSTALL_BIN) ./files/dnsmasq.init $(1)/etc/init.d/dnsmasq
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
