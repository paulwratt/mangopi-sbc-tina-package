#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pcl-msgs
PKG_VERSION:=indigo

PKG_SOURCE:=pcl_msgs-$(PKG_VERSION)-devel.zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/pcl_msgs-$(PKG_VERSION)-devel

PKG_BUILD_DEPENDS:= common_msgs std_msgs

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-3rdparty
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+common_msgs +std_msgs
  TITLE:=ROS package containing PCL-related messages
  URL:=https://github.com/ros-perception/pcl_msgs/tree/indigo-devel
endef

define Package/$(PKG_NAME)/description
ROS package containing PCL-related messages.
endef

pcl_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/msg/ModelCoefficients.msg \
	$(PKG_BUILD_DIR)/msg/PointIndices.msg \
	$(PKG_BUILD_DIR)/msg/PolygonMesh.msg \
	$(PKG_BUILD_DIR)/msg/Vertices.msg

pcl_srvs_MSGFILES:=  \
	$(PKG_BUILD_DIR)/srv/UpdateFilename.srv

define Build/Compile
	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach pmsg,$(pcl_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,pcl_msgs,include/pcl_msgs, \
			pcl_msgs:$(PKG_BUILD_DIR)/msg -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg \
			-I sensor_msgs:$(STAGING_DIR)/usr/share/sensor_msgs/msg \
			-I geometry_msgs:$(STAGING_DIR)/geometry_msgs/msg, $(pmsg),,))

	cd $(PKG_BUILD_DIR) && \
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(STAGING_DIR)/usr/bin/catkin_make_isolated --install \
		--source=$(PKG_BUILD_DIR) \
		--build=$(PKG_BUILD_DIR)/build_isolated \
		-DCATKIN_DEVEL_PREFIX=$(PKG_BUILD_DIR)/devel \
		-DCATKIN_ENABLE_TESTING=OFF \
		-DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR) \
		-DCMAKE_C_COMPILER="$(TARGET_CC)" \
		-DCMAKE_CXX_COMPILER="$(TARGET_CXX)" \
		-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/usr;$(PKG_INSTALL_DIR)"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
