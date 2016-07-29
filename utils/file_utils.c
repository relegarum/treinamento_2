#include "file_utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

const char * const IndexStr       = "/index.html";

int8_t verify_file_path(char *path, char *resource, char *full_path)
{
  int32_t resource_size = strlen(resource);
  if( strncmp(resource, "/", resource_size) == 0 ||
      strncmp(resource, ".", resource_size) == 0 )
  {
    strncpy(resource, IndexStr, strlen(IndexStr));
  }

  // build string
  resource_size = strlen(resource);
  const int32_t path_size      = strlen(path);
  const int32_t file_name_size = path_size + resource_size + 1;
  char real_path[PATH_MAX];
  memset(real_path, '\0', PATH_MAX);
  snprintf(full_path, file_name_size, "%s%s", path, resource);
  if  (realpath(full_path, real_path) != NULL )
  {
    if (strncmp(path, real_path, path_size) != 0)
    {
      char work_directory[PATH_MAX];
      if (getcwd(work_directory, PATH_MAX)!= NULL)
      {
        if (strncmp(work_directory, real_path, path_size) != 0)
        {
          goto clear_full_path;
        }
      }
      else
      {
        goto clear_full_path;
      }
    }
  }
  else
  {
    printf("Directory not found\n");
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

