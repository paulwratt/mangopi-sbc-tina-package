include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=miracast2
PKG_VERSION:=1
PKG_RELEASE:=1

SRC_CODE_DIR := $(TOPDIR)/external/miracast2
PKG_FILE_DEPENDS := $(SRC_CODE_DIR)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
	SUBMENU:=Homlet
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=Miracast Receiver for Tina Linux
	DEPENDS:=+cedar +libubox +libuci +libstdcpp
endef

define Package/$(PKG_NAME)/description
	MiracastReceiver for Tina Linux
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	echo "Compile do nothing, just install OK."
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc
	cp -rf $(PKG_BUILD_DIR)/bin/miracast2 $(1)/usr/bin/
	cp -rf $(PKG_BUILD_DIR)/bin/p2p_action.sh $(1)/usr/bin/
	cp -rf $(PKG_BUILD_DIR)/etc/p2p_supplicant_gc.conf $(1)/etc/
	
	echo "-----1---$(1)--------"
	cp -rf $(PKG_BUILD_DIR)/lib/*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
