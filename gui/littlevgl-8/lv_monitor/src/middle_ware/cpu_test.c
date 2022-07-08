#include "cpu_test.h"

static u32 sysfs_read_file(const s8 *path, s8 *buf, size_t buflen) {
    int fd;
    ssize_t numread;

    fd = open(path, O_RDONLY);
    if (fd == -1)
        return 0;

    numread = read(fd, buf, buflen - 1);
    if (numread < 1) {
        close(fd);
        return 0;
    }

    buf[numread] = '\0';
    close(fd);

    return (u32) numread;
}

static u32 sysfs_cpufreq_read_file(u32 cpu, const s8 *fname,
s8 *buf, size_t buflen) {
    s8 path[SYSFS_PATH_MAX];

    snprintf(path, sizeof(path), PATH_TO_CPU "cpu%u/cpufreq/%s", cpu, fname);
    return sysfs_read_file(path, buf, buflen);
}

static u32 sysfs_thermal_read_file(enum thermal_type type, s8 *buf,
        size_t buflen) {
    s8 path[SYSFS_PATH_MAX];

    snprintf(path, sizeof(path), PATH_TO_THERMAL "thermal_zone%d/temp", type);
    return sysfs_read_file(path, buf, buflen);
}

u32 sysfs_thermal_get_one_value(enum thermal_type type) {
    u32 value;
    u32 len;
    s8 linebuf[MAX_LINE_LEN];
    s8 *endp;

    len = sysfs_thermal_read_file(type, linebuf, sizeof(linebuf));

    if (len == 0)
        return 0;

    value = strtoul(linebuf, &endp, 0);

    if (endp == linebuf || errno == ERANGE)
        return 0;

    return value / 1000;
}

u32 sysfs_cpufreq_get_one_value(u32 cpu, enum cpufreq_value which) {
    u32 value;
    u32 len;
    s8 linebuf[MAX_LINE_LEN];
    s8 *endp;

    if (which >= MAX_CPUFREQ_VALUE_READ_FILES)
        return 0;

    len = sysfs_cpufreq_read_file(cpu, cpufreq_value_files[which], linebuf,
            sizeof(linebuf));

    if (len == 0)
        return 0;

    value = strtoul(linebuf, &endp, 0);

    if (endp == linebuf || errno == ERANGE)
        return 0;

    return value;
}

u32 find_cpu_online(u32 cpu) {
    s8 path[SYSFS_PATH_MAX];
    s8 linebuf[MAX_LINE_LEN];
    s8 *endp;
    u32 value;
    u32 len;

    snprintf(path, sizeof(path), PATH_TO_CPU "cpu%u/online", cpu);

    len = sysfs_read_file(path, linebuf, sizeof(linebuf));
    if (len == 0) {
		/* must have one core */
		if (cpu == 0) {
			return 1;
		}
	    return 0;
	}
    value = strtoul(linebuf, &endp, 0);

    if (endp == linebuf || errno == ERANGE)
        return 0;

    return value;
}

s32 read_proc_precpu_stat(struct cpu_usage *usages) {
    FILE *file;
    char buf[100];
    s32 i = 0;
    s32 item = 0;
    struct cpu_stat cur_cpu;
    if (!usages)
        return -1;

    file = fopen("/proc/stat", "r");
    if (!file) {
        printf("Could not open /proc/stat.\n");
        return -1;
    }

    fseek(file, 0, SEEK_SET);
    while ((fgets(buf, 100, file) != NULL) && (!strncmp(buf, "cpu", 3))) {
        item = 0;
        memset(&cur_cpu, 0, sizeof(cur_cpu));

        if (i == 0) {
            if (sscanf(buf, "cpu  %d %d %d %d %d %d %d", &cur_cpu.utime,
                    &cur_cpu.ntime, &cur_cpu.stime, &cur_cpu.itime,
                    &cur_cpu.iowtime, &cur_cpu.irqtime, &cur_cpu.sirqtime)
                    < 0) {
                fclose(file);
                return -1;
            }
            item = 8;
            i++;
        } else {
            if (sscanf(buf, "cpu%d  %d %d %d %d %d %d %d", &cur_cpu.id,
                    &cur_cpu.utime, &cur_cpu.ntime, &cur_cpu.stime,
                    &cur_cpu.itime, &cur_cpu.iowtime, &cur_cpu.irqtime,
                    &cur_cpu.sirqtime) < 0) {
                fclose(file);
                return -1;
            }
            item = cur_cpu.id;
        }
        cur_cpu.totalbusyTime = cur_cpu.utime + cur_cpu.stime + cur_cpu.ntime
                + cur_cpu.irqtime + cur_cpu.sirqtime;

        cur_cpu.totalcpuTime = cur_cpu.totalbusyTime + cur_cpu.itime
                + cur_cpu.iowtime;

        usages->active |= (1UL << item);
        memcpy(&usages->stats[item], &cur_cpu, sizeof(cur_cpu));
    }
    fclose(file);
    return 0;
}

