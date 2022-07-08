Package/uwe5622-firmware = $(call Package/firmware-default,uwe5622 firmware)

define Package/uwe5622-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DIR) $(1)/vendor/etc
	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/uwe5622/wcnmodem.bin $(1)/$(FIRMWARE_PATH)/wcnmodem.bin
	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/uwe5622/wifi_56630001_3ant.ini $(1)/vendor/etc/wifi_56630001_3ant.ini
endef
$(eval $(call BuildPackage,uwe5622-firmware))
