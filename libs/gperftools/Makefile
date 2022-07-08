#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gperftools
PKG_VERSION:=2.9.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/$(PKG_NAME)/$(PKG_NAME)/releases/download/$(PKG_NAME)-$(PKG_VERSION)/
PKG_HASH:=240deacdd628b6459671b83eb0c4db8e97baadf659f25b92e9a078d536bd513e

PKG_MAINTAINER:=Gerrit Beine <gb@simonwunderlich.de>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk

define Package/gperftools/Default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://github.com/gperftools/gperftools
  DEPENDS:=+libstdcpp
endef

define Package/gperftools
  $(call Package/gperftools/Default)
  TITLE:=Google performance tools
endef

define Package/tcmalloc
  $(call Package/gperftools/Default)
  TITLE:=high-performance multi-threaded malloc() - full version
endef

define Package/tcmalloc-minimal
  $(call Package/gperftools/Default)
  TITLE:=high-performance multi-threaded malloc() - light version
endef

define Package/gperftools/Default/description
	gperftools is a collection of a high-performance multi-threaded
	malloc() implementation, plus some pretty nifty performance analysis
	tools.
endef

define Package/gperftools/description
	$(call Package/gperftools/Default/description)
	.
	This package contains the perftools.
endef

define Package/tcmalloc/description
	$(call Package/gperftools/Default/description)
	.
	This package contains the full version of tcmalloc.
endef

define Package/tcmalloc-minimal/description
	$(call Package/gperftools/Default/description)
	.
	This package contains the minimal version of tcmalloc.
endef

define Package/gperftools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pprof $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*profiler.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/{google,gperftools}
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/google/* $(1)/usr/include/google/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/gperftools/* $(1)/usr/include/gperftools/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{so*,a,la} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
endef

define Package/tcmalloc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtcmalloc.so* $(1)/usr/lib/
endef

define Package/tcmalloc-minimal/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtcmalloc_minimal.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,gperftools))
$(eval $(call BuildPackage,tcmalloc))
$(eval $(call BuildPackage,tcmalloc-minimal))