u32 monitor_get_cpu_info(char *cpu_online, char *cpu_freq,
        u32 cpu_usages[CPU_NRS_MAX], u32 *cpu_online_num, u32 *cpu_usages_all) {
    static struct cpu_usage pre;
    struct cpu_usage cur;
    u32 i, ret;
    char cpu_freq_temp[MAX_LINE_LEN] = { '\0' };
    char cpu_online_temp[MAX_LINE_LEN] = { "CPU" };
    u32 total, busy = 0;

    memset(&cur, 0, sizeof(cur));
    ret = read_proc_precpu_stat(&cur);
    if (ret < 0) {
        printf("CPU read precpu stat err!\n");
        return -1;
    }

    for (i = 0; i < CPU_NRS_MAX; i++) {
        if (find_cpu_online(i)) {
            (*cpu_online_num)++;
            sprintf(cpu_freq, "%s%1.1f ", cpu_freq_temp,
                    (float) sysfs_cpufreq_get_one_value(i,
                            CPUINFO_CUR_FREQ)/KHZ_TO_GHZ);
            memcpy(cpu_freq_temp, cpu_freq, MAX_LINE_LEN);
            sprintf(cpu_online, "%s %d", cpu_online_temp, i);
            memcpy(cpu_online_temp, cpu_online, MAX_LINE_LEN);

            if (pre.stats[i].totalcpuTime && pre.stats[i].totalbusyTime) {
                //cpu use
                total = cur.stats[i].totalcpuTime - pre.stats[i].totalcpuTime;
                busy = cur.stats[i].totalbusyTime - pre.stats[i].totalbusyTime;

                if ((busy > 0) && (total > 0) && (total >= busy)) {
                    cpu_usages[i] = busy * 100 / total;
                } else {
                    cpu_usages[i] = 0;
                }

                *cpu_usages_all += cpu_usages[i];
            }
            pre.stats[i].totalcpuTime = cur.stats[i].totalcpuTime;
            pre.stats[i].totalbusyTime = cur.stats[i].totalbusyTime;
        } else
            cpu_usages[i] = 0;
    }

    return 0;
}

void* add_cpu_load(void *arg) {
    struct timeval tv_start;
    struct timeval tv_now;
    u32 busy, load, idle, size;
    struct cpu_load *cpu_load;
    u32 *mem;

    cpu_load = (struct cpu_load*) arg;
    load = cpu_load->load;
    size = cpu_load->mem_size;
    mem = (u32*) cpu_load->mem;
    idle = 100 - load;

    while (1) {
        /* cpu load first idle */
        if (load < 100)
            usleep(idle * 10);
        /* cpu load second busy */
        gettimeofday(&tv_start, NULL);
        while (1) {
            memset(mem, busy, size);
            gettimeofday(&tv_now, NULL);
            busy = tv_now.tv_sec * 1000000 + tv_now.tv_usec
                    - tv_start.tv_sec * 1000000 - tv_start.tv_usec;
            if (busy > (load * 10))
                break;
        }
        /* if cancel should exit */
        pthread_testcancel();
    }

}

