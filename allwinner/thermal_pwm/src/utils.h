
#ifndef _UTILS_H_
#define _UTILS_H_

int read_from_file(const char *path, char *buf, size_t size);
int write_to_file(const char *path, const char *buffer, int i);
int readIntFromFile(const char *path);
int writeIntToFile(const char *path, int t);

#endif
