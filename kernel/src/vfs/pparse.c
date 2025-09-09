#include "pparse.h"
#include "string.h"

void path_parse(const char* raw, path_t* out) {
    size_t part_index = 0;
    size_t i = 0;
    size_t len = strlen(raw);
    out->count = 0;

    while (i < len && part_index < MAX_PATH_PARTS) {
        while (i < len && (raw[i] == '/' || raw[i] == ' '))
            i++;

        if (i >= len) break;

        size_t j = 0;
        while (i < len && raw[i] != '/' && j < 255)
            out->parts[part_index][j++] = raw[i++];

        out->parts[part_index][j] = '\0';
        out->count++;
        part_index++;
    }
}
