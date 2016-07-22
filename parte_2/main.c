/* \file main.c
 *
 * \brief Este programa deverá ser um daemon para rodar em Linux. Ele deverá
 * utilizar APIs padrão Berkeley Sockets, e deverá ser centrado em torno da
 * função select ou poll. Este daemon não deverá criar novas threads ou
 * processos para cada conexão, mas sim manter o estado de cada uma delas em
 * uma lista de conexões ativas. O objetivo é familiarizar o novo programador
 * com as técnicas de sockets não bloqueantes, e com outras necessárias ao
 * desenvolvimento de daemons de rede de alta-performance. Como parâmetros de
 *  linha de comando, o programa deve receber uma porta para escutar conexões
 *  e um diretório para ser tratado como raiz do servidor web. Apenas os erros
 * de existência de arquivo e falta de permissões de acesso devem ser
 * reportados ao cliente.
 *
 *
 * "$Id: $"
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int32_t handle_arguments(int argc, char **argv, char **port, char **path)
{
  const int32_t index_of_executable = 0;
  const int32_t index_of_port       = 1;
  const int32_t index_of_path       = 2;
  const int32_t min_valid_port      = 1024;
  const int32_t max_valid_port      = 65535;

  if (argc < 3)
  {
    printf(" usage: %s port path", argv[index_of_executable]);
    return -1;
  }

  int32_t port_value = atoi(argv[index_of_port]);
  if( port_value < min_valid_port || port_value > max_valid_port )
  {
    printf(" invalid value for port: %d", port_value );
    return -1;
  }

  *port = argv[index_of_port];
  *path = argv[index_of_path];

  return 0;
}

int main(int argc, char *argv[])
{
  struct addrinfo *servinfo          = NULL;
  struct addrinfo *serverinfo_ptr    = NULL;
  int32_t listening_sock_description = -1;
  int32_t new_socket_description     = -1;

  char *port;
  char *path;

  int success = 0;
  if (handle_arguments( argc, argv, &port, &path) == -1)
  {
    success = 1;
    goto exit;
  }

  const int32_t           true_value      = 1;
  const int32_t number_of_connections     = 100;
  struct addrinfo         hints;
  struct sockaddr_storage client_address;
  socklen_t               sin_size;
  char client_address_string[INET6_ADDRSTRLEN];


  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  if ((success = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
  {
    printf("Error in getaddrinfo: %s\n", gai_strerror(success));
    goto exit;
  }

  // Get valid socket to listen
  for (serverinfo_ptr = servinfo;
       serverinfo_ptr != NULL;
       serverinfo_ptr = serverinfo_ptr->ai_next)
  {
    if ((listening_sock_description = socket(serverinfo_ptr->ai_family,
                                             serverinfo_ptr->ai_socktype,
                                             serverinfo_ptr->ai_protocol)) == -1)
    {
      perror("Server socket\n");
      continue;
    }

    if ((success = setsockopt(listening_sock_description,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              &true_value,
                              sizeof(true_value))) == -1)
    {
      perror("setsockopt");
      goto exit;
    }

    if (bind(listening_sock_description,
             serverinfo_ptr->ai_addr,
             serverinfo_ptr->ai_addrlen) == -1)
    {
      close(listening_sock_description);
      perror("server bind");
      continue;
    }

    break;
  }

  if (serverinfo_ptr == NULL)
  {
    printf("Failed to bind\n");
    success = -1;
    goto exit;
  }
  freeaddrinfo(servinfo);

  if (listen(listening_sock_description, number_of_connections) == -1)
  {
    perror("Listen\n");
    success = -1;
    goto exit;
  }

  printf("server: waiting for connections...\n");

  while(1)
  {
    sin_size = sizeof(client_address);
    new_socket_description = accept(listening_sock_description,
                                    (struct sockaddr *)&client_address,
                                    &sin_size);

    if (new_socket_description == -1)
    {
      perror("accept");
      continue;
    }

    inet_ntop(client_address.ss_family,
              get_in_addr((struct sockaddr *)&client_address),
              client_address_string,
              sizeof(client_address_string));
    printf("server: go connection from %s\n", client_address_string);
    close(new_socket_description);
  }

  success = 0;
exit:
  if (servinfo != NULL)
  {
    freeaddrinfo(servinfo);
  }

  if (listening_sock_description != -1)
  {
    close(listening_sock_description);
  }

  return success;
}
