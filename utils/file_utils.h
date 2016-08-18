/* \file file_utils.c
 *
 * \brief Declaracao de funcoes relativas ao tratamento de arquivos
 * necessarios ao servidor
 *
 * "$Id: $"
*/
#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>

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
  Success           =  0,
  FilePathNull      = -1,
  ExistentFile      = -2,
  UnknownFlag       = -3,
  CouldntOpen       = -4,
  FileComponentNull = -5,
  NotARegularFile   = -6
};

typedef struct file_struct
{
  FILE        *file_ptr;
  char        file_path[PATH_MAX];
  uint8_t     is_new_file;
  struct stat stats;
  uint8_t     should_delete;

}FileComponents;


void init_file_components(FileComponents *file);
int32_t set_file_components(FileComponents *file,
                             char *file_path,
                             const int8_t flags);

int32_t destroy_file_components(FileComponents *file);

void read_treatment(FileComponents *file, char *file_path);
int8_t write_treatment(FileComponents *file, char *file_path);

void setup_file_path(char *base_path, char *resource, char *full_path);
int8_t verify_file_path(char *base_path, char *full_path);
int8_t is_symbolic_link_and_valid(char *base_path,
                                         const int32_t base_path_len,
                                         char *full_path);

int8_t is_realpath_inside_base_path(char *base_path,
                                              const int32_t base_path_len,
                                              char *real_path);

int32_t get_file_mime(uint32_t full_path_size, char *full_path, char *mime);
int32_t treat_file_after_put(FileComponents *file, uint8_t error);

uint8_t get_file_stats(char *file_path, struct stat *file_stats);
uint8_t is_valid_file(FileComponents *file);
uint8_t is_regular_file(FileComponents *file);
uint8_t is_directory(FileComponents *file);

#endif /*FILE_UTILS_H*/
