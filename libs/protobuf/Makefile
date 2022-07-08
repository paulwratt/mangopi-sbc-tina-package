include $(TOPDIR)/rules.mk

PKG_NAME:=protobuf
PKG_VERSION:=3.10.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DEPENDS:=protobuf/host

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp +zlib
  TITLE:=Protocol Buffers - Google's data interchange format
  URL:=https://github.com/protocolbuffers/protobuf.git
endef

define Package/$(PKG_NAME)/description
Protocol Buffers (a.k.a., protobuf) are Google's language-neutral, platform-neutral, extensible mechanism for serializing structured data. You can find protobuf's documentation on the Google Developers site.
endef

PKG_JOBS = -j4
HOST_JOBS = -j8

CMAKE_SOURCE_SUBDIR:=cmake
CMAKE_OPTIONS += -Dprotobuf_BUILD_TESTS=OFF
CMAKE_HOST_OPTIONS += -Dprotobuf_BUILD_TESTS=OFF

define Host/Install
	$(INSTALL_DIR) $(OUT_DIR)/host/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/protoc $(OUT_DIR)/host/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/protoc-$(PKG_VERSION).0 $(OUT_DIR)/host/bin/

	$(INSTALL_DIR) $(OUT_DIR)/host/lib/
	$(CP) $(HOST_BUILD_DIR)/libproto*.a $(OUT_DIR)/host/lib/

	$(INSTALL_DIR) $(OUT_DIR)/host/lib/cmake/
	$(CP) $(HOST_BUILD_DIR)/cmake/* $(OUT_DIR)/host/lib/cmake/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin

	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,$(PKG_NAME)))