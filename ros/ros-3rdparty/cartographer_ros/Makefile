include $(TOPDIR)/rules.mk

PKG_NAME:=cartographer_ros
PKG_VERSION:=1.0.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:= boost-iostreams pcl catkin/host cartographer geometry perception_pcl urdf abseil-cpp

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-3rdparty
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+boost-iostreams +pcl +catkin +cartographer +geometry +perception_pcl +urdf +abseil-cpp
  TITLE:=Provides ROS integration for Cartographer
  URL:=https://github.com/googlecartographer/cartographer_ros
endef

define Package/$(PKG_NAME)/description
Cartographer is a system that provides real-time simultaneous localization and
mapping (SLAM) in 2D and 3D across multiple platforms and sensor configurations.
This project providesCartographer's ROS integration.
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread) -DTIXML_USE_STL -I$(PKG_INSTALL_DIR)/include -ltinyxml
TARGET_CXXFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread) -DTIXML_USE_STL -ltinyxml

CMAKE_OPTIONS += -DCMAKE_FIND_ROOT_PATH="$(STAGING_DIR)/usr;$(TOOLCHAIN_DIR)"

cartographer_ros_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/LandmarkEntry.msg \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/LandmarkList.msg \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/SensorTopics.msg \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/StatusCode.msg \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/StatusResponse.msg \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/SubmapEntry.msg \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/SubmapList.msg \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/SubmapTexture.msg \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg/TrajectoryOptions.msg

cartographer_ros_msgs_SRVFILES:= \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/srv/FinishTrajectory.srv \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/srv/StartTrajectory.srv \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/srv/SubmapQuery.srv \
	$(PKG_BUILD_DIR)/cartographer_ros_msgs/srv/WriteState.srv

define Build/Compile
	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach pmsg,$(cartographer_ros_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,cartographer_ros_msgs,include/cartographer_ros_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I geometry_msgs:$(STAGING_DIR)/usr/share/geometry_msgs/msg -I cartographer_ros_msgs:$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg, $(pmsg),,))
	$(foreach psrv,$(cartographer_ros_msgs_SRVFILES),$(call Build/Compile/MsgSrvToH,,cartographer_ros_msgs,include/cartographer_ros_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I geometry_msgs:$(STAGING_DIR)/usr/share/geometry_msgs/msg -I cartographer_ros_msgs:$(PKG_BUILD_DIR)/cartographer_ros_msgs/msg -I cartographer_ros_msgs:$(PKG_BUILD_DIR)/cartographer_ros_msgs/srv, $(psrv),,))

	cd $(PKG_BUILD_DIR) && \
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CXXFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(STAGING_DIR)/usr/bin/catkin_make_isolated --install \
		--source=$(PKG_BUILD_DIR) \
		--build=$(PKG_BUILD_DIR)/build_isolated \
		-DCATKIN_DEVEL_PREFIX=$(PKG_BUILD_DIR)/devel \
		-DCATKIN_ENABLE_TESTING=OFF \
		-DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR) \
		-DCMAKE_C_COMPILER="$(TARGET_CC)" \
		-DCMAKE_CXX_COMPILER="$(TARGET_CXX)" \
		-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/usr;$(PKG_INSTALL_DIR)" \
		--cmake-args $(CMAKE_OPTIONS)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
