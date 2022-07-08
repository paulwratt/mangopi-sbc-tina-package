#
# Copyright (C) 2015-2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=flutter-sunxi
PKG_VERSION:=1.0.6
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_HASH:=8b85dacaed4eee43d16813f3d3230c4f7732b3cbccc2a3fa6c7fed658c98efe4

PKG_MAINTAINER:=anruliu <anruliu@allwinnertech.com>
PKG_LICENSE:=BSD 3-Clause "New" or "Revised" License
PKG_LICENSE_FILES:=LICENSE

ifeq ($(ARCH), arm)
FLUTTER_ARCH:=arm
else
FLUTTER_ARCH:=arm64
endif

PKG_CONFIG_DEPENDS := \
	CONFIG_FLUTTER_USE_FBDEV \
	CONFIG_FLUTTER_USE_EGLFS \
	CONFIG_FLUTTER_DEMO_COMPLEX_LAYOUT \
	CONFIG_FLUTTER_DEMO_GALLERY

include $(BUILD_DIR)/package.mk

define Package/flutter-sunxi
  TITLE:=flutter-sunxi
  SECTION:=libs
  SUBMENU:=Flutter
  CATEGORY:=Gui
  URL:=https://flutter.dev/
  DEPENDS:=@USE_GLIBC +FLUTTER_USE_EGLFS:libgpu
  MENU:=1
endef

define Package/flutter-sunxi/description
	Flutter transforms the app development process. Build, test, and deploy beautiful mobile, web, desktop, and embedded apps from a single codebase.
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	$(Build/Prepare/Default)
ifeq ($(CONFIG_FLUTTER_DEMO_COMPLEX_LAYOUT),y)
	$(TAR) -zxvf $(PKG_BUILD_DIR)/complex_layout.tar.gz -C $(PKG_BUILD_DIR)
endif
ifeq ($(CONFIG_FLUTTER_DEMO_GALLERY),y)
	$(TAR) -zxvf $(PKG_BUILD_DIR)/gallery.tar.gz -C $(PKG_BUILD_DIR)
endif
endef

define Build/Compile
endef

define Package/flutter-sunxi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/libflutter_engine.so $(1)/usr/lib
ifeq ($(CONFIG_FLUTTER_USE_FBDEV),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/flutter_fbdev $(1)/usr/bin
endif
ifeq ($(CONFIG_FLUTTER_USE_EGLFS),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/flutter_eglfs $(1)/usr/bin
endif
ifeq ($(CONFIG_FLUTTER_DEMO_COMPLEX_LAYOUT),y)
	$(CP) $(PKG_BUILD_DIR)/bundle_complex_layout $(1)/usr/bin
endif
ifeq ($(CONFIG_FLUTTER_DEMO_GALLERY),y)
	$(CP) $(PKG_BUILD_DIR)/bundle_gallery $(1)/usr/bin
endif
endef

$(eval $(call BuildPackage,flutter-sunxi))
