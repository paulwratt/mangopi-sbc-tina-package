#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/signalfd.h>
#include <pthread.h>
#include "bt_dev_list.h"
#include "bt_log.h"
#include "bt_test.h"
#include "dirent.h"

static void cmd_bt_set_log_level(int argc, char *args[])
{
    int value = 0;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    value = atoi(args[0]);

    bt_manager_set_loglevel((btmg_log_level_t)value);
}

static void cmd_bt_enable(int argc, char *args[])
{
    int ret = BT_OK;
    int value = 0;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    value = atoi(args[0]);
    if (value)
        ret = bt_manager_enable(true);
    else
        ret = bt_manager_enable(false);

    if (ret < 0)
        BTMG_ERROR("error:%s", bt_manager_get_error_info(ret));
}

static void cmd_bt_scan(int argc, char *args[])
{
    int value = 0;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    btmg_scan_filter_t scan_filter = { 0 };
    scan_filter.type = BTMG_SCAN_BR_EDR;
    scan_filter.rssi = -90;
    value = atoi(args[0]);

    if (value) {
        bt_manager_scan_filter(&scan_filter);
        bt_manager_start_scan();
    } else
        bt_manager_stop_scan();
}

static void cmd_bt_pair(int argc, char *args[])
{
    int ret = BT_OK;

    if (argc != 1 && strlen(args[0]) != 17) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    ret = bt_manager_pair(args[0]);
    if (ret < 0)
        BTMG_ERROR("Pair failed:%s,error:%s", args[0], bt_manager_get_error_info(ret));
}

static void cmd_bt_unpair(int argc, char *args[])
{
    if (argc != 1 && strlen(args[0]) != 17) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    if (bt_manager_unpair(args[0]))
        BTMG_ERROR("Pair failed:%s", args[0]);
}

static void cmd_bt_inquiry_list(int argc, char *args[])
{
    dev_node_t *dev_node = NULL;

    dev_node = discovered_devices->head;
    while (dev_node != NULL) {
        BTMG_INFO("addr: %s, name: %s", dev_node->dev_addr, dev_node->dev_name);
        dev_node = dev_node->next;
    }
}

static void cmd_bt_pair_devices_list(int argc, char *args[])
{
    int i = 0;
    int count = -1;
    btmg_bt_device_t *devices = NULL;

    bt_manager_get_paired_devices(&devices, &count);
    if (count > 0) {
        for (i = 0; i < count; i++) {
            printf("\n \
				address:%s \n \
				name:%s \n \
				paired:%d \n \
				connected:%d\n",
                   devices[i].remote_address, devices[i].remote_name, devices[i].paired,
                   devices[i].connected);
        }
    } else {
        BTMG_ERROR("paired device is empty");
        return;
    }
    bt_manager_free_paired_devices(devices, count);
}

static void cmd_bt_get_adapter_state(int argc, char *args[])
{
    btmg_adapter_state_t bt_state;
    bt_state = bt_manager_get_adapter_state();
    if (bt_state == BTMG_ADAPTER_OFF)
        BTMG_INFO("The BT is OFF");
    else if (bt_state == BTMG_ADAPTER_ON)
        BTMG_INFO("The BT is ON");
    else if (bt_state == BTMG_ADAPTER_TURNING_ON)
        BTMG_INFO("The BT is turning ON");
    else if (bt_state == BTMG_ADAPTER_TURNING_OFF)
        BTMG_INFO("The BT is turning OFF");
}

static void cmd_bt_get_adapter_name(int argc, char *args[])
{
    char bt_name[128] = { 0 };

    bt_manager_get_adapter_name(bt_name);

    BTMG_INFO("bt adater name: %s", bt_name);
}

static void cmd_bt_set_adapter_name(int argc, char *args[])
{
    int ret = -1;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    if (bt_manager_set_adapter_name(args[0])) {
        BTMG_ERROR("Set bt name failed.");
    }
}

