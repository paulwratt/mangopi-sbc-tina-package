#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=displayd
PKG_VERSION:=1.0
PKG_RELEASE:=1

SRC_CODE_DIR := $(TOPDIR)/external/displayd
PKG_FILE_DEPENDS := $(SRC_CODE_DIR)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=Homlet
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:=+libcutils +libsysutils
  TITLE:=displayd
  MAINTAINER:=anruliu<anruliu@allwinnertech.com>
endef

define Package/$(PKG_NAME)/description
  The simple test application for Demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS+=-DPLATFORM_SUN8IW7P1

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) \
		$(PKG_BUILD_DIR)/src/displayd \
		$(1)/usr/bin/

endef

$(eval $(call BuildPackage,$(PKG_NAME)))
