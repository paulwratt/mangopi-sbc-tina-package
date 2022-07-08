include $(TOPDIR)/rules.mk

PKG_NAME:=gflags
PKG_VERSION:=2.2.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=BSD 3-Clause

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=google gflags
  URL:=https://github.com/gflags/gflags/tree/v2.2.2
endef

define Package/$(PKG_NAME)/description
The gflags package contains a C++ library that implements commandline flags processing.
endef

TARGET_CXXFLAGS+=-fPIC -shared

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
