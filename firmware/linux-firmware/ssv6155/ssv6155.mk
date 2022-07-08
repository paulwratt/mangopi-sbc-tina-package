Package/ssv6155-firmware = $(call Package/firmware-default,ssv6155 jixian firmware)
define Package/ssv6155-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ssv6155/ssv6155-wifi.cfg \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ssv6155/ssv6x5x-wifi.cfg \
		$(1)/$(FIRMWARE_PATH)/
endef
$(eval $(call BuildPackage,ssv6155-firmware))
