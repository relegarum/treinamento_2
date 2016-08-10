#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

extern const char *const IndexStr;
extern const char *const PutMark;

enum FileFlags
{
  ReadFile      = 1 << 0,
  WriteFile     = 1 << 1,
  BinaryFile    = 1 << 2,
  TextFile      = 1 << 3
};

enum InitFileComponentsEnum
{
  Success      =  0,
  FilePathNull = -1,
  ExistentFile = -2,
  UnknownFlag  = -3,
  CoudntOpen   = -4
};

typedef struct file_struct
{
  FILE *file_ptr;
  char file_path[PATH_MAX];
}FileComponents;

int32_t init_file_components(FileComponents *file,
                             char *file_path,
                             const int8_t flags);

int32_t destroy_file_components(FileComponents *file);

int8_t  verify_file_path(char *path, char *resource, char *full_path);
int32_t get_file_mime(uint32_t full_path_size, char *full_path, char *mime);
int32_t is_valid_file(FileComponents *file);
int32_t rename_file_after_put(FileComponents *file);

#endif /*FILE_UTILS_H*/
