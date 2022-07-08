include $(TOPDIR)/rules.mk

PKG_NAME:=cartographer
PKG_VERSION:=1.0.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=Apache License 2.0

PKG_BUILD_DEPENDS:= boost-iostreams googletest abseil-cpp ceres-solver lua5.3 protobuf libunwind gflags glog libcairo

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+boost-iostreams +boost-regex +googletest +abseil-cpp +ceres-solver +lua5.3 +protobuf +libunwind +gflags +glog +libcairo +@CAIRO_PNG
  TITLE:=google cartographer
  URL:=https://github.com/cartographer-project/cartographer/tree/2.0.0
endef

define Package/$(PKG_NAME)/description
Cartographer is a system that provides real-time simultaneous localization and mapping (SLAM) in 2D and 3D across multiple platforms and sensor configurations.
endef

PKG_JOBS = -j64
TARGET_CFLAGS += -lgmock
TARGET_CXXFLAGS += -lgmock
CMAKE_OPTIONS += -DLUA_INCLUDE_DIR=$(STAGING_DIR)/usr/include/lua5.3
CMAKE_OPTIONS += -DPROTOBUF_PROTOC_EXECUTABLE=$(OUT_DIR)/host/bin/protoc

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

#	$(INSTALL_DIR) $(1)/usr/include/cartographer/mapping/proto/2d
#	$(CP) $(PKG_INSTALL_DIR)/usr/include/cartographer/mapping/proto/*2d*.h $(1)/usr/include/cartographer/mapping/proto/2d

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
