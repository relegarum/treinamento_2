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
#include <signal.h>
#include <time.h>

#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "../utils/connection_manager.h"
#include "../utils/connection_item.h"
#include "../utils/http_utils.h"

//#include "../utils/test_suit.h"

int8_t terminated = 0;

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

void setup_deamon()
{
  pid_t pid;
  pid_t sid;

  pid = fork();
  if (pid < 0)
  {
    exit(EXIT_FAILURE);
  }

  if (pid > 0)
  {
    exit(EXIT_SUCCESS);
  }

  umask(0);
  sid = setsid();
  if (sid < 0)
  {
    exit(EXIT_FAILURE);
  }

  if ((chdir("/")) < 0)
  {
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}


void handle_sigint(int signal_number)
{
  if (signal_number == SIGINT)
  {
    terminated = 1;
  }

  if (signal_number == SIGABRT)
  {
    printf("Abort!");
  }
  terminated = 1;
}

int main(int argc, char **argv)
{
  //test_verify_path();
  //setup_deamon();
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

  create_default_response_files(path,
                                &bad_request_file,
                                &not_found_file,
                                &internal_error_file,
                                &unauthorized_file,
                                &wrong_version_file);

  const int32_t number_of_connections     = 200;
  if( setup_listening_connection(port, &listening_sock_description) == -1 )
  {
    success = -1;
    goto exit;
  }

  if (listen(listening_sock_description, number_of_connections) == -1)
  {
    perror("Listen\n");
    success = -1;
    goto exit;
  }

  signal(SIGINT, handle_sigint);
  signal(SIGABRT, handle_sigint);

  printf("server: waiting for connections...\n");

  const int32_t transmission_rate = BUFSIZ; // 1Mb/s
  int    greatest_file_desc;
  fd_set master;
  fd_set read_fds;
  fd_set write_fds;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_SET(listening_sock_description, &master);
  greatest_file_desc = listening_sock_description;

  /*time_t one_second_ms = 1000;
  time_t begin;
  time_t end;*/

  struct timeval timeout;

  while (1)
  {
    if (terminated)
    {
      goto exit;
    }
    read_fds  = master;
    write_fds = master;
    int ret = select(greatest_file_desc + 1, &read_fds, &write_fds, NULL, &timeout) == -1;
    /*if (ret == 0)
    {
      if (timeout.tv_sec == 999999)
      {
        time_t teste = 1000000-timeout.tv_usec;
        usleep(teste);
      }
      timeout.tv_sec = 999999;
    }
    else*/ if (ret == -1)
    {
      perror("select error");
      success = 4; /* Change this number */
      goto exit;
    }

    if (verify_connection(&manager, listening_sock_description, &read_fds, &master, &greatest_file_desc) == -1)
    {
      continue;
    }

    //time(&begin);
    {
      Connection *ptr = manager.head;
      while (ptr != NULL)
      {
        if ((ptr->state == Free ||
             ptr->state == Receiving ) &&
            FD_ISSET(ptr->socket_descriptor, &read_fds))
        {
          if (receive_request(ptr, transmission_rate) == -1)
          {
            success = -1;
            goto exit;
          }
        }

        if (ptr->state == Handling )
        {
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
          //printf("Socket = %d closed\n\n", ptr->socket_descriptor);
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
    }
    //time(&end);
    //double diff_trunc = difftime(begin, end);
    //int32_t mseconds = diff_trunc*1000;
    //time_t
    //time_t teste = (one_second_ms - diff_trunc)*1000;
    //usleep(teste);
//    if (diff_trunc > 0)
//    {
//      time_t teste = (one_second_ms - diff_trunc)*1000;
//      usleep(teste);
//    }
//    else
//    {
//      sleep(1);
//    }
  }

  success = 0;
exit:

  clean_default_files();

  free_list(&manager);

  if (listening_sock_description != -1)
  {
    close(listening_sock_description);
  }

  return success;
}
