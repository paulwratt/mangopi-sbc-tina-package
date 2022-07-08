include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk
include $(BUILD_DIR)/package.mk

PKG_NAME:=smartva_ota
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

PKG_BUILD_DEPENDS:=PACKAGE_tslib:tslib
define Package/$(PKG_NAME)
  SECTION:=gui
  SUBMENU:=Littlevgl
  CATEGORY:=Gui
  DEPENDS:= +libfreetype +swupdate +blkid +libfreetype +PACKAGE_tslib:tslib
  TITLE:=$(PKG_NAME) for pdc-pd1 use lvgl-6.1.2
  MAINTAINER:=shiqingli <shiqingli@allwinnertech.com>
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  $(PKG_NAME) for pdc-pd1
endef

define Package/$(PKG_NAME)/config
  source "$(SOURCE)/Config.in"
endef

ifeq ($(CONFIG_lvgl),y)
CONFIGURE_PRJ += ./../lvgl
endif

ifeq ($(CONFIG_lv_drivers),y)
CONFIGURE_PRJ += ./../lv_drivers
endif

ifeq ($(CONFIG_lv_examples),y)
CONFIGURE_PRJ += ./../lv_examples
endif

CONFIGURE_PRJ += ./src/common

#ifeq ($(CONFIG_lv_story_ota),y)
CONFIGURE_PRJ += ./src/story
CONFIGURE_APP += ./src/story
CONFIGURE_BIN += story_ota_bin
#endif

ifeq ($(TARGET_PRODUCT), c200s_F1C200s)
LOCAL_CFLAGS += -D__SUNXI_DISPLAY__
LOCAL_CFLAGS += -DCONFIG_C200S
LOCAL_CFLAGS += -I$(LINUX_DIR)/include/video
endif

ifeq ($(TARGET_PRODUCT), f133_evb1)
LOCAL_CFLAGS += -D__SUNXI_DISPLAY2__
LOCAL_CFLAGS += -DCONFIG_F133
LOCAL_CFLAGS += -I$(LINUX_DIR)/include/video
endif

ifeq ($(TARGET_PRODUCT), t113_evb1)
LOCAL_CFLAGS += -D__SUNXI_DISPLAY2__
LOCAL_CFLAGS += -DCONFIG_T113
LOCAL_CFLAGS += -I$(LINUX_DIR)/include/video
endif
ifeq ($(CONFIG_RTP_USE),y)
TARGET_LDFLAGS += -lts
LOCAL_CFLAGS += -D__RTP_USE__
endif

ifeq ($(CONFIG_CTP_USE), y)
LOCAL_CFLAGS += -D__CTP_USE__
endif

LOCAL_CFLAGS+=-I$(STAGING_DIR)/usr/include/freetype2

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)_real/src
	$(CP) -u $(CONFIGURE_PRJ)/ $(PKG_BUILD_DIR)_real/src/
	$(CP) -u $(CONFIGURE_APP)/config/* $(PKG_BUILD_DIR)_real/src/
	$(CP) -u $(CONFIGURE_APP)/Makefile $(PKG_BUILD_DIR)_real/src/
	ln -sf $(PKG_BUILD_DIR)_real/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LOCAL_CFLAGS="$(LOCAL_CFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/res/
	$(INSTALL_DIR) $(1)/usr/res/image/
	$(INSTALL_DIR) $(1)/usr/res/font/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(CONFIGURE_BIN) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/story/designer/image/res/* $(1)/usr/res/image/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/story/designer/font/res/* $(1)/usr/res/font//
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
