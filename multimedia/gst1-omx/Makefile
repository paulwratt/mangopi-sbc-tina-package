#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gst-omx
PKG_VERSION:=1.16.3
PKG_RELEASE:=1

PKG_SOURCE:=gst-omx-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://gstreamer.freedesktop.org/src/gst-omx/
PKG_MD5SUM:=d4d89dd44362c1d262186c60437cdbee

include $(BUILD_DIR)/package.mk

define Package/gst-omx
	SECTION:=multimedia
	CATEGORY:=Multimedia
	TITLE:=gst openmax lib
	DEPENDS:=+gstreamer1 +libgst1allocators +libgst1video +libgst1audio +libgst1pbutils +libcedarx +PACKAGE_libgst1gl:libgst1gl
endef

define Package/gst-omx/description
 GStreamer openmax API
endef

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM), r16 r311 r58 r40 g102 r11 r7 r528))
	OMX_COMPILE_TOOL_CHAIN = arm-openwrt-linux
endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM), r18 mr813 r818))
ifeq ($(CONFIG_COMPLILE_KERNEL64_USER32),y)
	OMX_COMPILE_TOOL_CHAIN = arm-openwrt-linux
else
	OMX_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),d1)
	OMX_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
endif

EXTRA_LDFLAGS+= \
	-Wl,-rpath-link=$(STAGING_DIR)/rootfs/lib \

define Build/Configure
	cd $(PKG_BUILD_DIR);./autogen.sh --host=$(OMX_COMPILE_TOOL_CHAIN) --prefix=$(PKG_BUILD_DIR) --target=arm-openwrt-linux --disable-gtk-doc --with-omx-target=generic
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)"
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)" \
		install
endef

define Package/gst-omx/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) -rf $(PKG_BUILD_DIR)/lib/gstreamer-1.0 $(1)/usr/lib
	mkdir -p $(1)/etc/xdg
	$(CP) $(PKG_BUILD_DIR)/config/bellagio/gstomx.conf $(1)/etc/xdg/
endef

$(eval $(call BuildPackage,gst-omx))
