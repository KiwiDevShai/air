#include "vfs/file.h"
#include "vfs/vfs.h"
#include "kprint.h"
#include "global.h"

#define MAX_OPEN_FILES 128

static vfs_node_t* open_table[MAX_OPEN_FILES];

int fopen(const char* path) {
    vfs_node_t* node = vfs_lookup(path);
    if (!node) {
        if (debug && VLEVEL >= 1)
            kprint(LOG_DEBUG, "open: '%s' not found\n", path);
        return -1;
    }

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!open_table[i]) {
            open_table[i] = node;
            if (node->ops && node->ops->open)
                node->ops->open(node);
            return i;
        }
    }

    return -1; // No free slot
}

ssize_t fread(int fd, void* buf, size_t size) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_table[fd]) return -1;
    return vfs_read(open_table[fd], 0, size, buf); // offset = 0 for now
}

ssize_t fwrite(int fd, const void* buf, size_t size) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_table[fd]) return -1;
    return vfs_write(open_table[fd], 0, size, buf); // offset = 0 for now
}

int fclose(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_table[fd]) return -1;

    if (open_table[fd]->ops && open_table[fd]->ops->close)
        open_table[fd]->ops->close(open_table[fd]);

    open_table[fd] = NULL;
    return 0;
}

int fmkdir(const char* path) {
    return vfs_create_dir(path) ? 0 : -1;
}

int fcreate(const char* path, const void* content, size_t size) {
    return vfs_create_file(path, content, size) ? 0 : -1;
}
