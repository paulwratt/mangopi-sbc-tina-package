include $(TOPDIR)/rules.mk

PKG_NAME:=ceres-solver
PKG_VERSION:=1.14.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=Apache License 2.0

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

PKG_BUILD_DEPENDS:= OpenBLAS lapack SuiteSparse eigen gflags glog

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+OpenBLAS +lapack +SuiteSparse +eigen +gflags +glog
  TITLE:=A large scale non-linear optimization library
  URL:=https://github.com/ceres-solver/ceres-solver/tree/1.14.0
endef

define Package/$(PKG_NAME)/description
Ceres Solver is an open source C++ library for modeling and solving large, complicated optimization problems.
endef

CMAKE_OPTIONS += -DBUILD_TESTING=OFF
CMAKE_OPTIONS += -DBUILD_EXAMPLES=OFF
CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=ON

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
