include $(TOPDIR)/rules.mk

PKG_NAME:= wirelesscommon
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

PKG_CONFIG_DEPENDS:= \
    CONFIG_USES_BCMDHD \
    CONFIG_USES_XR829 \
    CONFIG_USES_XR819S \
    CONFIG_USES_REALTEK \
    CONFIG_USES_ESP8089

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  SUBMENU:=wireless
  TITLE:= Allwinner Wi-Fi/BT Public lib
  DEPENDS:= +zlib
endef

define Package/$(PKG_NAME)/description
	Wi-Fi/BT api_action and log tracing
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lz -lrt -lm -ldl)

define Build/Configure
endef

ifeq ($(CONFIG_PACKAGE_wirelesscommon),y)
define Build/Compile/$(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		PKG_BUILD_DIR="$(PKG_BUILD_DIR)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_LIBC="$(CONFIG_LIBC)" \
		STAGING_DIR="$(STAGING_DIR)" \
		CPU="$(ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef
endif

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwirelesscom.so $(1)/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwirelesscom.so $(1)/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
