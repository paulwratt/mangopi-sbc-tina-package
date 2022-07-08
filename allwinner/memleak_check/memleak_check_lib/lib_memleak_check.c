#include <malloc.h>
#include <execinfo.h>
#include <string.h>
#include <unistd.h>
#include "lib_memleak_check.h"

#define MAX_FRAMES 16
#define DEBUG_FILE_PATH	"/mnt/exUDISK/mem_leak_log.txt"

static FILE *debug_output = NULL;
static void *(*old_malloc_hook)(size_t size, const void *caller);
static void (*old_free_hook)(void *ptr, const void *caller);
static void *(*old_realloc_hook)(void *ptr, size_t size, const void *caller);

static void* my_malloc_hook(size_t size, const void *caller);
static void my_free_hook(void *ptr, const void *caller);
static void *my_realloc_hook (void *ptr, size_t size, const void *caller);

static int init_output_log_file(void) {
	debug_output = fopen(DEBUG_FILE_PATH, "w");
	if (debug_output == NULL) {
		perror("file create error\n");
		return -1;
	}
	return 0;
}

static int uninit_output_log_file(void) {
	if (debug_output) {
		fsync(fileno(debug_output));
		fclose(debug_output);
		debug_output = NULL;
	}
	return 0;
}

static void* my_malloc_hook(size_t size, const void *caller) {
	int layers = 0, index = 0;
	char **symbols = NULL;
	void *result = NULL;
	void *frames[MAX_FRAMES] = {0};

	__malloc_hook	= NULL;
	__free_hook	= NULL;
	__realloc_hook	= NULL;

	result = malloc(size);
	if (debug_output == NULL) {
		printf("output file null\n");
		goto out;
	}
	if (result != NULL) {
		fprintf(debug_output, "[malloc:\n");
		layers = backtrace(frames, MAX_FRAMES);
		symbols = backtrace_symbols(frames, layers);
		if (symbols) {
			for (index = 0; index < layers; index++)
			{
				fprintf(debug_output, "malloc: %d: %s\n", index, symbols[index]);
			}
			free(symbols);
		}
		fprintf(debug_output, "mem_address:%p\n", result);
		fprintf(debug_output, "mem_size:0x%x\n", (unsigned int)size);
		fprintf(debug_output, "malloc_end]\n");
	}
	fsync(fileno(debug_output));
out:
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__realloc_hook	= my_realloc_hook;
	return result;
}

static void my_free_hook(void *ptr, const void *caller) {
	int layers = 0, index = 0;
	char **symbols = NULL;
	void *frames[MAX_FRAMES] = {0};
	void *result = NULL;

	__malloc_hook = NULL;
	__free_hook = NULL;
	__realloc_hook	= NULL;
	if (debug_output == NULL) {
		printf("output file null\n");
		goto out;
	}

	if (ptr != NULL) {
		fprintf(debug_output, "[free:\n");
		layers = backtrace(frames, MAX_FRAMES);
		symbols = backtrace_symbols(frames, layers);
		if (symbols) {
			for (index = 0; index < layers; index++)
			{
				fprintf(debug_output, "free:%d: %s\n", index, symbols[index]);
			}
			free(symbols);
		}

		fprintf(debug_output, "mem_address:%p\n", ptr);
		fprintf(debug_output, "free_end]\n");
	}
	fsync(fileno(debug_output));
out:
	free(ptr);
	__free_hook = my_free_hook;
	__malloc_hook = my_malloc_hook;
	__realloc_hook	= my_realloc_hook;
}
static void *my_realloc_hook (void *ptr, size_t size, const void *caller) {
	int layers = 0, index = 0;
	char **symbols = NULL;
	void *result = NULL;
	void *frames[MAX_FRAMES] = {0};

	__malloc_hook	= NULL;
	__free_hook	= NULL;
	__realloc_hook	= NULL;

	result = realloc(ptr, size);
	if (debug_output == NULL) {
		printf("output file null\n");
		goto out;
	}
	fprintf(debug_output, "[free:\n");
	fprintf(debug_output, "realloc_hook:\n");
	fprintf(debug_output, "mem_address:%p\n", ptr);
	fprintf(debug_output, "free_end]\n");
	if (result != NULL) {
		fprintf(debug_output, "[malloc:\n");
		fprintf(debug_output, "realloc:\n");
		layers = backtrace(frames, MAX_FRAMES);
		symbols = backtrace_symbols(frames, layers);
		if (symbols) {
			for (index = 0; index < layers; index++)
			{
				fprintf(debug_output, "malloc: %d: %s\n", index, symbols[index]);
			}
			free(symbols);
		}
		fprintf(debug_output, "mem_address:%p\n", result);
		fprintf(debug_output, "mem_size:0x%x\n", (unsigned int)size);
		fprintf(debug_output, "malloc_end]\n");
	}
	fsync(fileno(debug_output));
out:
	__malloc_hook = my_malloc_hook;
	__free_hook = my_free_hook;
	__realloc_hook	= my_realloc_hook;
	return result;
}

void my_init_hook(void) {
	if (init_output_log_file() != 0) {
		printf("%s %d %s failed!\n", __FILE__, __LINE__, __func__);
		return ;
	}
	old_malloc_hook	= __malloc_hook;
	old_free_hook	= __free_hook;
	old_realloc_hook = __realloc_hook;
	__realloc_hook	= my_realloc_hook;
	__malloc_hook	= my_malloc_hook;
	__free_hook		= my_free_hook;
}

void my_deinit_hook(void) {
	__realloc_hook	= old_realloc_hook;
	__malloc_hook	= old_malloc_hook;
	__free_hook		= old_free_hook;
	uninit_output_log_file();
}
