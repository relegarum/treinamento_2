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
#include <fcntl.h>

#include "../utils/connection_manager.h"
#include "../utils/connection_item.h"
#include "../utils/http_utils.h"


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

void teste_connection_manager()
{
  ConnectionManager manager;
  init_list(&manager);
  Connection *item1 = create_connection_item(1);
  Connection *item2 = create_connection_item(2);
  Connection *item3 = create_connection_item(3);
  Connection *item10 = create_connection_item(10);

  add_connection_in_list(&manager, item1);
  add_connection_in_list(&manager, item2);
  add_connection_in_list(&manager, item3);
  add_connection_in_list(&manager, item10);

  remove_connection_in_list(&manager, item1);

  free_list(&manager);
}

int main(int argc, char **argv)
{
  struct addrinfo *servinfo          = NULL;
  struct addrinfo *serverinfo_ptr    = NULL;
  int32_t listening_sock_description = -1;

  char *port;
  char *path;

  ConnectionManager manager = create_manager();

  int success = 0;
  if (handle_arguments(argc, argv, &port, &path) == -1)
  {
    success = 1;
    goto exit;
  }

  const int32_t           true_value      = 1;
  const int32_t number_of_connections     = 100;
  struct addrinfo         hints;


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
  serverinfo_ptr = NULL;

  if (listen(listening_sock_description, number_of_connections) == -1)
  {
    perror("Listen\n");
    success = -1;
    goto exit;
  }

  printf("server: waiting for connections...\n");

  const int32_t transmission_rate = BUFSIZ;
  int    greatest_file_desc;
  fd_set master;
  fd_set read_fds;
  fd_set write_fds;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_SET(listening_sock_description, &master);
  greatest_file_desc = listening_sock_description;

  while (1)
  {
    read_fds  = master;
    write_fds = master;
    if (select(greatest_file_desc + 1, &read_fds, &write_fds, NULL, NULL) == -1)
    {
      perror("select error");
      success = 4;
      goto exit;
    }

    if (verify_connection(&manager, listening_sock_description, &read_fds, &master, &greatest_file_desc) == -1)
    {
      continue;
    }

    //int32_t index = 0;
    Connection *ptr = manager.head;
    while (ptr != NULL)
    {
      if (FD_ISSET(ptr->socket_descriptor, &read_fds))
      {
        if (receive_request(ptr, transmission_rate) == -1)
        {
          success = -1;
          goto exit;
        }

        handle_request(ptr, path);
      }

      if (ptr->state == Sending &&
          (FD_ISSET(ptr->socket_descriptor, &write_fds)) )
      {
        send_response(ptr, transmission_rate);
      }

      if (ptr->state == Sent)
      {
        Connection *next = ptr->next_ptr;
        printf("Socket = %d closed\n\n", ptr->socket_descriptor);
        close(ptr->socket_descriptor);
        FD_CLR(ptr->socket_descriptor, &master);
        remove_connection_in_list(&manager, ptr);
        ptr = next;
      }
      else
      {
        ptr = ptr->next_ptr;
      }
    }


    /*for (;index <= greatest_file_desc; ++index)
    {
      if (FD_ISSET(index, &read_fds))
      {
        if (index == listening_sock_description)
        {
          continue;
        }
        else
        {
          Connection* item = create_connection_item((index));

          add_connection_in_list(&manager, item);

          if (receive_request(item, transmission_rate) == -1)
          {
            success = -1;
            goto exit;
          }

          handle_request(item, path);
          send_response(item, &master, transmission_rate);

          remove_connection_in_list(&manager,item);
        }
      }
    }*/
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
  free_list(&manager);

  return success;
}
