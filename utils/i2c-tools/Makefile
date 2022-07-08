#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=i2c-tools
PKG_VERSION:=3.1.2
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://dl.lm-sensors.org/i2c-tools/releases/ \
                http://fossies.org/linux/misc/

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_MD5SUM:=7104a1043d11a5e2c7b131614eb1b962

PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=COPYING

include $(BUILD_DIR)/package.mk

define Package/i2c/Default
  URL:=http://lm-sensors.org/wiki/I2CTools
  TITLE:=I2C
endef

define Package/i2c-tools
  $(call Package/i2c/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+=tools for Linux
endef

define Package/i2c-tools/description
 This package contains an heterogeneous set of I2C tools for Linux. These tools
 were originally part of the lm-sensors package.
endef

TARGET_CPPFLAGS += -I$(PKG_BUILD_DIR)/include

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		LINUX="$(LINUX_DIR)" \
		CC="$(TARGET_CC)" \
		STAGING_DIR="$(STAGING_DIR)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/i2c-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/i2cdetect $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/i2cdump $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/i2cset $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/i2cget $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,i2c-tools))