u32 monitor_add_cpu_load(u32 load, u32 cpu_online_num, u32 cpu_usages_all) {
    static u32 load_pre = 0;
    static u32 thread_state[CPU_NRS_MAX];
    static struct cpu_load cpu_load[CPU_NRS_MAX];
    static pthread_t thread[CPU_NRS_MAX];
    cpu_set_t cpuset;
    u32 i;

    if (load_pre != load) {
        for (i = 0; i < CPU_NRS_MAX; i++) {
            if (find_cpu_online(i)) {
                if (thread_state[i]) {
                    pthread_cancel(thread[i]);
                    pthread_join(thread[i], NULL);
                    thread_state[i] = 0;
                    if (cpu_load[i].mem != NULL) {
                        free(cpu_load[i].mem);
                    }
                    cpu_load[i].mem_size = 0;
                }

                if (load * cpu_online_num > cpu_usages_all) {
                    cpu_load[i].mem_size = MEM_TEST_SIZE;
                    cpu_load[i].mem = (u32*) malloc(MEM_TEST_SIZE);
                    cpu_load[i].load = load;

                    pthread_create(&thread[i], NULL, add_cpu_load,
                            &cpu_load[i]);
                    CPU_ZERO(&cpuset);
                    CPU_SET(i, &cpuset);
                    pthread_setaffinity_np(thread[i], sizeof(cpu_set_t),
                            &cpuset);
                    thread_state[i] = 1;
                    load_pre = load;
                }
            }
        }
    }
    return 0;
}

void* get_ddr_info(void *arg) {
    FILE *fp;
    char buf[256] = { 0 };
    char command[32] = "mtop -n 1";
    /* de_ddr can replace de0_ddr */
    char *name[DDR_INFO_NUM] = { "totddr", "riscv_sys", "cpuddr", "ce_ddr",
            "csi_ddr", "isp_ddr", "de0_ddr", "de1_ddr", "di_ddr", "dma_ddr",
            "iommu_ddr", "mahb_ddr", "tvd_ddr", "ve_ddr", "dsp_sys", "g2d_ddr",
            "gpuddr", "ndfc_ddr", "r_dma_ddr", "aipu_ddr", "eink_ddr", "othddr",
            "de_ddr" };
    char *q = NULL, *value = NULL, *ddr_name = NULL;
    int i, is_totddr, ddr_name_num;

    while (1) {
        i = 0;
        is_totddr = 0;
        ddr_name_num = 0;
        if ((fp = popen(command, "r")) != NULL) {
            while (fgets(buf, sizeof(buf), fp) != NULL) {
                if (is_totddr) {
                    is_totddr = 0;
                    value = strtok(buf, " ");
                    i = 0;
                    while (value != NULL) {
                        /* atoi will increase a lot of cpu occupancy */
                        ddr_info[i++].value = atoi(value);
                        value = strtok(NULL, " ");
                    }
                    break;
                }

                /* Judgment is totddr riscv_sys  ce_ddr csi_ddr  de_ddr.. */
                q = strstr(buf, "totddr");
                if (q != NULL) {
                    q = NULL;
                    is_totddr = 1;

                    /* Only need to be parsed once */
                    if (ddr_info[0].use == 0) {
                        value = strtok(buf, " ");

                        while (value != NULL) {
                            /* The last string has a newline */
                            if ((ddr_name = strstr(value, "\n")))
                                *ddr_name = '\0';

                            for (i = 0; i < DDR_INFO_NUM; i++) {
                                if (strcmp(value, name[i]) == 0) {
                                    ddr_info[ddr_name_num].use = 1;
                                    if (i != 22)
                                        ddr_info[ddr_name_num].type = i;
                                    else
                                        ddr_info[ddr_name_num].type = 6;
                                    ddr_name_num++;
                                    break;
                                }
                            }

                            value = strtok(NULL, " ");
                        }
                    }
                }
            }
            pclose(fp);
        }
        usleep(500 * 1000);

        /* if cancel should exit */
        pthread_testcancel();
    }
}

u32 monitor_add_ddr_load() {
    memset(&ddr_info, 0, sizeof(struct ddr_info) * (DDR_INFO_NUM - 1));
    pthread_create(&ddr_info_thread, NULL, get_ddr_info, NULL);
    return 0;
}

u32 monitor_add_ddr_cancel() {
    pthread_cancel(ddr_info_thread);
    pthread_join(ddr_info_thread, NULL);
    return 0;
}
