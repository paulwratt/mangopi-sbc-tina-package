include $(TOPDIR)/rules.mk

PKG_NAME:=wifimanager-v2.0
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  SUBMENU:=wireless
  TITLE:=Tina wifimanager-v2.0
  DEPENDS:=+libpthread +libnl-tiny +libstdcpp +liballwinner-base +wirelesscommon +libopenssl
endef

define Package/$(PKG_NAME)
  $(call Package/wifimanager-v2.0/Default)
  MENU:=1
  DEFAULT:=1
endef

ifdef CONFIG_WMG_PROTOCOL_BLE
DEMO_DEPENDS_FILE += +btmanager-v4.0 +libjson-c
endif

ifdef CONFIG_WMG_PROTOCOL_XCONFIG
DEMO_DEPENDS_FILE += +libopenssl
endif

ifdef CONFIG_WMG_PROTOCOL_SOUNDWAVE
DEMO_DEPENDS_FILE += +alsa-lib +libuci
endif

define Package/$(PKG_NAME)-demo
  $(call Package/wifimanager-v2.0/Default)
  TITLE:=Tina wifimanager-v2.0 app demo
  DEPENDS := wifimanager-v2.0 +wifimanager-v2.0 +wirelesscommon $(DEMO_DEPENDS_FILE)
endef


define Package/$(PKG_NAME)-demo/config
source "$(SOURCE)/Config.in"
endef

PKG_CONFIG_DEPENDS += CONFIG_WIFIMANAGER_ENABLE_DAEMON

define Package/$(PKG_NAME)/description
	Tina wifi manager v2.0
endef

define Package/$(PKG_NAME)-demo/description
	Tina wifimanager-v2.0 app demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./core $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lrt -ldl)
##TARGET_CFLAGS += -DOS_NET_LINUX_OS

define Build/Compile/$(PKG_NAME)
	#libwifimg-v2.0.so
	$(MAKE) -C $(PKG_BUILD_DIR)/core/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

ifeq ($(CONFIG_PACKAGE_wifimanager-v2.0-demo),y)
define Build/Compile/$(PKG_NAME)-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef
endif

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
	$(Build/Compile/$(PKG_NAME)-demo)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifimg-v2.0.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_DIR) $(1)/etc/wifi/wpa_supplicant
	$(INSTALL_DIR) $(1)/etc/wifi/hostapd
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifimg-v2.0.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DATA) ./files/hostapd.conf $(1)/etc/wifi/hostapd
	$(INSTALL_DATA) ./files/wpa_supplicant.conf $(1)/etc/wifi/wpa_supplicant
	$(INSTALL_DATA) ./files/wpa_supplicant_src.conf $(1)/etc/wifi/wpa_supplicant
	$(INSTALL_DATA) ./files/wpa_supplicant_overlay.conf $(1)/etc/wifi/wpa_supplicant
	$(INSTALL_DATA) ./files/wifi_monitor.conf $(1)/etc/wifi
	$(INSTALL_BIN) ./files/wpa_supplicant.init $(1)/etc/init.d/wpa_supplicant
	$(INSTALL_BIN) ./files/udhcpc_wlan0.init $(1)/etc/wifi/udhcpc_wlan0
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_deamon $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi $(1)/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
