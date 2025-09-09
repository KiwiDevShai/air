#ifndef VFS_FILE_H
#define VFS_FILE_H

#include "global.h"
#include <stddef.h>

int fopen(const char* path);
ssize_t fread(int fd, void* buf, size_t size);
ssize_t fwrite(int fd, const void* buf, size_t size);
int fclose(int fd);

int fmkdir(const char* path);
int fcreate(const char* path, const void* content, size_t size);

#endif // VFS_FILE_H
