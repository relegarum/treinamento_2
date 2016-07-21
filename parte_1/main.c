/* \file main.c
 *
 * \brief Treinamento Este programa deverá ser operado por linha de comando e
 * compilado sobre Linux. Ele deverá receber como parâmetro uma URI completa e
 * um nome de arquivo, e em seguida conectar-se ao servidor, recuperar a
 * página e salvá-la no arquivo informado.
 * Não é permitido usar libfetch ou outras bibliotecas equivalentes.
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
                     FILE **output)
{
  const int32_t number_of_elements = 3;
  if (argc < number_of_elements)
  {
    printf("wrong number of arguments");
    exit(1);
  }

  struct addrinfo hints, *res;
  //char ip_string[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  char hostname[50];
  get_resource(argv[1], hostname, resource_required);

  /*Get */
  int32_t status = getaddrinfo(hostname, "80", &hints, &res);
  if (status != 0)
  {
    printf("getaddrinfo: %s\n", gai_strerror(status));
    exit(1);
  }

  /*printf("IP adresses for %s :", hostname);

  struct addrinfo *p;
  for ( p = res; p != NULL; p = p->ai_next )
  {
    void *addr;
    char ip_version[ 50 ];

    if ( p->ai_family == AF_INET )
    {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &( ipv4->sin_addr );
      strncpy( ip_version, "IPv4", 5 );
    }
    else
    {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &( ipv6->sin6_addr );
      strncpy( ip_version, "IPv6", 5 );
    }

    inet_ntop(p->ai_family, addr, ip_string, sizeof(ip_string));
    printf( " %s: %s\n", ip_version, ip_string );
  }*/

  *server_info = res;

  /* Verify the overwrite flag existence */
  const char* overwrite_flag = "over";
  char* file_name = argv[2];
  uint8_t ovewrite = 0;
  if ((argc == 4) &&
     (strncmp( argv[3], overwrite_flag, strlen(overwrite_flag)) == 0))
  {
    ovewrite = 1;
  }

  return handle_file(file_name, ovewrite, output);
}

int main(int argc, char **argv)
{
  struct addrinfo *server_info = NULL;
  FILE            *output_file = NULL;
  char		         resource_required[ 50 ];

  if (handle_arguments(argc, argv, &server_info, resource_required, &output_file) != 0)
  {
    printf( "Couldn't handle arguments\n" );
    return 1;
  }

  int socket_descriptor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
  if (socket_descriptor == -1)
  {
    perror("socket");
    exit(1);
  }

  printf("Trying to Connect..");

  int status = connect(socket_descriptor, server_info->ai_addr, server_info->ai_addrlen);
  if (status == -1)
  {
    perror("connect");
    exit(1);
  }

  printf("Connected!\n");

  const int32_t transmission_rate = 512;
  if (download_file(socket_descriptor, resource_required, transmission_rate, output_file ) != 0)
  {
    close( socket_descriptor );
    freeaddrinfo(server_info);
    fclose(output_file);
    return -1;
  }

  close( socket_descriptor );
  freeaddrinfo(server_info);
  fclose(output_file);
  return 0;
}

