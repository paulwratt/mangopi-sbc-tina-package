include $(TOPDIR)/rules.mk

PKG_NAME:=libc-testsuite
PKG_VERSION:=2013-04-22
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=4dbca0ae3eafb9e21f2cdb2305395e7976feacb4
PKG_SOURCE_URL:=git://git.musl-libc.org/$(PKG_NAME)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_MD5SUM:=492fdcacf63c47808bb9c11afef8a163

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	URL:=https://git.musl-libc.org/cgit/libc-testsuite
	TITLE:=unit tests for C/POSIX standard library
	DEPENDS:= +libc +librt +libpthread +libm
endef

define Package/$(PKG_NAME)/description
	This package is a collection of unit tests to measure the correctness
	and robustness of a C/POSIX standard library implementation. It was
	developed as a tool for testing musl and its first-generation
	predecessor. As such, most of the tests were written to be
	self-contained and not to make assumptions about other library
	interfaces being present or working correctly.
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
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/testsuite $(1)/usr/bin/testsuite
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
