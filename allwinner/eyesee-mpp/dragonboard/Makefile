##############################################
# OpenWrt Makefile for eyesee-mpp-framework
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=eyesee-mpp-dragonboard
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=AW-PDC-PD5
# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
SRC_COMPILE_DIR := $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
SRC_CODE_DIR := $(TOPDIR)/external/eyesee-mpp/dragonboard

# This specifies the directory where file depends. when files in it update, increasing compile will be enable.
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)

include $(BUILD_DIR)/package.mk

############################################################################
#below items are used for mpp configuration of previous EyeseeLinux,
#for smooth migration to tina, we reserve these config items, define them here and export.
#ref to mpp_config.mk of EyeseeLinux.
#[config item], valid in makefile.
#config chip version: AW1721
ifeq ($(TARGET_CHIP), sun8iw12p1)
    MPPCFG_CHIP := AW1721
else ifeq ($(TARGET_CHIP), sun8iw16p1)
    MPPCFG_CHIP := AW1816
else ifeq ($(TARGET_CHIP), sun8iw21p1)
    MPPCFG_CHIP := AW1886
else
    $(error fatal error! $(TARGET_CHIP) is not support by eyesee-mpp!)
endif
export MPPCFG_CHIP
#config write file method: LINUX_VFS, DIRECT_IO
ifeq ($(CONFIG_mpp_fwrite_method_vfs),y)
    MPPCFG_FILE_SYSTEM := LINUX_VFS
else ifeq ($(CONFIG_mpp_fwrite_method_directIO),y)
    MPPCFG_FILE_SYSTEM := DIRECT_IO
else
    MPPCFG_FILE_SYSTEM := LINUX_VFS
endif
export MPPCFG_FILE_SYSTEM
#config if use hw display method: true, false
ifeq ($(CONFIG_mpp_hw_display),y)
    MPPCFG_HW_DISPLAY := true
else
    MPPCFG_HW_DISPLAY := false
endif
export MPPCFG_HW_DISPLAY
#config if enable mpp_vi
ifeq ($(CONFIG_mpp_vi),y)
    MPPCFG_VI := Y
else
    MPPCFG_VI := N
endif
export MPPCFG_VI
#config if enable mpp_vo
ifeq ($(CONFIG_mpp_vo),y)
    MPPCFG_VO := Y
else
    MPPCFG_VO := N
endif
export MPPCFG_VO
#config if enable mpp_textEnc
ifeq ($(CONFIG_mpp_textenc),y)
    MPPCFG_TEXTENC := Y
else
    MPPCFG_TEXTENC := N
endif
export MPPCFG_TEXTENC
#config if enable mpp_venc
ifeq ($(CONFIG_mpp_venc),y)
    MPPCFG_VENC := Y
else
    MPPCFG_VENC := N
endif
export MPPCFG_VENC
#config if enable mpp_vdec
ifeq ($(CONFIG_mpp_vdec),y)
    MPPCFG_VDEC := Y
else
    MPPCFG_VDEC := N
endif
export MPPCFG_VDEC
#config if enable mpp_aio
ifeq ($(CONFIG_mpp_aio),y)
    MPPCFG_AIO := Y
else
    MPPCFG_AIO := N
endif
export MPPCFG_AIO
#config if enable mpp_aenc
ifeq ($(CONFIG_mpp_aenc),y)
    MPPCFG_AENC := Y
else
    MPPCFG_AENC := N
endif
export MPPCFG_AENC
ifeq ($(CONFIG_mpp_aenc_pcm),y)
    MPPCFG_AENC_PCM := Y
else
    MPPCFG_AENC_PCM := N
endif
export MPPCFG_AENC_PCM
ifeq ($(CONFIG_mpp_aenc_g711),y)
    MPPCFG_AENC_G711 := Y
else
    MPPCFG_AENC_G711 := N
endif
export MPPCFG_AENC_G711
ifeq ($(CONFIG_mpp_aenc_g726),y)
    MPPCFG_AENC_G726 := Y
else
    MPPCFG_AENC_G726 := N
endif
export MPPCFG_AENC_G726
ifeq ($(CONFIG_mpp_aenc_aac),y)
    MPPCFG_AENC_AAC := Y
else
    MPPCFG_AENC_AAC := N
endif
export MPPCFG_AENC_AAC
ifeq ($(CONFIG_mpp_aenc_mp3),y)
    MPPCFG_AENC_MP3 := Y
else
    MPPCFG_AENC_MP3 := N
endif
export MPPCFG_AENC_MP3
#config if enable mpp_adec
ifeq ($(CONFIG_mpp_adec),y)
    MPPCFG_ADEC := Y
