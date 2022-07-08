#
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wpa_supplicant_rtl
PKG_VERSION:=4.3
PKG_RELEASE:=1

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
#PKG_SOURCE_URL:=https://www.kernel.org/pub/software/network/iw
#PKG_MD5SUM:=7adec72e91ebdd9c55429fa34a23a6f5

#PKG_MAINTAINER:=Felix Fietkau <nbd@openwrt.org>
#PKG_LICENSE:=GPL-2.0

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=wpa_supplicant for Realtek
  URL:=http://wireless.kernel.org/en/users/Documentation/iw
  DEPENDS:= +libnl-tiny 
endef

define Package/$(PKG_NAME)/description
	wpa_supplicant for Realtek
endef

define Build/Configure
	echo Empty
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef


TARGET_CPPFLAGS:= \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	$(TARGET_CPPFLAGS) \
	-D_GNU_SOURCE

MAKE_FLAGS += \
	EXTRA_CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -ffunction-sections -fdata-sections" \
	EXTRA_LDFLAGS="$(TARGET_LDFLAGS) -Wl,--gc-sections" 
 

define Build/Compile
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/tina.config $(PKG_BUILD_DIR)/wpa_supplicant/.config
	cd $(PKG_BUILD_DIR)/wpa_supplicant; \
	$(MAKE) $(MAKE_FLAGS); 
endef

define Build/InstallDev
	echo Empty InstallDev 
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_cli $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