static void cmd_bt_get_device_name(int argc, char *args[])
{
    int ret = BT_OK;
    char remote_name[128] = { 0 };

    if (argc != 1 && strlen(args[0]) != 17) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    ret = bt_manager_get_device_name(args[0], remote_name);
    if (ret) {
        BTMG_ERROR("get bt remote name fail.");
    } else {
        BTMG_INFO("get bt remote name:%s", remote_name);
    }
}

static void cmd_bt_get_adapter_address(int argc, char *args[])
{
    int ret = BT_OK;
    char bt_addr[18] = { 0 };

    ret = bt_manager_get_adapter_address(bt_addr);
    if (ret) {
        BTMG_ERROR("get bt address failed.");
    } else {
        BTMG_INFO("BT address: %s", bt_addr);
    }
}

static void cmd_bt_avrcp(int argc, char *args[])
{
    int avrcp_type = -1;
    char bt_addr[18] = { 0 };
    int i;
    int ret;
    const char *cmd_type_str[] = {
        "play", "pause", "stop", "fastforward", "rewind", "forward", "backward",
    };
    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    for (i = 0; i < sizeof(cmd_type_str) / sizeof(char *); i++) {
        if (strcmp(args[0], cmd_type_str[i]) == 0) {
            avrcp_type = i;
            break;
        }
    }

    if (avrcp_type == -1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    ret = bt_manager_get_adapter_address(bt_addr);
    if (ret) {
        BTMG_ERROR("get bt address failed.");
        return;
    } else {
        BTMG_INFO("BT address:%s", bt_addr);
    }

    bt_manager_avrcp_command(bt_addr, (btmg_avrcp_command_t)avrcp_type);
}

static void cmd_bt_device_connect(int argc, char *args[])
{
    int ret = BT_OK;

    if (argc != 1 && strlen(args[0]) != 17) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    ret = bt_manager_connect(args[0]);

    if (ret < 0)
        BTMG_ERROR("error:%s", bt_manager_get_error_info(ret));
}

static void cmd_bt_device_disconnect(int argc, char *args[])
{
    int ret = BT_OK;

    if (argc != 1 && strlen(args[0]) != 17) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    ret = bt_manager_disconnect(args[0]);

    if (ret < 0)
        BTMG_ERROR("error:%s", bt_manager_get_error_info(ret));
}

static void cmd_bt_remove_device(int argc, char *args[])
{
    if (argc != 1 && strlen(args[0]) != 17) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    bt_manager_remove_device(args[0]);
}

typedef struct {
    unsigned int riff_type;
    unsigned int riff_size;
    unsigned int wave_type;
    unsigned int format_type;
    unsigned int format_size;
    unsigned short compression_code;
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int bytes_per_second;
    unsigned short block_align;
    unsigned short bits_per_sample;
    unsigned int data_type;
    unsigned int data_size;
} wav_header_t;

typedef struct {
    char music_path[128];
    int music_number;
} wav_musiclist_t;

typedef struct {
    char type;
    char parameter[128];
} a2dp_src_run_args_t;

static bool a2dp_src_loop = false;
static bool a2dp_src_enable = false;
static bool a2dp_src_playstate_play = false;
static bool a2dp_src_playstate_pause = false;
static pthread_t bt_test_a2dp_source_thread;
static a2dp_src_run_args_t *a2dp_src_run_args = NULL;
wav_musiclist_t *a2dp_source_musiclist = NULL;
static int a2dp_source_musiclist_number = 1;

#define A2DP_SRC_BUFF_SIZE (1024 * 4)

static int _a2dp_source_traverse_musiclist(char *foldpath)
{
    char file_format[8] = { 0 };
    DIR *record_dir = NULL;
    struct dirent *de = NULL;
    FILE *file = NULL;
    int file_count = 0;

    record_dir = opendir(foldpath);
    if (record_dir == NULL) {
        BTMG_ERROR("Path OPEN error");
        return -1;
    }

    if (a2dp_source_musiclist != NULL) {
        free(a2dp_source_musiclist);
    }
    a2dp_source_musiclist_number = 1;
    a2dp_source_musiclist = (wav_musiclist_t *)malloc(1 * sizeof(wav_musiclist_t));

    while ((de = readdir(record_dir)) != 0) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        } else if (de->d_type == 8) { /* file */
            int filelen = strlen(de->d_name);
            memset(file_format, '\0', sizeof(file_format));
            strncpy(file_format, de->d_name + filelen - 3, 3); /* 记录文件格式 */
            if (!strcmp("wav", file_format)) {
                wav_musiclist_t *ml = &a2dp_source_musiclist[a2dp_source_musiclist_number - 1];
                if (foldpath[strlen(foldpath) - 1] != '/')
                    sprintf(ml->music_path, "%s/%s", foldpath, de->d_name);
                else
                    sprintf(ml->music_path, "%s%s", foldpath, de->d_name);

                ml->music_number = a2dp_source_musiclist_number;
                a2dp_source_musiclist_number++;
                BTMG_DEBUG("find fire path : %s", ml->music_path);

                wav_musiclist_t * new_musiclist;
                new_musiclist = (wav_musiclist_t *)realloc(a2dp_source_musiclist,
                                                                   a2dp_source_musiclist_number *
                                                                           sizeof(wav_musiclist_t));
                if (new_musiclist == NULL) {
                    BTMG_ERROR("realloc fail");
                    return -1;
                }
                else {
                    a2dp_source_musiclist = new_musiclist;
                }
            }
        }
    }
    return 0;
}