else
    MPPCFG_ADEC := N
endif
export MPPCFG_ADEC
#config if enable mpp_muxer
ifeq ($(CONFIG_mpp_muxer),y)
    MPPCFG_MUXER := Y
else
    MPPCFG_MUXER := N
endif
export MPPCFG_MUXER
#config gps data pack method
ifeq ($(CONFIG_mpp_gps_pack_in_track),y)
    MPPCFG_GPS_PACK_IN_TRACK := Y
else
    MPPCFG_GPS_PACK_IN_TRACK := N
endif
export MPPCFG_GPS_PACK_IN_TRACK
ifeq ($(CONFIG_mpp_gps_pack_in_mdat),y)
    MPPCFG_GPS_PACK_IN_MDAT := Y
else
    MPPCFG_GPS_PACK_IN_MDAT := N
endif
export MPPCFG_GPS_PACK_IN_MDAT
#config if enable mpp_demuxer
ifeq ($(CONFIG_mpp_demuxer),y)
    MPPCFG_DEMUXER := Y
else
    MPPCFG_DEMUXER := N
endif
export MPPCFG_DEMUXER
#config if include libmpp_ise.so and enable ise function.
ifeq ($(CONFIG_mpp_ise),y)
    MPPCFG_ISE := Y
else
    MPPCFG_ISE := N
endif
export MPPCFG_ISE
#config if include libmpp_ise.so and enable ise mo module.
ifeq ($(CONFIG_mpp_ise_mo),y)
    MPPCFG_ISE_MO := Y
else
    MPPCFG_ISE_MO := N
endif
export MPPCFG_ISE_MO
#config if include libmpp_ise.so and enable ise gdc module.
ifeq ($(CONFIG_mpp_ise_gdc),y)
    MPPCFG_ISE_GDC := Y
else
    MPPCFG_ISE_GDC := N
endif
export MPPCFG_ISE_GDC
#config if include libmpp_ise.so and enable ise bi module.
ifeq ($(CONFIG_mpp_ise_bi),y)
    MPPCFG_ISE_BI := Y
else
    MPPCFG_ISE_BI := N
endif
export MPPCFG_ISE_BI
#config if include libmpp_ise.so and enable ise bi soft module.
ifeq ($(CONFIG_mpp_ise_bi_soft),y)
    MPPCFG_ISE_BI_SOFT := Y
else
    MPPCFG_ISE_BI_SOFT := N
endif
export MPPCFG_ISE_BI_SOFT
#config if include libmpp_ise.so and enable ise sti module.
ifeq ($(CONFIG_mpp_ise_sti),y)
    MPPCFG_ISE_STI := Y
else
    MPPCFG_ISE_STI := N
endif
export MPPCFG_ISE_STI
#config if include libmpp_eis.so and enable eis function.(Electronic Image Stabilization)
ifeq ($(CONFIG_mpp_eis),y)
    MPPCFG_EIS := Y
else
    MPPCFG_EIS := N
endif
export MPPCFG_EIS
#config if include libai_MOD.so and enable MOD(Motion Object Detection) detect.
ifeq ($(CONFIG_mpp_mod),y)
    MPPCFG_MOD := Y
else
    MPPCFG_MOD := N
endif
export MPPCFG_MOD
#config if include libeve_face.so and enable face detect.
ifeq ($(CONFIG_mpp_eveface),y)
    MPPCFG_EVEFACE := Y
else
    MPPCFG_EVEFACE := N
endif
export MPPCFG_EVEFACE
#config if include libai_VLPR.so and enable VLPR(Vehicle License Plate Recognition) detect.
ifeq ($(CONFIG_mpp_vlpr),y)
    MPPCFG_VLPR := Y
else
    MPPCFG_VLPR := N
endif
export MPPCFG_VLPR
#config if include libawmd.so and enable motion  detect.
ifeq ($(CONFIG_mpp_motion_detect_soft),y)
    MPPCFG_MOTION_DETECT_SOFT := Y
else
    MPPCFG_MOTION_DETECT_SOFT := N
endif
export MPPCFG_MOTION_DETECT_SOFT
#config if include libadas.so and enable adas  detect.
ifeq ($(CONFIG_mpp_adas_detect),y)
    MPPCFG_ADAS_DETECT := Y
else
    MPPCFG_ADAS_DETECT := N
endif
export MPPCFG_ADAS_DETECT
#config if include libadas-v2.so and enable adas-v2  detect.
ifeq ($(CONFIG_mpp_adas_detect_v2),y)
    MPPCFG_ADAS_DETECT_V2 := Y
else
    MPPCFG_ADAS_DETECT_V2 := N
