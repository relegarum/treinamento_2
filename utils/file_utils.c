/* \file file_utils.c
 *
 * \brief Implementacao de funcoes relativas ao tratamento de arquivos
 * necessarios ao servidor
 *
 * "$Id: $"
*/
#include "file_utils.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

const char * const IndexStr       = "/index.html";
const char * const PutMark        = ".~put";

#define PutMarkSize 5


void init_file_components(FileComponents *file)
{
  file->file_path[0] = '\0';
  file->file_ptr     = NULL;
  file->is_new_file  = 0;
  memset(&(file->stats), 0, sizeof(file->stats));
  file->should_delete = 1;
}

int32_t set_file_components(FileComponents *file,
                             char *file_path,
                             const int8_t flags)
{
  if (file == NULL)
  {
    printf("Trying to initialize a null FileComponent\n");
    return FileComponentNull;
  }

  file->file_ptr = NULL;

  if (file_path == NULL ||
      file_path[0] == '\0')
  {
    printf("Wrong file path\n");
    return FilePathNull;
  }

  get_file_stats(file_path, &(file->stats));

  if (flags & ReadFile)
  {
    if (!is_regular_file(file))
    {
      return NotARegularFile;
    }

    read_treatment(file, file_path);
  }
  else if (flags & WriteFile)
  {
    if (is_directory(file))
    {
      return NotARegularFile;
    }

    int8_t ret = write_treatment(file, file_path);
    if (ret == ExistentFile)
    {
      return ret;
    }
  }
  else
  {
    printf("Unknown flags");
    return UnknownFlag;
  }

  if (file->file_ptr == NULL)
  {
    return CouldntOpen;
  }

  return Success;
}

void read_treatment(FileComponents *file, char *file_path)
{
  size_t file_path_size = strlen(file_path);
  strncpy(file->file_path, file_path, file_path_size);
  file->file_path[file_path_size] = '\0';
  file->file_ptr = fopen(file->file_path, "rb");
}

int8_t write_treatment(FileComponents *file, char *file_path)
{
  snprintf(file->file_path, PATH_MAX, "%s%s", file_path, PutMark);

  if (get_file_stats(file->file_path, &(file->stats)))
  {
    return ExistentFile;
  }

  if (get_file_stats(file_path, &(file->stats)))
  {
    file->is_new_file = 0;
  }
  else
  {
    file->is_new_file = 1;
  }
  file->file_ptr = fopen(file->file_path, "wb");

  return Success;
}

int32_t destroy_file_components(FileComponents *file)
{
  if (file->file_ptr != NULL)
  {
    fclose(file->file_ptr);
  }

  file->file_path[0] = '\0';
  file->should_delete = 1;
  memset(&(file->stats), 0, sizeof(file->stats));
  return 0;
}

void setup_file_path(char *base_path, char *resource, char *full_path)
{
  int32_t resource_size = strlen(resource);
  if ((strncmp(resource, "/", resource_size) == 0) ||
      (strncmp(resource, ".", resource_size) == 0) )
  {
    strncpy(resource, IndexStr, strlen(IndexStr));
  }

  resource_size = strlen(resource);
  const int32_t path_size      = strlen(base_path);
  const int32_t file_name_size = path_size + resource_size + 1;

  snprintf(full_path, file_name_size, "%s%s", base_path, resource);
}

int8_t verify_file_path(char *base_path, char *full_path)
{
  const int32_t path_size = strlen(base_path);

  char real_path[PATH_MAX];
  memset(real_path, '\0', PATH_MAX);
  realpath(full_path, real_path); /**/

  if (*real_path != '\0')
  {
    if (strncmp(base_path, real_path, path_size) != 0)
    {
      char work_directory[PATH_MAX];
      if (getcwd(work_directory, PATH_MAX)!= NULL)
      {
        if (strncmp(work_directory, real_path, path_size) != 0)
        {
          struct stat buf;
          if ((stat(full_path, &buf) == -1) &&
              !S_ISLNK(buf.st_mode))
          {
            perror("stat");
            goto clear_full_path;
          }

          if (strncmp(work_directory, full_path, path_size) == 0)
          {
            return 0;
          }
          goto clear_full_path;
        }
      }
      else
      {
        goto clear_full_path;
      }
    }

    if (strncmp(real_path, full_path, strlen(full_path)) != 0)
    {
      goto clear_full_path;
    }
  }
  else
  {
    goto clear_full_path;
  }

  memset(full_path, '\0', PATH_MAX);
  strncpy(full_path, real_path, strlen(real_path) + 1);
  return 0;

clear_full_path:
  memset(full_path, '\0', PATH_MAX);
  return 1;
}

int32_t get_file_mime(uint32_t full_path_size, char *full_path, char *mime)
{
  char *cmd_mask = "file -i %s";
  int32_t total_size = strlen(cmd_mask) + full_path_size;

  FILE* pipe = NULL;
  char *cmd = malloc(sizeof(char)*(total_size));
  {
    snprintf(cmd, total_size, cmd_mask, full_path);
    pipe = popen(cmd, "r");
  }
  free(cmd);

  if (pipe == NULL)
  {
    return -1;
  }

  fscanf(pipe, "%*s %128[^\n]s\n", mime); /*MAX_MIME_SIZE*/
  pclose(pipe);
  return 0;
}


int32_t erase_put_mark(FileComponents *file)
{
  uint32_t name_size = strlen(file->file_path);
  if (name_size < PutMarkSize)
  {
    printf("Invalid Name\n");
    return -1;
  }

  uint32_t name_size_without_put_mark  = name_size - PutMarkSize;
  if (strncmp(file->file_path + name_size_without_put_mark,
              PutMark,
              PutMarkSize) != 0)
  {
    printf("Invalid file name %s\n", file->file_path);
    return -1;
  }

  char old_file_name[PATH_MAX];
  strncpy(old_file_name,file->file_path, name_size);
  strncpy(file->file_path, old_file_name, name_size_without_put_mark);
  old_file_name[name_size]       = '\0';
  file->file_path[name_size_without_put_mark] = '\0';

  if (rename(old_file_name, file->file_path) < 0)
  {
    printf("old_name: %s\n", old_file_name);
    printf("new name: %s\n", file->file_path);
    perror("rename");
    return -1;
  }
  return 0;
}

int32_t remove_file(FileComponents *file)
{
  if (file->file_ptr != NULL)
  {
    if (remove(file->file_path) < 0)
    {
      printf("remove file:");
      return -1;
    }
  }
  return 0;
}

int32_t treat_file_after_put(FileComponents *file, uint8_t error)
{
  if (file == NULL)
  {
    printf("Null file component received\n");
    return -1;
  }

  if (error)
  {
    return remove_file(file);
  }
  else
  {
    if (file->file_ptr != NULL)
    {
      fclose(file->file_ptr);
      file->file_ptr = NULL;
    }

    return erase_put_mark(file);
  }
}

uint8_t get_file_stats(char *file_path, struct stat *file_stats)
{
  return (stat(file_path, file_stats) == 0);
}

uint8_t is_valid_file(FileComponents *file)
{
  uint8_t  ret = (file->file_ptr == NULL) ? 0 : 1;
  return ret;
}

uint8_t is_regular_file(FileComponents *file)
{
  if (!S_ISREG(file->stats.st_mode))
  {
    return 0;
  }

  return 1;
}

uint8_t is_directory(FileComponents *file)
{
  if (file == NULL)
  {
    printf("file is null");
  }

  if (!S_ISDIR(file->stats.st_mode))
  {
    return 0;
  }

  return 1;
}
