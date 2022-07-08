define Package/r528-dsp-firmware
    KCONFIG:= \
		CONFIG_REMOTEPROC=y \
   		CONFIG_SUNXI_RPROC=y \
		CONFIG_VIRTIO_BLK=n \
		CONFIG_VIRTIO_CONSOLE=n \
		CONFIG_CRYPTO_DEV_VIRTIO=n \
	$(call Package/firmware-default,R528 dsp firmware)
endef

define Package/r528-dsp-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(TOPDIR)/device/config/chips/$(TARGET_PLATFORM)/bin/dsp0.bin $(1)/$(FIRMWARE_PATH)/rproc-sunxi-rproc-fw
endef
$(eval $(call BuildPackage,r528-dsp-firmware))
