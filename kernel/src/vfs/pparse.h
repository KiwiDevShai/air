#ifndef PPARSE_H
#define PPARSE_H

#include <stddef.h>

// Max components in a path ("/a/b/c" -> 3)
#define MAX_PATH_PARTS 32

typedef struct {
    char parts[MAX_PATH_PARTS][256];
    size_t count;
} path_t;

// Parse raw path into path_t
void path_parse(const char* raw, path_t* out);

#endif // PPARSE_H