endif
export MPPCFG_ADAS_DETECT_V2
#config if include libai_HCNT.so and enable HCNT(human count) detect.
ifeq ($(CONFIG_mpp_hcnt),y)
    MPPCFG_HCNT := Y
else
    MPPCFG_HCNT := N
endif
export MPPCFG_HCNT
#config if include libai_BDII.so and enable BDII() detect.
ifeq ($(CONFIG_mpp_bdii),y)
    MPPCFG_BDII := Y
else
    MPPCFG_BDII := N
endif
export MPPCFG_BDII
#config if include libeve_face.so and enable face detect.
ifeq ($(CONFIG_mpp_eveface),y)
    MPPCFG_EVEFACE := Y
else
    MPPCFG_EVEFACE := N
endif
export MPPCFG_EVEFACE
ifeq ($(CONFIG_mpp_body_detect),y)
    MPPCFG_BODY_DETECT := Y
else
    MPPCFG_BODY_DETECT := N
endif
export MPPCFG_BODY_DETECT
#config if enable mpp_aec
ifeq ($(CONFIG_mpp_aec),y)
    MPPCFG_AEC := Y
else
    MPPCFG_AEC := N
endif
export MPPCFG_AEC
#config if enable mpp_softdrc
ifeq ($(CONFIG_mpp_softdrc),y)
    MPPCFG_SOFTDRC := Y
else
    MPPCFG_SOFTDRC := N
endif
export MPPCFG_SOFTDRC
#config if enable mpp_agc
ifeq ($(CONFIG_mpp_agc),y)
    MPPCFG_AGC := Y
else
    MPPCFG_AGC := N
endif
export MPPCFG_AGC

ifeq ($(CONFIG_mpp_ai_agc),y)
    MPPCFG_AI_AGC := Y
else
    MPPCFG_AI_AGC := N
endif
export MPPCFG_AI_AGC

#config if enable mpp_ans
ifeq ($(CONFIG_mpp_ans),y)
    MPPCFG_ANS := Y
else
    MPPCFG_ANS := N
endif
export MPPCFG_ANS

#config ans library of webrtc if enable mpp_ans_libwebrtc
ifeq ($(CONFIG_mpp_ans_libwebrtc),y)
    MPPCFG_ANS_LIB := libwebrtc
endif

#config ans library of lstm if enable mpp_ans_liblstm
ifeq ($(CONFIG_mpp_ans_liblstm),y)
    MPPCFG_ANS_LIB := liblstm
endif
export MPPCFG_ANS_LIB

#config if include libmpp_uvc.so
ifeq ($(CONFIG_mpp_uvc),y)
    MPPCFG_UVC := Y
else
    MPPCFG_UVC := N
endif
export MPPCFG_UVC
#prebuild all AI libs and component by static
ifeq ($(CONFIG_mpp_compile_static_lib),y)
    MPPCFG_COMPILE_STATIC_LIB := Y
else
    MPPCFG_COMPILE_STATIC_LIB := N
endif
export MPPCFG_COMPILE_STATIC_LIB
#build component to dynamic so if possible
ifeq ($(CONFIG_mpp_compile_dynamic_lib),y)
    MPPCFG_COMPILE_DYNAMIC_LIB := Y
else
    MPPCFG_COMPILE_DYNAMIC_LIB := N
endif
export MPPCFG_COMPILE_DYNAMIC_LIB

############################################################################
#below items are used for mpp custom_aw configuration
#config in eyesee-mpp-middleware, custom_aw alse need to export.
ifeq ($(CONFIG_imx317),y)
    SENSOR_NAME:=imx317
else ifeq ($(CONFIG_imx278),y)
    SENSOR_NAME:=imx278
else ifeq ($(CONFIG_imx386),y)
    SENSOR_NAME:=imx386
else
    SENSOR_NAME:=
endif
export SENSOR_NAME
#export BOARD_TYPE by TARGET_PRODUCT.
#ifneq (,$(findstring $(strip $(TARGET_PRODUCT)),v316_perfnor v316_sdv))
ifeq ("$(strip $(TARGET_PRODUCT))", "v316_perfnor")
    BOARD_TYPE := PER1
else ifeq ("$(strip $(TARGET_PRODUCT))", "v316_sdv")
    BOARD_TYPE := PRO
else ifeq ("$(strip $(TARGET_PRODUCT))", "v536_cdr")
    BOARD_TYPE := C26A
else ifeq ("$(strip $(TARGET_PRODUCT))", "v833_perf1")
    BOARD_TYPE := PER1
else ifeq ("$(strip $(TARGET_PRODUCT))", "v853_perf1")
    BOARD_TYPE := PER1
