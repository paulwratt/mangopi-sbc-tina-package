# auto-generated TEE configuration file
# TEE version e4202bb-dirty
set(CFG_8250_UART y)
set(CFG_AES_GCM_TABLE_BASED y)
set(CFG_ARCH_SUN50IW10P1 y)
set(CFG_ARCH_SUNXI y)
set(CFG_ARM32_core y)
set(CFG_ARM32_ldelf y)
set(CFG_ARM32_ta_arm32 y)
# CFG_BOOT_SECONDARY_REQUEST is not set
set(CFG_CACHE_API y)
set(CFG_CC_OPTIMIZE_FOR_SIZE y)
# CFG_CORE_BGET_BESTFIT is not set
set(CFG_CORE_BIGNUM_MAX_BITS 4096)
set(CFG_CORE_CLUSTER_SHIFT 2)
# CFG_CORE_DUMP_OOM is not set
set(CFG_CORE_DYN_SHM y)
set(CFG_CORE_HEAP_SIZE 65536)
set(CFG_CORE_HUK_SUBKEY_COMPAT y)
# CFG_CORE_LARGE_PHYS_ADDR is not set
set(CFG_CORE_MBEDTLS_MPI y)
set(CFG_CORE_NEX_HEAP_SIZE 16384)
set(CFG_CORE_RESERVED_SHM y)
# CFG_CORE_RODATA_NOEXEC is not set
set(CFG_CORE_RWDATA_NOEXEC y)
# CFG_CORE_SANITIZE_KADDRESS is not set
# CFG_CORE_SANITIZE_UNDEFINED is not set
set(CFG_CORE_TZSRAM_EMUL_SIZE 458752)
set(CFG_CORE_UNMAP_CORE_AT_EL0 y)
set(CFG_CORE_WORKAROUND_NSITR_CACHE_PRIME y)
set(CFG_CORE_WORKAROUND_SPECTRE_BP y)
set(CFG_CORE_WORKAROUND_SPECTRE_BP_SEC y)
set(CFG_CRYPTO y)
set(CFG_CRYPTOLIB_DIR core/lib/libtomcrypt)
set(CFG_CRYPTOLIB_NAME tomcrypt)
set(CFG_CRYPTOLIB_NAME_tomcrypt y)
set(CFG_CRYPTO_AES y)
# CFG_CRYPTO_AES_GCM_FROM_CRYPTOLIB is not set
set(CFG_CRYPTO_CBC y)
set(CFG_CRYPTO_CBC_MAC y)
set(CFG_CRYPTO_CCM y)
set(CFG_CRYPTO_CMAC y)
set(CFG_CRYPTO_CONCAT_KDF y)
set(CFG_CRYPTO_CTR y)
set(CFG_CRYPTO_CTS y)
set(CFG_CRYPTO_DES y)
set(CFG_CRYPTO_DH y)
set(CFG_CRYPTO_DSA y)
set(CFG_CRYPTO_ECB y)
set(CFG_CRYPTO_ECC y)
set(CFG_CRYPTO_GCM y)
set(CFG_CRYPTO_HKDF y)
set(CFG_CRYPTO_HMAC y)
set(CFG_CRYPTO_MD5 y)
set(CFG_CRYPTO_PBKDF2 y)
set(CFG_CRYPTO_RSA y)
set(CFG_CRYPTO_RSASSA_NA1 y)
set(CFG_CRYPTO_SHA1 y)
set(CFG_CRYPTO_SHA224 y)
set(CFG_CRYPTO_SHA256 y)
set(CFG_CRYPTO_SHA384 y)
set(CFG_CRYPTO_SHA512 y)
set(CFG_CRYPTO_SHA512_256 y)
set(CFG_CRYPTO_SIZE_OPTIMIZATION y)
set(CFG_CRYPTO_XTS y)
set(CFG_DEBUG_INFO y)
set(CFG_DEVICE_ENUM_PTA y)
# CFG_DT is not set
set(CFG_DTB_MAX_SIZE 0x10000)
# CFG_EARLY_TA is not set
# CFG_EMBED_DTB is not set
# CFG_ENABLE_SCTLR_RR is not set
# CFG_ENABLE_SCTLR_Z is not set
# CFG_EXTERNAL_DTB_OVERLAY is not set
set(CFG_FTRACE_US_MS 10000)
set(CFG_GENERIC_BOOT y)
set(CFG_GIC y)
set(CFG_GP_SOCKETS y)
set(CFG_HWSUPP_MEM_PERM_PXN y)
set(CFG_HWSUPP_MEM_PERM_WXN y)
set(CFG_KERN_LINKER_ARCH arm)
set(CFG_KERN_LINKER_FORMAT elf32-littlearm)
set(CFG_LIBUTILS_WITH_ISOC y)
# CFG_LOCKDEP is not set
set(CFG_LPAE_ADDR_SPACE_SIZE (1ull << 32))
set(CFG_LTC_OPTEE_THREAD y)
set(CFG_MMAP_REGIONS 13)
set(CFG_MSG_LONG_PREFIX_MASK 0x1a)
set(CFG_NUM_THREADS 4)
set(CFG_OPTEE_REVISION_MAJOR 3)
set(CFG_OPTEE_REVISION_MINOR 7)
set(CFG_OS_REV_REPORTS_GIT_SHA1 y)
# CFG_PAGED_USER_TA is not set
set(CFG_PM_STUBS y)
set(CFG_REE_FS y)
set(CFG_REE_FS_TA y)
# CFG_REE_FS_TA_BUFFERED is not set
set(CFG_RESERVED_VASPACE_SIZE (1024 * 1024 * 10))
set(CFG_RPMB_FS y)
set(CFG_RPMB_FS_DEV_ID 0)
# CFG_RPMB_WRITE_KEY is not set
# CFG_SCTLR_ALIGNMENT_CHECK is not set
set(CFG_SECSTOR_TA y)
set(CFG_SECSTOR_TA_MGMT_PTA y)
# CFG_SECURE_DATA_PATH is not set
set(CFG_SECURE_TIME_SOURCE_CNTPCT y)
# CFG_SHOW_CONF_ON_BOOT is not set
set(CFG_SM_NO_CYCLE_COUNTING y)
set(CFG_SUNXI_CE21 y)
set(CFG_SUNXI_EFUSE_ARCH64 y)
set(CFG_SUNXI_HASH_CHECK y)
set(CFG_SUNXI_NCAT y)
set(CFG_SUNXI_SIGN_KEY_V2 y)
set(CFG_SUNXI_SMC_30 y)
# CFG_SYSCALL_WRAPPERS_MCOUNT is not set
set(CFG_SYSTEM_PTA y)
# CFG_TA_ASLR is not set
set(CFG_TA_ASLR_MAX_OFFSET_PAGES 128)
set(CFG_TA_ASLR_MIN_OFFSET_PAGES 0)
set(CFG_TA_BIGNUM_MAX_BITS 2048)
set(CFG_TA_DYNLINK y)
# CFG_TA_FLOAT_SUPPORT is not set
# CFG_TA_FTRACE_SUPPORT is not set
# CFG_TA_GPROF_SUPPORT is not set
set(CFG_TA_MBEDTLS y)
set(CFG_TA_MBEDTLS_MPI y)
set(CFG_TA_MBEDTLS_SELF_TEST y)
set(CFG_TEE_API_VERSION GPD-1.1-dev)
set(CFG_TEE_CORE_DEBUG y)
set(CFG_TEE_CORE_EMBED_INTERNAL_TESTS y)
set(CFG_TEE_CORE_LOG_LEVEL 1)
# CFG_TEE_CORE_MALLOC_DEBUG is not set
set(CFG_TEE_CORE_TA_TRACE y)
set(CFG_TEE_FW_IMPL_VERSION FW_IMPL_UNDEF)
set(CFG_TEE_FW_MANUFACTURER FW_MAN_UNDEF)
set(CFG_TEE_IMPL_DESCR OPTEE)
set(CFG_TEE_MANUFACTURER LINARO)
set(CFG_TEE_TA_LOG_LEVEL 1)
# CFG_TEE_TA_MALLOC_DEBUG is not set
# CFG_ULIBS_MCOUNT is not set
# CFG_ULIBS_SHARED is not set
set(CFG_UNWIND y)
# CFG_VIRTUALIZATION is not set
set(CFG_WITH_ARM_TRUSTED_FW y)
# CFG_WITH_PAGER is not set
set(CFG_WITH_SOFTWARE_PRNG y)
set(CFG_WITH_STACK_CANARIES y)
set(CFG_WITH_USER_TA y)
set(PLATFORM_FLAVOR sun50iw10)
set(PLATFORM_FLAVOR_sun50iw10 y)
set(PLATFORM_sun50iw10p1 y)
set(_CFG_CORE_LTC_ACIPHER y)
set(_CFG_CORE_LTC_AES y)
# _CFG_CORE_LTC_AES_ARM32_CE is not set
# _CFG_CORE_LTC_AES_ARM64_CE is not set
set(_CFG_CORE_LTC_ASN1 y)
set(_CFG_CORE_LTC_AUTHENC y)
set(_CFG_CORE_LTC_BIGNUM_MAX_BITS 4096)
set(_CFG_CORE_LTC_CBC y)
set(_CFG_CORE_LTC_CBC_MAC y)
set(_CFG_CORE_LTC_CCM y)
# _CFG_CORE_LTC_CE is not set
set(_CFG_CORE_LTC_CIPHER y)
set(_CFG_CORE_LTC_CMAC y)
set(_CFG_CORE_LTC_CTR y)
set(_CFG_CORE_LTC_CTS y)
set(_CFG_CORE_LTC_DES y)
set(_CFG_CORE_LTC_DH y)
set(_CFG_CORE_LTC_DSA y)
set(_CFG_CORE_LTC_ECB y)
set(_CFG_CORE_LTC_ECC y)
set(_CFG_CORE_LTC_HASH y)
set(_CFG_CORE_LTC_HMAC y)
# _CFG_CORE_LTC_HWSUPP_PMULL is not set
set(_CFG_CORE_LTC_MAC y)
set(_CFG_CORE_LTC_MD5 y)
set(_CFG_CORE_LTC_MPI y)
set(_CFG_CORE_LTC_OPTEE_THREAD y)
# _CFG_CORE_LTC_PAGER is not set
set(_CFG_CORE_LTC_RSA y)
set(_CFG_CORE_LTC_SHA1 y)
# _CFG_CORE_LTC_SHA1_ARM32_CE is not set
# _CFG_CORE_LTC_SHA1_ARM64_CE is not set
set(_CFG_CORE_LTC_SHA224 y)
set(_CFG_CORE_LTC_SHA256 y)
# _CFG_CORE_LTC_SHA256_ARM32_CE is not set
# _CFG_CORE_LTC_SHA256_ARM64_CE is not set
set(_CFG_CORE_LTC_SHA384 y)
set(_CFG_CORE_LTC_SHA512 y)
set(_CFG_CORE_LTC_SHA512_256 y)
set(_CFG_CORE_LTC_SIZE_OPTIMIZATION y)
# _CFG_CORE_LTC_VFP is not set
set(_CFG_CORE_LTC_XTS y)
