/* \file main.c
 *
 * \brief Treinamento Este programa deverá ser operado por linha de comando e
 * compilado sobre Linux. Ele deverá receber como parâmetro uma URI completa e
 * um nome de arquivo, e em seguida conectar-se ao servidor, recuperar a
 * página e salvá-la no arquivo informado.
 * Não é permitido usar libfetch ou outras bibliotecas equivalentes.
 * Teste string: (echo -e "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Length: `wc -c /bin/bash | cut -d ' ' -f 1`\r\n\r\n";cat /bin/bash) | sudo nc -l 80 > /tmp/output
 *
 *
 * "$Id: $"
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex.h>

#include "http_utils.h"

int handle_file(char* fileName, uint8_t overwrite, FILE **output)
{
  {
    struct stat buffer;
    int result = stat(fileName, &buffer);
    if (result != 0)
    {
      *output = fopen(fileName, "w");
    }
    else
    {
      if (overwrite == 1)
      {
        *output = fopen(fileName, "w");
      }
      else
      {
        printf("File already exist and the overwrite flags wasn't passed\n");
        return -1;
      }
    }
  }

  if (*output == NULL)
  {
    perror("Coudn't open file");
    return -1;
  }

  return 0;
}

int handle_arguments(int argc,
                     char **argv,
                     struct addrinfo **server_info,
                     char *resource_required,
                     char *hostname,
                     FILE **output)
{
  const int32_t  number_of_elements   = 3;
  const int32_t  number_of_elements_with_flag = 4;
  const uint32_t index_of_uri         = 1;
  const uint32_t index_of_output_file = 2;
  const uint32_t index_of_flag        = 3;


  if (argc < number_of_elements)
  {
    printf("wrong number of arguments");
    return -1;
  }

  struct addrinfo hints;
  struct addrinfo *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  get_resource(argv[index_of_uri], hostname, resource_required);

  /*Get addrinfo*/
  int32_t status = getaddrinfo(hostname, "80", &hints, &res);
  if (status != 0)
  {
    printf("getaddrinfo: %s\n", gai_strerror(status));
    return -1;
  }

  *server_info = res;

  /* Verify the overwrite flag existence */
  const char* overwrite_flag = "-o";
  char* file_name = argv[index_of_output_file];
  uint8_t ovewrite = 0;
  if ((argc == number_of_elements_with_flag) &&
     (strncmp(argv[index_of_flag], overwrite_flag, strlen(overwrite_flag)) == 0))
  {
    ovewrite = 1;
  }

  return handle_file(file_name, ovewrite, output);
}

int main(int argc, char **argv)
{
  struct addrinfo *server_info = NULL;
  FILE            *output_file = NULL;
  char		       resource_required[ 100 ];
  char		       hostname[ 100 ];

  int32_t ret =  0;
  int socket_descriptor = -1;
  if (handle_arguments(argc, argv, &server_info, resource_required, hostname, &output_file) != 0)
  {
    printf( "Couldn't handle arguments\n" );
    ret = -1;
    goto exit;
  }

  socket_descriptor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
  if (socket_descriptor == -1)
  {
    perror("socket");
    ret = -1;
    goto exit;
  }

  printf("Trying to Connect..");

  int status = connect(socket_descriptor, server_info->ai_addr, server_info->ai_addrlen);
  if (status == -1)
  {
    perror("connect");
    ret = -1;
    goto exit;
  }

  printf("Connected!\n");

  const int32_t transmission_rate = 512;
  if (download_file(socket_descriptor, hostname, resource_required, transmission_rate, output_file ) != 0)
  {
    ret = -1;
    goto exit;
  }

 exit:
  if( socket_descriptor != -1)
  {
    close( socket_descriptor );
  }

  if( server_info != NULL )
  {
    freeaddrinfo(server_info);
  }
  if( output_file != NULL )
  {
    fclose(output_file);
  }

  return ret;
}

