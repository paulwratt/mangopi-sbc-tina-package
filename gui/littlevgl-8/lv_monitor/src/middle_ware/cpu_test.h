/**
 * @file cpu_test.h
 *
 */

#ifndef CPU_TEST_H
#define CPU_TEST_H

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>

#define PATH_TO_CPU "/sys/devices/system/cpu/"
#define PATH_TO_THERMAL "/sys/class/thermal/"
#define SYSFS_PATH_MAX	255
#define MAX_LINE_LEN	255
#define CPU_NRS_MAX     (8)
#define KHZ_TO_GHZ 1000000
#define MEM_TEST_SIZE (1024*1024)
#define DDR_INFO_NUM  23

#undef  u8
#define u8     unsigned char
#undef  s8
#define s8     char
#undef  u32
#define u32    unsigned int
#undef  s32
#define s32    int

struct cpu_stat {
    u32 id;
    u32 utime, ntime, stime, itime;
    u32 iowtime, irqtime, sirqtime;
    u32 totalcpuTime;
    u32 totalbusyTime;
};

struct cpu_usage {
    struct cpu_stat stats[CPU_NRS_MAX + 1];
    u32 active;
};

struct cpu_load {
    u32 mem_size;
    u32 load;
    u32 *mem;
};

struct ddr_info {
    u32 use;
    u32 type;
    u32 value;
};

struct ddr_info ddr_info[DDR_INFO_NUM];
pthread_t ddr_info_thread;

enum cpufreq_value {
    AFFECTED_CPUS = 0,
    CPUINFO_BOOT_FREQ,
    CPUINFO_BURST_FREQ,
    CPUINFO_CUR_FREQ,
    CPUINFO_MAX_FREQ,
    CPUINFO_MIN_FREQ,
    CPUINFO_LATENCY,
    SCALING_CUR_FREQ,
    SCALING_MIN_FREQ,
    SCALING_MAX_FREQ,
    STATS_NUM_TRANSITIONS,
    MAX_CPUFREQ_VALUE_READ_FILES
};

enum thermal_type {
    CPU_THERMAL = 0, GPU_THERMAL, DDR_THERMAL,
};

static const char *cpufreq_value_files[MAX_CPUFREQ_VALUE_READ_FILES] = {
    [AFFECTED_CPUS]     = "affected_cpus",
    [CPUINFO_BOOT_FREQ] = "cpuinfo_boot_freq",
    [CPUINFO_BURST_FREQ]="cpuinfo_burst_freq",
    [CPUINFO_CUR_FREQ] = "cpuinfo_cur_freq",
    [CPUINFO_MAX_FREQ] = "cpuinfo_max_freq",
    [CPUINFO_MIN_FREQ] = "cpuinfo_min_freq",
    [CPUINFO_LATENCY]  = "cpuinfo_transition_latency",
    [SCALING_CUR_FREQ] = "scaling_cur_freq",
    [SCALING_MIN_FREQ] = "scaling_min_freq",
    [SCALING_MAX_FREQ] = "scaling_max_freq",
    [STATS_NUM_TRANSITIONS] = "stats/total_trans"
};

u32 sysfs_cpufreq_get_one_value(u32 cpu, enum cpufreq_value which);

u32 sysfs_thermal_get_one_value(enum thermal_type type);

u32 monitor_get_cpu_info(char *cpu_online, char *cpu_freq,
        u32 cpu_usages[CPU_NRS_MAX], u32 *cpu_online_num, u32 *cpu_usages_all);

u32 monitor_add_cpu_load(u32 load, u32 cpu_online_num, u32 cpu_usages_all);

u32 monitor_add_ddr_load();
u32 monitor_add_ddr_cancel();

#endif