static void *_a2dp_source_thread_func(void *arg)
{
    char muisc_path[256] = { 0 };
    int path_length = 0;
    int ret = -1, len = 0, fd = -1;
    char buffer[A2DP_SRC_BUFF_SIZE] = { 0 };
    unsigned int c = 0, written = 0, count = 0;
    a2dp_src_run_args_t *a2dp_source_thread_arg = (a2dp_src_run_args_t *)arg;
    wav_header_t wav_header;
    static int wav_number = 0;
    char type = a2dp_source_thread_arg->type;
    if (type == 'f') {
        strcpy(muisc_path, a2dp_source_thread_arg->parameter);
    } else if (type == 'p') {
        ret = _a2dp_source_traverse_musiclist(a2dp_source_thread_arg->parameter);
        if (ret == BT_ERROR) {
            BTMG_ERROR("_a2dp_source_thread_func EMD");
            pthread_exit((void *)-1);
            ;
        }
        strcpy(muisc_path, (const char *)&a2dp_source_musiclist[wav_number].music_path);
    }
start:
    len = 0, c = 0, written = 0, count = 0;
    memset(buffer, 0, sizeof(buffer));
    path_length = strlen(muisc_path);
    if (path_length < 5) { //File path meets at least length 5
        BTMG_ERROR("Please enter the correct file path");
        a2dp_src_enable = false;
        pthread_exit((void *)-1);
    }
    if (strcmp(".wav", &muisc_path[path_length - 4])) {
        BTMG_ERROR("Please enter the correct audio format - 'wav' ");
        a2dp_src_enable = false;
        pthread_exit((void *)-1);
    }

    fd = open(muisc_path, O_RDONLY); // "/mnt/44100-stereo-s16_le-10s.wav"
    if (fd < 0) {
        BTMG_ERROR("Cannot open input file:%s", strerror(errno));
        a2dp_src_enable = false;
        pthread_exit((void *)-1);
    }
    ret = read(fd, &wav_header, sizeof(wav_header_t));
    if (ret != sizeof(wav_header_t)) {
        BTMG_ERROR("read wav file header failed.");
        close(fd);
        pthread_exit((void *)-1);
    }
    if (bt_manager_a2dp_src_init(wav_header.num_channels, wav_header.sample_rate) != 0) {
        BTMG_ERROR("a2dp source init error");
        close(fd);
        pthread_exit((void *)-1);
    }

    ret = -1;
    a2dp_src_enable = true;
    a2dp_src_loop = true;
    count = wav_header.data_size;
    bt_manager_set_avrcp_status(BTMG_AVRCP_PLAYSTATE_PLAYING);
    bt_manager_a2dp_src_stream_start(A2DP_SRC_BUFF_SIZE);
    BTMG_INFO("start a2dp src loop, data size:%d, ch:%d, sample:%d ,path: %s", wav_header.data_size,
              wav_header.num_channels, wav_header.sample_rate, muisc_path);

    while (a2dp_src_loop) {
        btmg_avrcp_play_state_t status = bt_manager_get_avrcp_status();
        switch (status) {
        case BTMG_AVRCP_PLAYSTATE_PLAYING:
            if (a2dp_src_playstate_pause) {
                ret = -1;
                a2dp_src_playstate_pause = false;
                bt_manager_a2dp_src_stream_start(A2DP_SRC_BUFF_SIZE);
            }
            if (ret != 0) {
                c = count - written;
                if (c > A2DP_SRC_BUFF_SIZE) {
                    c = A2DP_SRC_BUFF_SIZE;
                }
                len = read(fd, buffer, c);
                if (len == 0) {
                    lseek(fd, 0, SEEK_SET);
                    written = 0;
                    continue;
                }
                if (len < 0) {
                    BTMG_ERROR("read file error,ret:%d,c=%d", len, c);
                    break;
                }
            } else {
                usleep(1000 * 20);
            }
            if (len > 0) {
                ret = bt_manager_a2dp_src_stream_send(buffer, len);
                written += ret;
            }
            break;
        case BTMG_AVRCP_PLAYSTATE_PAUSED:
            if (!a2dp_src_playstate_pause) {
                bt_manager_a2dp_src_stream_stop(false);
                a2dp_src_playstate_pause = true;
            }
            usleep(10 * 1000);
            break;

        case BTMG_AVRCP_PLAYSTATE_FORWARD:
            if (!a2dp_src_playstate_pause) {
                bt_manager_a2dp_src_stream_stop(false);
                bt_manager_a2dp_src_deinit();
                a2dp_src_playstate_pause = true;
            }
            if (type == 'p' || type == 'P') {
                wav_number++;
                if (wav_number >= a2dp_source_musiclist_number - 1) {
                    wav_number = 0;
                }
                memset(muisc_path, 0, sizeof(muisc_path));
                strcpy(muisc_path, (const char *)&a2dp_source_musiclist[wav_number].music_path);
            }
            close(fd);
            a2dp_src_loop = false;
            goto start;
            break;
        case BTMG_AVRCP_PLAYSTATE_BACKWARD:
            if (!a2dp_src_playstate_pause) {
                bt_manager_a2dp_src_stream_stop(false);
                bt_manager_a2dp_src_deinit();
                a2dp_src_playstate_pause = true;
            }
            if (type == 'p' || type == 'P') {
                wav_number--;
                if (wav_number < 0) {
                    wav_number = a2dp_source_musiclist_number - 1 - 1;
                }
                memset(muisc_path, 0, sizeof(muisc_path));
                strcpy(muisc_path, (const char *)&a2dp_source_musiclist[wav_number].music_path);
            }
            close(fd);
            a2dp_src_loop = false;
            goto start;
            break;

        default:
            break;
        }
    }
    if (fd != -1) {
        close(fd);
    }

    bt_manager_a2dp_src_stream_stop(false);
    bt_manager_a2dp_src_deinit();
    a2dp_src_enable = false;

    BTMG_INFO("a2dp source play thread quit");

    return NULL;
}

