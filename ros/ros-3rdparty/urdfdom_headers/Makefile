include $(TOPDIR)/rules.mk

PKG_NAME:=urdfdom_headers
PKG_VERSION:=master

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SUBMENU:=ros-3rdparty
  SECTION:=ROS
  CATEGORY:=ROS
  TITLE:=Headers for URDF parsers
  URL:=https://github.com/ros/urdfdom_headers.git
endef

define Package/$(PKG_NAME)/description
The URDF (U-Robot Description Format) headers provides core data structure headers for URDF.
For now, the details of the URDF specifications reside on http://ros.org/wiki/urdf
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
