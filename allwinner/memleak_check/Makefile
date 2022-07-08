include $(TOPDIR)/rules.mk

PKG_NAME	:=memleak_check
PKG_VERSION	:=0.0.1
PKG_RELEASE	:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
	SECTION	:=utils
	CATEGORY:=Allwinner
	TITLE	:=Tina memleak check
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  MENU:=1
  DEFAULT:=1
endef

define Package/$(PKG_NAME)/description
	Tina memleak check
	malloc/free hook function
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./memleak_check_lib $(PKG_BUILD_DIR)/
	$(CP) -r ./tools $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	#libpeq.so
	$(MAKE) -C $(PKG_BUILD_DIR)/memleak_check_lib/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	#peq_test
	$(MAKE) -C $(PKG_BUILD_DIR)/tools/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libmemleak_check.so $(1)/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/memleak_check $(1)/usr/bin/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libmemleak_check.so $(1)/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/memleak_check $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