static void cmd_bt_a2dp_src_run(int argc, char *args[])
{
    if (a2dp_src_enable == true) {
        BTMG_INFO("a2dp source play thread already started");
        return;
    }
    if (a2dp_src_loop == false) {
#if 0
		// int i = 0, opt = 0;
		// const char *optstring = "F:f:P:p:";//fire or path
		// char **a2dp_src_args_list = (char **)malloc( 10 * sizeof(a2dp_src_args_list) );
		// for (i = 0; i < argc; i++){
      	// 	a2dp_src_args_list[i + 1] = args[i];
		// 	BTMG_ERROR("a2dp_src_args_list[%d] = %s",i+1, a2dp_src_args_list[i+1]);
   		// }
		// while ((opt = getopt(argc + 1, a2dp_src_args_list, optstring)) != -1){
        // 	printf("opt = %c\t\t", opt);
       	// 	printf("optarg = %s\t\t",optarg);
        // 	printf("optind = %d\t\t",optind);
        // 	printf("argv[optind] = %s\n",a2dp_src_args_list[optind]);
		// }
#endif
        if (a2dp_src_run_args)
            free(a2dp_src_run_args);
        a2dp_src_run_args = (a2dp_src_run_args_t *)malloc(sizeof(a2dp_src_run_args_t));
        if (strcmp("-f", args[0]) == 0 || strcmp("-F", args[0]) == 0) {
            a2dp_src_run_args->type = 'f';
            memset(a2dp_src_run_args->parameter, 0, sizeof(a2dp_src_run_args->parameter));
            strcpy(a2dp_src_run_args->parameter, args[1]);
        } else if (strcmp("-p", args[0]) == 0 || strcmp("-P", args[0])) {
            a2dp_src_run_args->type = 'p';
            memset(a2dp_src_run_args->parameter, 0, sizeof(a2dp_src_run_args->parameter));
            strcpy(a2dp_src_run_args->parameter, args[1]);
        } else {
            BTMG_ERROR("Please enter the parameters correctly");
            free(a2dp_src_run_args);
            return;
        }
        pthread_create(&bt_test_a2dp_source_thread, NULL, _a2dp_source_thread_func,
                       (void *)a2dp_src_run_args);
    }
}