else
    $(warning "ERROR: TARGET_PRODUCT:$(TARGET_PRODUCT) is unknown, default config will be used")
    BOARD_TYPE := PER1
endif
export BOARD_TYPE
#config if boot mode is normal or fast
export BOARD_BOOT_TYPE="normal"

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SECTION:=eyesee-mpp-dragonboard
  CATEGORY:=Allwinner
  SUBMENU:=eyesee-mpp
  TITLE:=eyesee-mpp-dragonboard for Tina Linux
  DEPENDS:=+eyesee-mpp-external +eyesee-mpp-middleware +eyesee-mpp-framework +libjpeg +libpng +eyesee-minigui +tslib +libopenssl
endef

define Package/$(PKG_NAME)/config
#source "$(SOURCE)/Config.in"
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
  eyesee-mpp-dragonboard is our solutions for CDR dragonboard. It mainly use eyesee-mpp-framework to implement
  multimedia function.
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		rm -rf $(SRC_COMPILE_DIR); \
	fi
	ln -s $(SRC_CODE_DIR) $(SRC_COMPILE_DIR)
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(SRC_COMPILE_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef

define Build/Clean
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		MPPCFG_COMPILE_DYNAMIC_LIB=Y \
		MPPCFG_COMPILE_STATIC_LIB=Y \
		$(MAKE) -C $(SRC_COMPILE_DIR) \
			ARCH="$(TARGET_ARCH)" \
			AR="$(TARGET_AR)" \
			CC="$(TARGET_CC)" \
			CXX="$(TARGET_CXX)" \
			CFLAGS="$(TARGET_CFLAGS)" \
			CXXFLAGS="$(TARGET_CXXFLAGS)" \
			CPPFLAGS="$(TARGET_CPPFLAGS)" \
			LDFLAGS="$(TARGET_LDFLAGS)" \
			clean; \
	else \
		echo "$(SRC_COMPILE_DIR) is not exist!"; \
	fi
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/eyesee-mpp
	$(INSTALL_DIR) $(1)/usr/include/eyesee-mpp/dragonboard
	$(CP) -r $(SRC_COMPILE_DIR)/include $(1)/usr/include/eyesee-mpp/dragonboard
	#$(CP) $(SRC_COMPILE_DIR)/apps/cdr/webserver/libwebserver.a $(1)/usr/lib/eyesee-mpp
endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one


# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/eyesee-mpp
	$(INSTALL_DIR) $(1)/usr/bin
	#find $(SRC_COMPILE_DIR)/lib/out -maxdepth 1 \( -name "libminigui_ths*.so*" -o -name "*.so" \) -exec $(CP) {} $(1)/usr/lib/eyesee-mpp \;

	$(INSTALL_DIR) $(1)/etc
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/config/out/etc/MiniGUI.cfg $(1)/etc/MiniGUI.cfg
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/config/out/etc/sunxi-keyboard.kl $(1)/etc/sunxi-keyboard.kl
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/config/out/etc/ts.conf $(1)/etc/ts.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/config/out/etc/init.d/S01app $(1)/etc/init.d/S01app
	$(INSTALL_DIR) $(1)/usr/share/res/audio
	$(INSTALL_DIR) $(1)/usr/share/res/video
	$(INSTALL_DIR) $(1)/usr/share/res/picture
	$(INSTALL_DIR) $(1)/usr/share/minigui/res
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/res/out/startup.wav $(1)/usr/share/res/audio
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/res/out/test.mp4 $(1)/usr/share/res/video
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/res/out/bike_480x320_220.bin $(1)/usr/share/res/picture
	$(CP) -r $(SRC_COMPILE_DIR)/apps/DragonBoard/res/out/font $(1)/usr/share/minigui/res
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/core/dragonboard $(1)/usr/bin
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/core/test_config.fex $(1)/usr/bin

	# the following system and peripheral
	$(INSTALL_DIR) $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/rtctester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/ledtester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/usbtester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/ddrtester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/tftester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/nortester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/nandtester $(1)/usr/bin/testcase

	# the following mpp test cases run on sd card.
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/csitester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/csitester.conf $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/mictester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/mictester.conf $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/venctester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/venctester.conf $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/vdectester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/vdectester.conf $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/isptester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/g2dtester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/g2dtester.conf $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/cetester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/cetester.conf $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/votester $(1)/usr/bin/testcase
	$(CP) $(SRC_COMPILE_DIR)/apps/DragonBoard/bin/votester.conf $(1)/usr/bin/testcase
endef

#define Package/$(PKG_NAME)/InstallDev
#endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
