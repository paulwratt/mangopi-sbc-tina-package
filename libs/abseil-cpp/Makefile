include $(TOPDIR)/rules.mk

PKG_NAME:=abseil-cpp
PKG_VERSION:=20210324.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=Apache License 2.0

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Abseil - C++ Common Libraries
  URL:=https://github.com/abseil/abseil-cpp/tree/20210324.2
endef

define Package/$(PKG_NAME)/description
The repository contains the Abseil C++ library code. Abseil is an open-source collection of C++ code (compliant to C++11) designed to augment the C++ standard library.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
