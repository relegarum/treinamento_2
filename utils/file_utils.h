#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

extern const char *const IndexStr;
extern const char *const PutMark;

typedef struct file_struct
{
  FILE *file_ptr;
  char file_path[PATH_MAX];
}FileComponents;

int32_t init_file_components(FileComponents *file, char *file_path, const char *flags);
int32_t destroy_file_components(FileComponents *file);

int8_t verify_file_path(char *path, char *resource, char *full_path);
int32_t get_file_mime(uint32_t full_path_size, char *full_path, char *mime);

#endif /*FILE_UTILS_H*/
