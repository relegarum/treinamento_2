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
#include <limits.h>
#include <dirent.h>

#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>


#include "../utils/handle_settings.h"
#include "../utils/connection_manager.h"
#include "../utils/connection_item.h"
#include "../utils/http_utils.h"
#include "../utils/request_manager.h"
#include "../utils/thread.h"
#include "../utils/test_suit.h"
#include "../utils/Config.h"


#define MAX_TIMEOUT 9999
#define MAX_RETRIES 1000
#define NUMBER_OF_THREADS 8

int signal_operation = 0;

enum SignalOperationEnum
{
  Terminate = -1,
  ReadFileSignal  =  1
};

void handle_signal(int signal_number)
{
  printf("Signal %d\n", signal_number);

  if (signal_number == SIGUSR1)
  {
    signal_operation = ReadFile;
    return;
  }

  signal_operation = Terminate;
  return;
}

void setup_threads(thread *thread_pool,
                   const uint32_t pool_size,
                   request_manager *manager)
{
  uint32_t index = 0;
  for (;index < pool_size; ++index)
  {
    init_thread(&(thread_pool[index]), manager, index);
  }
}

void handle_socket_destroy(int *socket,
                           ConnectionManager *manager,
                           int *greatest,
                           fd_set *master)
{
  close_socket(socket, master);
  if (*socket == *greatest)
  {
    *greatest = get_greatest_socket_descriptor(manager);
  }
}

int32_t prepare_port(char *port,
                     int32_t *listener_socket,
                     const uint32_t number_of_connections)
{
  if (setup_listening_connection(port, listener_socket) == -1)
  {
    return -1;
  }

  if (listen(*listener_socket, number_of_connections) == -1)
  {
    perror("Listen\n");
    return -1;
  }

  return 0;
}

