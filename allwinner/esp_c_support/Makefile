include $(TOPDIR)/rules.mk

PKG_NAME:=esp_c_support
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Esp32 c test tool
endef

define Package/$(PKG_NAME)
  $(call Package/esp_c_support/Default)
  MENU:=1
  DEFAULT:=1
endef

PKG_CONFIG_DEPENDS += CONFIG_ESP32_ENABLE_TEST

define Package/$(PKG_NAME)/description
	Esp32 wifi test
endef

define Package/$(PKG_NAME)/config
config ESP32_ENABLE_TEST
	bool "Enable esp32 enable test support"
	default n
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)

define Build/Compile/$(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/esp32_test $(1)/bin
endef
$(eval $(call BuildPackage,$(PKG_NAME)))
