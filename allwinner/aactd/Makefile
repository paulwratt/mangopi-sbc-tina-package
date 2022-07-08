include $(TOPDIR)/rules.mk

PKG_NAME:=aactd
PKG_VERSION:=v1.0.0
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Allwinner
  TITLE:=Allwinner Audio Calibration Tool Daemon
  DEPENDS:=+aactd-lib
endef

define Package/$(PKG_NAME)/description
  A tool which runs on the target device and communicates with AACT
  (Allwinner Audio Calibration Tool) on PC, to support tuning runtime
  audio effect arguments (such as EQ, DRC, and so on).
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		TARGET_PLATFORM="$(TARGET_PLATFORM)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