static void cmd_bt_a2dp_src_set_status(int argc, char *args[])
{
    if (a2dp_src_enable == true) {
        if (strcmp("play", args[0]) == 0) {
            bt_manager_set_avrcp_status(BTMG_AVRCP_PLAYSTATE_PLAYING);
        } else if (strcmp("pause", args[0]) == 0) {
            bt_manager_set_avrcp_status(BTMG_AVRCP_PLAYSTATE_PAUSED);
        } else if (strcmp("forward", args[0]) == 0) {
            bt_manager_set_avrcp_status(BTMG_AVRCP_PLAYSTATE_FORWARD);
        } else if (strcmp("backward", args[0]) == 0) {
            bt_manager_set_avrcp_status(BTMG_AVRCP_PLAYSTATE_BACKWARD);
        }
        return;
    }
    if (a2dp_src_loop == false) {
        BTMG_INFO("a2dp source play thread no start");
    }
}

void bt_a2dp_src_stop(void)
{
    if (a2dp_src_enable == false) {
        BTMG_INFO("a2dp source play thread doesn't run");
        return;
    }

    a2dp_src_loop = false;
    pthread_join(bt_test_a2dp_source_thread, NULL);
    bt_manager_set_avrcp_status(BTMG_AVRCP_PLAYSTATE_STOPPED);
    if (a2dp_src_run_args) {
        free(a2dp_src_run_args);
        a2dp_src_run_args = NULL;
    }

    if (a2dp_source_musiclist) {
        free(a2dp_source_musiclist);
        a2dp_source_musiclist = NULL;
    }
}

static void cmd_bt_a2dp_src_stop(int argc, char *args[])
{
	bt_a2dp_src_stop();
}

bool bt_a2dp_src_is_run(void)
{
    return a2dp_src_loop;
}

static void cmd_bt_a2dp_set_vol(int argc, char *args[])
{
    int vol_value = 0;

    vol_value = atoi(args[0]);
    if (vol_value > 100)
        vol_value = 100;

    if (vol_value < 0)
        vol_value = 0;

    bt_manager_a2dp_set_vol(vol_value);

    BTMG_INFO("a2dp set vol:%d", vol_value);
}

static void cmd_bt_a2dp_get_vol(int argc, char *args[])
{
    int vol_value = 0;

    vol_value = bt_manager_a2dp_get_vol();

    BTMG_INFO("a2dp get vol:%d", vol_value);
}

