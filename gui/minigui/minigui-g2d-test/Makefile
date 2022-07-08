#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minigui-g2d-test
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  SUBMENU:=Minigui
  CATEGORY:=Gui
  DEPENDS:= +libminigui-gpl +libuapi +MINIGUI_GPU_SUPPORT:libgpuapi
  TITLE:=minigui-g2d-test
  MAINTAINER:=anruliu<anruliu@allwinnertech.com>
endef

ifeq ($(CONFIG_MINIGUI_GPU_SUPPORT),y)
TARGET_LDFLAGS+= -lgpuapi -lstdc++
endif

define Package/$(PKG_NAME)/description
  The minigui g2d test application for Demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/res
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./res $(PKG_BUILD_DIR)/
endef

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
	$(INSTALL_DIR) $(1)/usr/res
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		$(PKG_BUILD_DIR)/src/minigui-g2d-test \
		$(1)/usr/bin/
	$(CP) \
		$(PKG_BUILD_DIR)/res/* \
		$(1)/usr/res
	$(CP) \
		$(PKG_BUILD_DIR)/res/MiniGUI.cfg \
		$(1)/usr/local/etc/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
