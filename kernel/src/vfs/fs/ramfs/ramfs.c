#include "ramfs.h"
#include "kprint.h"
#include "heap/kheap.h"
#include "string.h"
#include "global.h"
#include "vfs/vfs.h"

typedef struct ramfs_file {
    char name[256];
    bool is_dir;
    uint8_t* data;
    size_t size;
    struct ramfs_file* parent;
    struct ramfs_file* children;
    struct ramfs_file* next;
} ramfs_file_t;

// Forward declarations
static vfs_node_t* ramfs_finddir(vfs_node_t* node, const char* name);
static int ramfs_readdir(vfs_node_t* node, size_t index, vfs_dirent_t* dirent);
static ssize_t ramfs_read(vfs_node_t* node, size_t offset, size_t size, void* buffer);
static ssize_t ramfs_write(vfs_node_t* node, size_t offset, size_t size, const void* buffer);
static vfs_node_t* ramfs_create_node(vfs_node_t* parent_node, const char* name, bool is_dir, const void* content, size_t size);

// Root file and node
static ramfs_file_t ramfs_root_file = {
    .name = "",
    .is_dir = true,
    .data = NULL,
    .size = 0,
    .parent = NULL,
    .children = NULL,
    .next = NULL
};

static vfs_ops_t ramfs_ops;

static vfs_node_t ramfs_root_node = {
    .name = "/",
    .type = VFS_NODE_DIR,
    .permissions = VFS_READ | VFS_EXEC,
    .size = 0,
    .private_data = &ramfs_root_file,
    .ops = &ramfs_ops,
    .parent = NULL
};

static vfs_node_t* ramfs_mount(void* data) {
    (void)data;
    if (debug && VLEVEL >= 2) 
        kprint(LOG_DEBUG, "ramfs: mount()\n");
    return &ramfs_root_node;
}

filesystem_t ramfs_fs = {
    .name = "ramfs",
    .init = NULL,
    .mount = ramfs_mount
};

// Core VFS ops
static ssize_t ramfs_read(vfs_node_t* node, size_t offset, size_t size, void* buffer) {
    ramfs_file_t* file = (ramfs_file_t*)node->private_data;
    if (debug && VLEVEL >= 1) 
        kprint(LOG_DEBUG, "ramfs: read(%s, offset=%zu, size=%zu)\n", file->name, offset, size);

    if (!file || file->is_dir || !file->data) return -1;
    if (offset >= file->size) return 0;

    size_t to_read = (offset + size > file->size) ? (file->size - offset) : size;
    memcpy(buffer, file->data + offset, to_read);
    return to_read;
}

static ssize_t ramfs_write(vfs_node_t* node, size_t offset, size_t size, const void* buffer) {
    ramfs_file_t* file = (ramfs_file_t*)node->private_data;
    if (debug && VLEVEL >= 1) 
        kprint(LOG_DEBUG, "ramfs: write(%s, offset=%zu, size=%zu)\n", file->name, offset, size);

    if (!file || file->is_dir || !buffer) return -1;

    size_t end = offset + size;

    if (end > file->size) {
        uint8_t* new_data = kmalloc(end);
        if (file->data) {
            memcpy(new_data, file->data, file->size);
            kfree(file->data);
        }
        if (offset > file->size) {
            memset(new_data + file->size, 0, offset - file->size);
        }
        file->data = new_data;
        file->size = end;
    }

    memcpy(file->data + offset, buffer, size);
    return size;
}

static int ramfs_readdir(vfs_node_t* node, size_t index, vfs_dirent_t* dirent) {
    ramfs_file_t* dir = (ramfs_file_t*)node->private_data;
    if (debug && VLEVEL >= 4) 
        kprint(LOG_DEBUG, "ramfs: readdir(%s, index=%zu)\n", dir->name, index);

    if (!dir || !dir->is_dir) return -1;

    ramfs_file_t* child = dir->children;
    for (size_t i = 0; child != NULL; child = child->next, i++) {
        if (i == index) {
            strncpy(dirent->name, child->name, sizeof(dirent->name));
            dirent->name[sizeof(dirent->name) - 1] = '\0';

            vfs_node_t* child_node = kmalloc(sizeof(vfs_node_t));
            strncpy(child_node->name, child->name, sizeof(child_node->name));
            child_node->name[sizeof(child_node->name) - 1] = '\0';
            child_node->type = child->is_dir ? VFS_NODE_DIR : VFS_NODE_FILE;
            child_node->permissions = VFS_READ | VFS_WRITE;
            child_node->size = child->size;
            child_node->private_data = child;
            child_node->ops = &ramfs_ops;
            child_node->parent = node;

            dirent->node = child_node;
            return 0;
        }
    }

    return -1;
}

static vfs_node_t* ramfs_finddir(vfs_node_t* node, const char* name) {
    ramfs_file_t* dir = (ramfs_file_t*)node->private_data;
    if (debug && VLEVEL >= 3) 
        kprint(LOG_DEBUG, "ramfs: finddir(%s in %s)\n", name, dir->name);

    if (!dir || !dir->is_dir) return NULL;

    ramfs_file_t* child = dir->children;
    while (child) {
        if (strcmp(child->name, name) == 0) {
            vfs_node_t* child_node = kmalloc(sizeof(vfs_node_t));
            strncpy(child_node->name, child->name, sizeof(child_node->name));
            child_node->name[sizeof(child_node->name) - 1] = '\0';
            child_node->type = child->is_dir ? VFS_NODE_DIR : VFS_NODE_FILE;
            child_node->permissions = VFS_READ | VFS_WRITE;
            child_node->size = child->size;
            child_node->private_data = child;
            child_node->ops = &ramfs_ops;
            child_node->parent = node;
            return child_node;
        }
        child = child->next;
    }

    return NULL;
}

static vfs_node_t* ramfs_create_node(vfs_node_t* parent_node, const char* name, bool is_dir, const void* content, size_t size) {
    ramfs_file_t* parent = (ramfs_file_t*)parent_node->private_data;
    if (debug && VLEVEL >= 2) 
        kprint(LOG_DEBUG, "ramfs: create_node(%s, dir=%d)\n", name, is_dir);

    if (!parent || !parent->is_dir) return NULL;

    ramfs_file_t* file = kmalloc(sizeof(ramfs_file_t));
    memset(file, 0, sizeof(ramfs_file_t));
    strncpy(file->name, name, sizeof(file->name));
    file->name[sizeof(file->name) - 1] = '\0';
    file->is_dir = is_dir;
    file->parent = parent;

    if (!is_dir && content && size > 0) {
        file->data = kmalloc(size);
        memcpy(file->data, content, size);
        file->size = size;
    }

    file->next = parent->children;
    parent->children = file;

    vfs_node_t* node = kmalloc(sizeof(vfs_node_t));
    strncpy(node->name, name, sizeof(node->name));
    node->name[sizeof(node->name) - 1] = '\0';
    node->type = is_dir ? VFS_NODE_DIR : VFS_NODE_FILE;
    node->permissions = VFS_READ | VFS_WRITE;
    node->size = file->size;
    node->private_data = file;
    node->ops = &ramfs_ops;
    node->parent = parent_node;

    return node;
}

// Register the ops table
static vfs_ops_t ramfs_ops = {
    .read = ramfs_read,
    .write = ramfs_write,
    .open = NULL,
    .close = NULL,
    .readdir = ramfs_readdir,
    .finddir = ramfs_finddir,
    .create = ramfs_create_node
};
