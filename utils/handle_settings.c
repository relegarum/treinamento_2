/* \file handle_settings.c
 *
 * \brief Contem a implementacao de funcoes de validação de settings
 * ativas.
 *
 * "$Id: $"
*/
#include "handle_settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include "http_utils.h"
#include "dirent.h"

#define MIN_PORT 1024
#define MAX_PORT 65535
#define MAX_PORT_SIZE 5
#define MAX_TRANSMISSION_RATE_SIZE 10

int32_t handle_arguments(int argc,
                         char **argv,
                         char *port,
                         char *path,
                         int32_t* transmission_rate)
{
  const int32_t index_of_executable         = 0;
  const int32_t index_of_port               = 1;
  const int32_t index_of_path               = 2;
  const int32_t index_of_transmission_rate  = 3;

  if (argc < index_of_transmission_rate)
  {
    printf(" usage: %s port path transmission_rate\n",
           argv[index_of_executable]);
    return -1;
  }

  if(verify_if_valid_port(argv[index_of_port]) == -1)
  {
    printf(" invalid value for port: %s!\n"
           " Please use a port between - 1024 and 65535.\n", argv[index_of_port] );
    return -1;
  }
  strncpy(port, argv[index_of_port], MAX_PORT_SIZE);

  if (verify_if_valid_path(argv[index_of_path]) == -1)
  {
    printf(" invalid path! Please use a valid path!\n");
    return -1;
  }

  if (strcmp(argv[index_of_path], ".") == 0)
  {
    if (getcwd(path, PATH_MAX) == NULL)
    {
      printf(" couldn't get the working directory\n");
      return -1;
    }
  }
  else
  {
    strncpy(path,argv[index_of_path], strlen(argv[index_of_path]));
  }

  if (argc < index_of_transmission_rate + 1)
  {
    printf(" Transmission Rate not passed, setting 8kbps as default\n");
    *transmission_rate = BUFSIZ;
    return 0;
  }

  if (treat_transmission_rate(argv[index_of_transmission_rate], transmission_rate) == -1)
  {
    printf(" Transmission Rate unknown, setting 8kbps as default\n");
    *transmission_rate = BUFSIZ;
  }

  return 0;
}

int32_t verify_if_valid_path(const char *path)
{
  DIR *dir = opendir(path);
  if (dir == NULL)
  {
    return -1;
  }
  closedir(dir);

  return 0;
}

int32_t verify_if_valid_port(const char *port)
{
  int32_t port_value = atoi(port);
  if( port_value < MIN_PORT || port_value > MAX_PORT )
  {
    return -1;
  }
  return 0;
}

int32_t get_real_path(const char *path, char *real_path)
{
  memset(real_path, '\0', PATH_MAX);
  if (realpath(path, real_path) == NULL)
  {
    return -1;
  }
  return 0;
}

int32_t treat_transmission_rate( const char *transmission_rate_string,
                                 int32_t *transmission_rate)
{
  char *end_ptr = '\0';
  *transmission_rate = strtol(transmission_rate_string,
                              &end_ptr,
                              MAX_TRANSMISSION_RATE_SIZE);
  if (*transmission_rate <= 0)
  {
    return- 1;
  }

  return 0;
}