static void cmd_bt_hfp_answer_call(int argc, char *args[])
{
    bt_manager_hfp_hf_send_at_ata();
}

static void cmd_bt_hfp_hangup(int argc, char *args[])
{
    bt_manager_hfp_hf_send_at_chup();
}

static void cmd_bt_hfp_dial_num(int argc, char *args[])
{
    bt_manager_hfp_hf_send_at_atd(args[0]);
}

static void cmd_bt_hfp_subscriber_number(int argc, char *args[])
{
    bt_manager_hfp_hf_send_at_cnum();
}

static void cmd_bt_hfp_last_num_dial(int argc, char *args[])
{
    bt_manager_hfp_hf_send_at_bldn();
}

static void cmd_bt_hfp_hf_vgs_volume(int argc, char *args[])
{
    int val;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    val = atoi(args[0]);
    if (val > 15 || val < 0) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    bt_manager_hfp_hf_send_at_vgs(val);
}

static void cmd_bt_spp_client_connect(int argc, char *args[])
{
    if (argc != 1 && strlen(args[0]) != 17) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    bt_manager_spp_client_connect(args[0]);
}

static void cmd_bt_spp_client_send(int argc, char *args[])
{
    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    bt_manager_spp_client_send(args[0], strlen(args[0]));
}

static void cmd_bt_spp_client_disconnect(int argc, char *args[])
{
    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    bt_manager_spp_client_disconnect(args[0]);
}

static void cmd_bt_spp_service_accept(int argc, char *args[])
{
    bt_manager_spp_service_accept();
}

static void cmd_bt_spp_service_send(int argc, char *args[])
{
    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    bt_manager_spp_service_send(args[0], strlen(args[0]));
}

static void cmd_bt_spp_service_disconnect(int argc, char *args[])
{
    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    bt_manager_spp_service_disconnect(args[0]);
}

static void cmd_bt_set_ex_debug_mask(int argc, char *args[])
{
    int val;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    val = atoi(args[0]);
    bt_manager_set_ex_debug_mask(val);
}

static void cmd_bt_set_scan_mode(int argc, char *args[])
{
    int val;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    val = atoi(args[0]);
    if (val > 2 || val < 0) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    bt_manager_set_scan_mode((btmg_scan_mode_t)val);
}
static void cmd_bt_set_page_timeout(int argc, char *args[])
{
    int val;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    val = atoi(args[0]);
    if (val > 65535 || val < 1) {
        BTMG_ERROR("page timeout out of range: 1~65535");
        return;
    }

    bt_manager_set_page_timeout(val);
}

static void cmd_bt_set_io_capability(int argc, char *args[])
{
    int val;

    if (argc != 1) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }

    val = atoi(args[0]);
    if (val > 4 || val < 0) {
        BTMG_ERROR("Unexpected argc: %d, see help", argc);
        return;
    }
    bt_manager_agent_set_io_capability((btmg_io_capability_t)val);
}

static void cmd_bt_get_version(int argc, char *args[])
{
    BTMG_INFO("btmanager version:%s, builed time:%s-%s", BTMGVERSION, __DATE__, __TIME__);
}

