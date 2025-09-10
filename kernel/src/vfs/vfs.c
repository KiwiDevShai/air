#include "vfs.h"
#include "string.h"
#include "kprint.h"
#include "global.h"
#include "pparse.h"

#define MAX_FILESYSTEMS 8
#define MAX_MOUNTS 16

static filesystem_t* registered_filesystems[MAX_FILESYSTEMS];
static mount_t mounts[MAX_MOUNTS];

void vfs_init(void) {
    if (debug && VLEVEL >= 1) kprint(LOG_DEBUG, "vfs: init()\n");

    for (int i = 0; i < MAX_FILESYSTEMS; i++)
        registered_filesystems[i] = NULL;

    for (int i = 0; i < MAX_MOUNTS; i++) {
        mounts[i].root_node = NULL;
        mounts[i].path[0] = '\0';
    }
}

int vfs_register_filesystem(filesystem_t* fs) {
    if (debug && VLEVEL >= 2) 
        kprint(LOG_DEBUG, "vfs: register_filesystem('%s')\n", fs->name);

    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        if (!registered_filesystems[i]) {
            registered_filesystems[i] = fs;
            return 0;
        }
    }
    return -1;
}

int vfs_mount(const char* fs_name, void* mount_data, const char* mount_path) {
    if (debug && VLEVEL >= 2) 
        kprint(LOG_DEBUG, "vfs: mount('%s') at '%s'\n", fs_name, mount_path);

    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        filesystem_t* fs = registered_filesystems[i];
        if (fs && strcmp(fs->name, fs_name) == 0) {
            if (fs->init) fs->init();
            vfs_node_t* root = fs->mount(mount_data);
            if (!root) return -1;

            for (int j = 0; j < MAX_MOUNTS; j++) {
                if (!mounts[j].root_node) {
                    strncpy(mounts[j].path, mount_path, 255);
                    mounts[j].path[255] = '\0';
                    mounts[j].root_node = root;
                    return 0;
                }
            }
        }
    }

    return -1;
}

vfs_node_t* vfs_root(void) {
    return vfs_resolve("/");
}

vfs_node_t* vfs_resolve(const char* path) {
    if (debug && VLEVEL >= 2) 
        kprint(LOG_DEBUG, "vfs: resolve('%s')\n", path);

    mount_t* best = NULL;
    size_t best_len = 0;

    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (!mounts[i].root_node) continue;

        size_t len = strlen(mounts[i].path);
        if (strncmp(path, mounts[i].path, len) == 0 && len > best_len) {
            best = &mounts[i];
            best_len = len;
        }
    }

    if (!best) return NULL;

    path_t parsed;
    path_parse(path + best_len, &parsed);

    vfs_node_t* node = best->root_node;

    for (size_t i = 0; i < parsed.count; i++) {
        if (!node->ops || !node->ops->finddir)
            return NULL;
        node = node->ops->finddir(node, parsed.parts[i]);
        if (!node) return NULL;
    }

    return node;
}

vfs_node_t* vfs_lookup(const char* path) {
    return vfs_resolve(path);
}

ssize_t vfs_read(vfs_node_t* node, size_t offset, size_t size, void* buffer) {
    if (debug && VLEVEL >= 1) 
        kprint(LOG_DEBUG, "vfs: read(%s, %zu, %zu)\n", node ? node->name : "null", offset, size);

    if (!node || !node->ops || !node->ops->read)
        return -1;
    return node->ops->read(node, offset, size, buffer);
}

ssize_t vfs_write(vfs_node_t* node, size_t offset, size_t size, const void* buffer) {
    if (debug && VLEVEL >= 1) 
        kprint(LOG_DEBUG, "vfs: write(%s, %zu, %zu)\n", node ? node->name : "null", offset, size);

    if (!node || !node->ops || !node->ops->write)
        return -1;
    return node->ops->write(node, offset, size, buffer);
}

int vfs_open(vfs_node_t* node) {
    if (debug && VLEVEL >= 1) 
        kprint(LOG_DEBUG, "vfs: open(%s)\n", node ? node->name : "null");

    if (!node || !node->ops || !node->ops->open)
        return -1;
    return node->ops->open(node);
}

int vfs_close(vfs_node_t* node) {
    if (debug && VLEVEL >= 1) 
        kprint(LOG_DEBUG, "vfs: close(%s)\n", node ? node->name : "null");

    if (!node || !node->ops || !node->ops->close)
        return -1;
    return node->ops->close(node);
}

int vfs_readdir(vfs_node_t* node, size_t index, vfs_dirent_t* dirent) {
    if (debug && VLEVEL >= 4) 
        kprint(LOG_DEBUG, "vfs: readdir(%s, %zu)\n", node ? node->name : "null", index);

    if (!node || !node->ops || !node->ops->readdir)
        return -1;
    return node->ops->readdir(node, index, dirent);
}

vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name) {
    if (debug && VLEVEL >= 3) 
        kprint(LOG_DEBUG, "vfs: finddir(%s, '%s')\n", node ? node->name : "null", name);

    if (!node || !node->ops || !node->ops->finddir)
        return NULL;
    return node->ops->finddir(node, name);
}

vfs_node_t* vfs_create_file(const char* path, const void* content, size_t size) {
    if (debug && VLEVEL >= 2) 
        kprint(LOG_DEBUG, "vfs: create_file('%s')\n", path);

    char tmp[256];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    char* last = strrchr(tmp, '/');
    if (!last) return NULL;

    *last = '\0';
    const char* name = last + 1;

    vfs_node_t* parent = vfs_lookup(tmp[0] ? tmp : "/");
    if (!parent || !parent->ops || !parent->ops->create)
        return NULL;

    return parent->ops->create(parent, name, false, content, size);
}

vfs_node_t* vfs_create_dir(const char* path) {
    if (debug && VLEVEL >= 2) 
        kprint(LOG_DEBUG, "vfs: create_dir('%s')\n", path);

    char tmp[256];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    char* last = strrchr(tmp, '/');
    if (!last) return NULL;

    *last = '\0';
    const char* name = last + 1;

    vfs_node_t* parent = vfs_lookup(tmp[0] ? tmp : "/");
    if (!parent || !parent->ops || !parent->ops->create)
        return NULL;

    return parent->ops->create(parent, name, true, NULL, 0);
}
