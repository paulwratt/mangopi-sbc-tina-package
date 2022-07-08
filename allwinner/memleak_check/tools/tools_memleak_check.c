#include "tools_memleak_check.h"
#define NODE_MAX_LEN	(8192)
static void usage(void) {
	printf("-i input file_name\n-o output file_name\n");
}

static int memleak_find_malloc_node(unsigned char *buff, FILE *fp) {
	unsigned char line[1024] = {0};
	unsigned char start = 0;
	unsigned int index = 0;
	static fpos_t cur_pos = {0};

	fsetpos(fp, &cur_pos);
	memset(buff, 0x00, NODE_MAX_LEN);
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strstr(line, "[malloc:") != NULL) {
			start = 1;
		}

		if (start == 1) {
			if (index >= NODE_MAX_LEN) {
				printf("%s %d %s failed will panic\n", __FILE__, __LINE__, __func__);
				return -1;
			}
			strncpy(buff + index, line, strlen(line));
			index += strlen(line);
		}

		if (strstr(line, "malloc_end]") != NULL) {
			start = 0;
			fgetpos(fp, &cur_pos);
//			printf("%s %d %s find_malloc_node:\n", __FILE__, __LINE__, __func__);
//			printf("%s", buff);
			return 1;
		}
		memset(line, 0x00, sizeof(line));
	}
	return 0;
}
static int memleak_find_free_node(unsigned char *buff, FILE *fp) {
	unsigned char line[1024] = {0};
	unsigned char start = 0;
	unsigned int index = 0;

	memset(buff, 0x00, NODE_MAX_LEN);
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strstr(line, "[free:") != NULL) {
			start = 1;
		}

		if (start == 1) {
			if (index >= NODE_MAX_LEN) {
				printf("%s %d %s failed will panic\n", __FILE__, __LINE__, __func__);
				return -1;
			}
			strncpy(buff + index, line, strlen(line));
			index += strlen(line);
		}

		if (strstr(line, "free_end]") != NULL) {
			start = 0;
//			printf("%s %d %s find_free_node:\n", __FILE__, __LINE__, __func__);
//			printf("%s", buff);
			return 1;
		}
		memset(line, 0x00, sizeof(line));
	}
	return 0;
}

static int memleak_del_node_form_file(unsigned char *buff, FILE *fp) {
	int node_len = strlen(buff);
	unsigned char line[1024] = {0};
	unsigned char start = 0;

//	printf("%s %d %s:\n", __FILE__, __LINE__, __func__);
//	printf("%s", buff);

	fseek(fp, -node_len, SEEK_CUR);
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strstr(line, "[free:") != NULL) {
			start = 1;
		}

		if (start == 1) {
			unsigned char line_temp[1024] = {0};
			int line_len = strlen(line);
			memset(line_temp, '-', line_len);
			line_temp[line_len - 1] = '\n';
			fseek(fp, -line_len, SEEK_CUR);
			fwrite(line_temp, sizeof(unsigned char), line_len, fp);
		}

		if (strstr(line, "free_end]") != NULL) {
			start = 0;
			return 1;
		}
		memset(line, 0x00, sizeof(line));
	}
	return 0;
}
static int find_address(unsigned char *buff, unsigned char *address) {
	unsigned char *temp_start = NULL, *temp_end = NULL;
	temp_start = strstr(buff, "mem_address:");
	if (temp_start == NULL) {
		printf("%s %d %s failed", __FILE__, __LINE__, __func__);
		printf("\n%s\n", buff);
		return -1;
	}
	temp_end = strchr(temp_start, '\n');
	memcpy(address, temp_start, temp_end - temp_start);
	return 0;
}
void main(int argc, void *argv) {
	char ch;
	unsigned char cmd[1024] = {0};
	FILE *input_file_fp = NULL;
	FILE *output_file_fp = NULL;
	unsigned char *input_file = NULL;
	unsigned char *output_file = NULL;
	unsigned char malloc_buff_temp[NODE_MAX_LEN] = {0};
	unsigned char free_buff_temp[NODE_MAX_LEN] = {0};

	if (argc != 5) {
		usage();
		return ;
	}
	while ((ch = getopt(argc, argv, "i:o:")) != 255) {
		extern char *optarg;
		extern int optind;
		extern int optopt;

//		printf("optind:%d optopt=%c, optarg=%s ch:%d  \n", optind, optopt, optarg, ch);
		switch (ch)
		{
			case 'i':
				input_file = optarg;
				printf("input file:%s\n\n", optarg);
			break;
			case 'o':
				output_file = optarg;
				printf("output file:%s\n\n", output_file);
			break;

			case '?':
			case 'h':
			default:
				usage();
				return ;
		}
	}
	sprintf(cmd, "cp %s ./input.txt", input_file);
	system(cmd);
	input_file_fp = fopen("./input.txt", "r+");
	if (input_file_fp == NULL) {
		printf("fopen %s fail!\n", input_file);
		return ;
	}

	output_file_fp = fopen(output_file, "w");
	if (output_file_fp == NULL) {
		printf("fopen %s fail!\n", output_file);
		return ;
	}
	while (memleak_find_malloc_node(malloc_buff_temp, input_file_fp)) {
		unsigned char malloc_address[64] = {0};
		unsigned char free_address[64] = {0};

		if (find_address(malloc_buff_temp, malloc_address) != 0) {
			printf("%s %d %s find_address failed", __FILE__, __LINE__, __func__);
			printf("\n%s\n", malloc_buff_temp);
			return ;
		}
//		printf("%s %d %s malloc_address:%s", __FILE__, __LINE__, __func__, malloc_address);
		memset(free_buff_temp, 0x00, sizeof(free_buff_temp));
		while (memleak_find_free_node(free_buff_temp, input_file_fp)) {
			memset(free_address, 0x00, sizeof(free_address));
			if (find_address(free_buff_temp, free_address) != 0) {
				printf("%s %d %s find_address failed", __FILE__, __LINE__, __func__);
				printf("\n%s\n", free_buff_temp);
				return ;
			}

//			printf("%s %d %s free_address:%s", __FILE__, __LINE__, __func__, free_address);
			if (strlen(free_address) == strlen(malloc_address) && strcmp(free_address, malloc_address) == 0) {//地址相同
				//需要删掉文件中的已经匹配的free_node
				memleak_del_node_form_file(free_buff_temp, input_file_fp);
				goto address_match;
			}
		}
		//未匹配的写入文件
		fwrite(malloc_buff_temp, sizeof(char), strlen(malloc_buff_temp), output_file_fp);
address_match:
		memset(malloc_buff_temp, 0x00, sizeof(malloc_buff_temp));
	}
	fsync(fileno(input_file_fp));
	fsync(fileno(output_file_fp));
	fclose(input_file_fp);
	fclose(output_file_fp);
	memset(cmd, 0x00, sizeof(cmd));
	sprintf(cmd, "rm -rf ./input.txt");
	system(cmd);
}