cmd_tbl_t bt_cmd_table[] = {
    { "enable", NULL, cmd_bt_enable, "enable [0/1]: open bt or not" },
    { "scan", NULL, cmd_bt_scan, "scan [0/1]: scan for devices" },
    { "scan_list", NULL, cmd_bt_inquiry_list, "scan_list: list available devices" },
    { "pair", NULL, cmd_bt_pair, "pair [mac]: pair with devices" },
    { "unpair", NULL, cmd_bt_unpair, "unpair [mac]: unpair with devices" },
    { "paired_list", NULL, cmd_bt_pair_devices_list, "paired_list: list paired devices" },
    { "get_adapter_state", NULL, cmd_bt_get_adapter_state,
      "get_adapter_state: get bt adapter state" },
    { "get_adapter_name", NULL, cmd_bt_get_adapter_name, "get_adapter_name: get bt adapter name" },
    { "set_adapter_name", NULL, cmd_bt_set_adapter_name,
      "set_adapter_name [name]: set bt adapter name" },
    { "get_adapter_addr", NULL, cmd_bt_get_adapter_address,
      "get_adapter_addr: get bt adapter address" },
    { "get_device_name", NULL, cmd_bt_get_device_name,
      "get_device_name[mac]: get remote device name" },
    { "set_scan_mode", NULL, cmd_bt_set_scan_mode,
      "set_scan_mode [0~2]:0-NONE,1-page scan,2-inquiry scan&page scan" },
    { "set_page_to", NULL, cmd_bt_set_page_timeout, "real timeout = slots * 0.625ms" },
    { "set_io_cap", NULL, cmd_bt_set_io_capability,
      "set_io_cap [0~4]:0-keyboarddisplay,1-displayonly,2-displayyesno,3-keyboardonly,4-noinputnooutput" },
    { "avrcp", NULL, cmd_bt_avrcp,
      "avrcp [play/pause/stop/fastforward/rewind/forward/backward]: avrcp control" },
    { "connect", NULL, cmd_bt_device_connect, "connect [mac]:generic method to connect" },
    { "disconnect", NULL, cmd_bt_device_disconnect,
      "disconnect [mac]:generic method to disconnect" },
    { "remove", NULL, cmd_bt_remove_device, "remove [mac]:removes the remote device" },
    { "a2dp_src_run", NULL, cmd_bt_a2dp_src_run,
      "a2dp_src_run -p [folder path]  or  a2dp_src_run -f [file path]" },
    { "a2dp_src_set_status", NULL, cmd_bt_a2dp_src_set_status,
      "a2dp_src_set_status [status]:play pause forward backward" },
    { "a2dp_src_stop", NULL, cmd_bt_a2dp_src_stop, "a2dp_src_stop:stop a2dp source playing" },
    { "a2dp_set_vol", NULL, cmd_bt_a2dp_set_vol, "a2dp_set_vol: set a2dp device volme" },
    { "a2dp_get_vol", NULL, cmd_bt_a2dp_get_vol, "a2dp_src_get_vol: get a2dp device volme" },
    { "hfp_answer", NULL, cmd_bt_hfp_answer_call, "hfp_answer: answer the phone" },
    { "hfp_hangup", NULL, cmd_bt_hfp_hangup, "hfp_hangup: hangup the phone" },
    { "hfp_dial", NULL, cmd_bt_hfp_dial_num, "hfp_dial [num]: call to a phone number" },
    { "hfp_cnum", NULL, cmd_bt_hfp_subscriber_number, "hfp_cum: Subscriber Number Information" },
    { "hfp_last_num", NULL, cmd_bt_hfp_last_num_dial,
      "hfp_last_num: calling the last phone number dialed" },
    { "hfp_vol", NULL, cmd_bt_hfp_hf_vgs_volume, "hfp_vol [0~15]: update phone's volume." },
    { "sppc_connect", NULL, cmd_bt_spp_client_connect, "sppc_connect[mac]:connect to spp server" },
    { "sppc_send", NULL, cmd_bt_spp_client_send, "sppc_send xxx: send data" },
    { "sppc_disconnect", NULL, cmd_bt_spp_client_disconnect,
      "sppc_disconnect[mac]:disconnect spp server" },
    { "spps_accept", NULL, cmd_bt_spp_service_accept, "spps_accept the client" },
    { "spps_send", NULL, cmd_bt_spp_service_send, "spps_send data" },
    { "spps_disconnect", NULL, cmd_bt_spp_service_disconnect, "spps_disconnect dst" },
    { "get_version", NULL, cmd_bt_get_version, "get_version: get btmanager version" },
    { "debug", NULL, cmd_bt_set_log_level, "debug [0~5]: set debug level" },
    { "ex_dbg", NULL, cmd_bt_set_ex_debug_mask, "ex_dbg [mask]: set ex debug mask" },
    { NULL, NULL, NULL, NULL },
};
