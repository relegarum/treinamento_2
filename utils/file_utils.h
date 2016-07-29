#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <stdint.h>

extern const char *const IndexStr;

int8_t verify_file_path(char *path, char *resource, char *full_path);
int32_t get_file_mime(uint32_t full_path_size, char *full_path, char *mime);

#endif /*FILE_UTILS_H*/
