include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=awcast
PKG_VERSION:=1
PKG_RELEASE:=1

SRC_CODE_DIR := $(TOPDIR)/external/awcast
PKG_FILE_DEPENDS := $(SRC_CODE_DIR)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
	SUBMENU:=Homlet
	SECTION:=gui
	CATEGORY:=Allwinner
	TITLE:=Awcast Main Program Demo (with MiniGUI)
	DEPENDS:=+libminigui-gpl +libpthread +libjpeg +libpng +libm +libfreetype +libz +libpng12 +libfreetype +libpthread +libbz2 \
		+libubus +libuci +libjson-c +ubus +libxml2 +cedar +miracast2
endef

define Package/$(PKG_NAME)/description
	Awcast Main Program Demo (with MiniGUI)
endef

define Package/$(PKG_NAME)/config
config AWCAST_DLNA_ENABLE
	bool "Enable dlna function"
	depends on PACKAGE_awcast
	default y

config AWCAST_MIRACAST_ENABLE
	bool "Enable miracast function"
	depends on PACKAGE_awcast && PACKAGE_miracast2
	default y
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ${SRC_CODE_DIR}/src/* $(PKG_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/build

	cd $(PKG_BUILD_DIR)/build && cmake ..
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	$(CP) $(PKG_BUILD_DIR)/build/tinyui/*.so $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tinyui/tinyui.h  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/build
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/res
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) -r ${SRC_CODE_DIR}/res/* $(1)/usr/res
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/tinyui/libtinyui.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dlna/lib/*.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/awcast $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/awcast.init $(1)/etc/init.d/awcast
	#$(INSTALL_DATA) $(SRC_CODE_DIR)/conf/awcast.cfg $(1)/usr/local/etc/awcast.cfg
	$(INSTALL_DATA) $(SRC_CODE_DIR)/conf/MiniGUI.cfg $(1)/usr/local/etc/MiniGUI.cfg
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