int main(int argc, char **argv)
{
  Config* config = create_config();

  //daemon(0 , 0);
  int32_t listening_sock_description = -1;
  int32_t transmission_rate = 0;

  char *port = NULL;
  char path[PATH_MAX];
  memset(path, '\0', PATH_MAX);

  ConnectionManager manager = create_manager();

  request_manager req_manager = create_request_manager();

  thread thread_pool[NUMBER_OF_THREADS];
  setup_threads(thread_pool, NUMBER_OF_THREADS, &req_manager);
  start_thread_pool(thread_pool, NUMBER_OF_THREADS);


  const int32_t number_of_connections     = 300;
  int success = 0;
  if (handle_arguments(argc, argv, &port, path, &transmission_rate) == -1)
  {
    success = 1;
    goto exit;
  }

  write_into_config_file(config, path, port, transmission_rate, getpid());

  create_default_response_files(path);

  if (prepare_port(port,
                   &listening_sock_description,
                   number_of_connections) == -1)
  {
    success = -1;
    goto exit;
  }

  signal(SIGINT, handle_signal);
  signal(SIGUSR1, handle_signal);

  printf("server: waiting for connections...\n");

  int    greatest_file_desc;
  fd_set master;
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);
  FD_SET(listening_sock_description, &master);
  greatest_file_desc = listening_sock_description;

  struct timeval timeout;
  struct timeval lowest;
  lowest.tv_sec = INT_MAX;
  timeout.tv_sec = MAX_TIMEOUT;
  timeout.tv_usec = 0;

  while (1)
  {
    if (signal_operation == Terminate)
    {
      goto exit;
    }

    if (signal_operation == ReadFileSignal)
    {
      char new_port[MAX_PORT_SIZE];
      signal_operation = 0;
      read_config_file(config, path, new_port, &transmission_rate);
      printf("New configuration!\n"
             "Path: %s\n"
             "Port: %s\n"
             "Transmission Rate %d\n", path, new_port, transmission_rate);
      if (strncmp(new_port, port, MAX_PORT_SIZE) != 0)
      {
        strncpy(port, new_port, MAX_PORT_SIZE);
        handle_socket_destroy(&listening_sock_description,
                              &manager,
                              &greatest_file_desc,
                              &master);
        prepare_port(port, &listening_sock_description, number_of_connections);
        FD_SET(listening_sock_description, &master);
      }
    }

    read_fds   = master;
    write_fds  = master;
    except_fds = master;

    int ret = select(greatest_file_desc + 1,
                     &read_fds,
                     &write_fds,
                     &except_fds,
                     &timeout);

    if ((ret == -1) || FD_ISSET(listening_sock_description, &except_fds) )
    {
      if (errno != EINTR)
      {
        perror("select error");
      }
      continue;
    }

    int8_t allinactive = 1;

    if (verify_connection(&manager,
                          listening_sock_description,
                          &read_fds,
                          &master,
                          &greatest_file_desc) == -1)
    {
      continue;
    }

    lowest.tv_sec = INT_MAX;
    lowest.tv_usec = INT_MAX;

    Connection *ptr = manager.head;
    while (ptr != NULL)
    {

      if (verify_if_has_to_exchange_data(ptr))
      {
        if (FD_ISSET(ptr->socket_descriptor, &read_fds))
        {
          if (ptr->state == Free ||
              ptr->state == Receiving)
          {
            allinactive &= 0;
            if (receive_request(ptr, transmission_rate) == -1)
            {
              ptr->state = Closed;
            }
          }

          if ((ptr->state == ReceivingFromPut))
          {
            if (receive_data_from_put(ptr, transmission_rate) == -1)
            {
              ptr->state = Closed;
            }
          }

          if (ptr->partial_read + BUFSIZ > (uint32_t )transmission_rate)
          {
            gettimeofday(&(ptr->last_connection_time), NULL);
            ptr->partial_read = 0;
          }
        }

        if (ptr->state == Handling)
        {
          handle_request(ptr, path);
        }

        if (FD_ISSET(ptr->socket_descriptor, &write_fds))
        {
          allinactive &= 0;
          if (ptr->state == SendingHeader)
          {
            if (send_header(ptr, transmission_rate) == -1)
            {
              ptr->state = Closed;
            }
          }

          if (ptr->state == SendingResource)
          {
            if (send_response(ptr, transmission_rate) == -1)
            {
              ptr->state = Closed;
            }
          }

          if (ptr->partial_wrote + BUFSIZ > (uint32_t )transmission_rate)
          {
            gettimeofday(&(ptr->last_connection_time), NULL);
            ptr->partial_wrote = 0;
          }
        }
      }

      if (ptr->state == WritingIntoFile)
      {
        queue_request_to_write(ptr, &req_manager, &master, &greatest_file_desc);
      }

      if (ptr->state == ReadingFromFile)
      {
        queue_request_to_read(ptr,
                              &req_manager,
                              transmission_rate,
                              &master,
                              &greatest_file_desc);
      }

      if (ptr->state == WaitingFromIORead
          && FD_ISSET(ptr->datagram_socket, &read_fds))
      {
        receive_from_thread_read(ptr, transmission_rate);
        if (ptr->state != ReadingFromFile)
        {
          handle_socket_destroy(&ptr->datagram_socket,
                                &manager,
                                &greatest_file_desc,
                                &master);
        }
      }

      if (ptr->state == WaitingFromIOWrite
          && FD_ISSET(ptr->datagram_socket, &read_fds))
      {

        receive_from_thread_write(ptr);
        handle_socket_destroy(&ptr->datagram_socket,
                              &manager,
                              &greatest_file_desc,
                              &master);

      }

      if (timercmp(&(ptr->last_connection_time), &lowest, <))
      {
        lowest.tv_sec = ptr->last_connection_time.tv_sec;
        lowest.tv_usec = ptr->last_connection_time.tv_usec;
      }

      verify_connection_state(ptr);

      if (ptr->state == Sent ||
          ptr->state == Closed)
      {
        Connection *next = ptr->next_ptr;

        handle_socket_destroy(&ptr->socket_descriptor,
                              &manager,
                              &greatest_file_desc,
                              &master);

        handle_socket_destroy(&ptr->datagram_socket,
                              &manager,
                              &greatest_file_desc,
                              &master);
        remove_connection_in_list(&manager, ptr);
        ptr = next;
      }
      else
      {
        ptr = ptr->next_ptr;
      }
    }

    useconds_t time_to_sleep = calculate_time_to_sleep(&manager,
                                                       &lowest,
                                                       allinactive);
    if (time_to_sleep != 0 )
    {
      usleep(time_to_sleep);
    }
  }

  success = 0;

exit:
  clean_default_files();

  free_request_list(&req_manager);
  free_list(&manager);
  join_thread_pool(thread_pool, NUMBER_OF_THREADS);

  if (listening_sock_description != -1)
  {
    close(listening_sock_description);
  }

  release_config(&config);
  config = NULL;
  return success;
}
