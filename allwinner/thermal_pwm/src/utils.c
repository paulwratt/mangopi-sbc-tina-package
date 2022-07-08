
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.h"

int read_from_file(const char *path, char *buf, size_t size)
{
    int fd = open(path, O_RDONLY, 0);
    if (fd == -1) {
        INFO("Could not open '%s', %s(%d)", path, strerror(errno), errno);
        return -errno;
    }
    ssize_t count = read(fd, buf, size - 1);
    if (count > 0)
        buf[count] = '\0';
    else
        buf[0] = '\0';

    close(fd);
    return count;
}

int write_to_file(const char *path, const char *buffer, int i) {
    int fd = open(path, O_WRONLY|O_CREAT, 0);
    if (fd == -1) {
        INFO("Could not open '%s', %s(%d)", path, strerror(errno), errno);
        return -1;
    }
    write(fd, buffer, i);
    close(fd);
    return 0;
}

int readIntFromFile(const char *path) {
    char buf[32] = {0};
    if (read_from_file(path, buf, 32) > 0) {
        return strtoul(buf, 0, 0);
    }
    return -1;
}

int writeIntToFile(const char *path, int t) {
    int fd = open(path, O_WRONLY, 0);
    if (fd == -1) {
        INFO("Could not open '%s', %s(%d)", path, strerror(errno), errno);
        return -1;
    }
    char buf[16] = {0};
    sprintf(buf, "%d", t);
    write(fd, buf, strlen(buf));
    close(fd);
    return 0;
}

