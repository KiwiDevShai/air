#ifndef VFS_H
#define VFS_H

#include "global.h"
#include <stddef.h>
#include <stdint.h>

// Types

typedef enum {
    VFS_NODE_FILE,
    VFS_NODE_DIR,
    VFS_NODE_SYMLINK,
    VFS_NODE_DEVICE,
    VFS_NODE_UNKNOWN
} vfs_node_type_t;

typedef enum {
    VFS_READ  = 0x1,
    VFS_WRITE = 0x2,
    VFS_EXEC  = 0x4,
} vfs_perm_t;

struct vfs_node;
typedef struct vfs_node vfs_node_t;

typedef struct {
    char name[256];
    vfs_node_t* node;
} vfs_dirent_t;

typedef struct {
    ssize_t     (*read)(vfs_node_t* node, size_t offset, size_t size, void* buffer );
    ssize_t     (*write)(vfs_node_t* node, size_t offset, size_t size, const void* buffer);
    int         (*open)(vfs_node_t* node);      
    int         (*close)(vfs_node_t* node);
    int         (*readdir)(vfs_node_t* node, size_t index, vfs_dirent_t* dirent);
    vfs_node_t* (*finddir)(vfs_node_t* node, const char* name);
    vfs_node_t* (*create)(vfs_node_t* parent, const char* name, bool is_dir, const void* content, size_t size);
} vfs_ops_t;

struct vfs_node {
    char name[256];
    vfs_node_type_t type;
    uint32_t permissions;
    size_t size;
    void* private_data;
    vfs_ops_t* ops;
    vfs_node_t* parent;
};

typedef struct filesystem {
    const char* name;
    void (*init)(void);
    vfs_node_t* (*mount)(void* data);
} filesystem_t;

// Mountpoint structure
typedef struct mount {
    char path[4096];
    vfs_node_t* root_node;
} mount_t;

// API
void vfs_init(void);
int vfs_register_filesystem(filesystem_t* fs);
int vfs_mount(const char* fs_name, void* mount_data, const char* mount_path);

vfs_node_t* vfs_resolve(const char* path);
vfs_node_t* vfs_root(void);
vfs_node_t* vfs_lookup(const char* path);
vfs_node_t* vfs_create_file(const char* path, const void* content, size_t size);
vfs_node_t* vfs_create_dir(const char* path);

ssize_t vfs_read(vfs_node_t* node, size_t offset, size_t size, void* buffer);
ssize_t vfs_write(vfs_node_t* node, size_t offset, size_t size, const void* buffer);
int vfs_open(vfs_node_t* node);
int vfs_close(vfs_node_t* node);
int vfs_readdir(vfs_node_t* node, size_t index, vfs_dirent_t* dirent);
vfs_node_t* vfs_finddir(vfs_node_t* node, const char* name);

#endif // VFS_H
