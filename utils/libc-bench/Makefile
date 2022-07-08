include $(TOPDIR)/rules.mk

PKG_NAME:=libc-bench
PKG_VERSION:=2014-04-03
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=b6b2ce5f9f87a09b14499cb00c600c601f022634
PKG_SOURCE_URL:=git://git.musl-libc.org/$(PKG_NAME)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_MD5SUM:=85897d334b5811f28f7a63aad1fcb646

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	URL:=https://git.musl-libc.org/cgit/libc-bench
	TITLE:=Performance tests for C/POSIX standard library
	DEPENDS:= +libc +librt +libpthread +libm
endef

define Package/$(PKG_NAME)/description
A performance and memory usage benchmark for comparing libc implementations
It is provided by musl-libc.org
endef

TARGET_CFLAGS+=-Wno-format-extra-args

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libc-bench $(1)/usr/bin/libc-bench
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
