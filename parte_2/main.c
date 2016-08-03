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

#include "../utils/connection_manager.h"
#include "../utils/connection_item.h"
#include "../utils/http_utils.h"
#include "../utils/request_manager.h"
#include "../utils/thread.h"
#include "../utils/test_suit.h"

#define MAX_TIMEOUT 9999
#define NUMBER_OF_THREADS 8

ConnectionManager* manager_ptr = NULL;
request_manager*   request_manager_pr = NULL;
int32_t* listening_socket_ptr  = NULL;

int32_t handle_arguments(int argc,
                         char **argv,
                         char **port,
                         char **path,
                         int32_t* transmission_rate)
{
  const int32_t index_of_executable         = 0;
  const int32_t index_of_port               = 1;
  const int32_t index_of_path               = 2;
  const int32_t index_of_transmission_rate  = 3;
  const int32_t min_valid_port              = 1024;
  const int32_t max_valid_port              = 65535;

  if (argc < 3)
  {
    printf(" usage: %s port path transmission_rate\n",
           argv[index_of_executable]);
    return -1;
  }

  int32_t port_value = atoi(argv[index_of_port]);
  if( port_value < min_valid_port || port_value > max_valid_port )
  {
    printf(" invalid value for port: %d!\n"
           " Please use a port between - 1024 and 65535.\n", port_value );
    return -1;
  }

  *port = argv[index_of_port];
  *path = argv[index_of_path];

  DIR *dir = opendir(*path);
  if (dir == NULL)
  {
    printf(" invalid path! Please use a valid path!\n");
    return -1;
  }
  closedir(dir);

  if (argc < 4)
  {
    printf(" Transmission Rate not passed, setting 8kbps as default\n");
    *transmission_rate = BUFSIZ;
    return 0;
  }
  else
  {
    char *end_ptr = "\0";
    *transmission_rate = strtol(argv[index_of_transmission_rate], &
                                end_ptr, 10);
    if (*transmission_rate <= 0)
    {
      printf(" Transmission Rate unknown, setting 8kbps as default\n");
      *transmission_rate = BUFSIZ;
    }
  }

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

int terminate = 0;

void handle_sigint(int signal_number)
{
  printf("signal free1");

  terminate =1;

  return;
  if (signal_number == SIGINT)
  {
    clean_default_files();

  }

  if (manager_ptr != NULL)
  {
    printf("signal free");
    free_list(manager_ptr);
  }

  if (request_manager_pr != NULL)
  {
    free_request_list(request_manager_pr);
  }

  if (( listening_socket_ptr != NULL ) &&
      (*listening_socket_ptr != -1))
  {
    close(*listening_socket_ptr);
  }

  exit(1);
}

void setup_threads(thread *thread_pool, const uint32_t pool_size, request_manager *manager)
{
  uint32_t index = 0;
  for (;index < pool_size; ++index)
  {
    init_thread(&(thread_pool[index]), manager, index);
  }
}

void start_threads(thread *thread_pool, const uint32_t pool_size)
{
  uint32_t index = 0;
  for (;index < pool_size; ++index)
  {
    start_thread(&(thread_pool[index]));
  }
}

/*void clean_threads(thread *thread_pool, const uint32_t pool_size)
{
  uint32_t index = 0;
  for (;index < pool_size; ++index)
  {
    clean_thread(&(thread_pool[index]));
  }
}*/


int main(int argc, char **argv)
{
  //sleep(15);
  /*setup_deamon();*/
  int32_t listening_sock_description = -1;
  int32_t transmission_rate    = 0;

  char *port = NULL;
  char *path = NULL;

  ConnectionManager manager = create_manager();
  manager_ptr = &manager;

  request_manager req_manager = create_request_manager();
  request_manager_pr = &req_manager;

  thread thread_pool[NUMBER_OF_THREADS];
  setup_threads(thread_pool, NUMBER_OF_THREADS, &req_manager);
  start_threads(thread_pool, NUMBER_OF_THREADS);


  int success = 0;
  if (handle_arguments(argc, argv, &port, &path, &transmission_rate) == -1)
  {
    success = 1;
    goto exit;
  }

  create_default_response_files(path,
                                &bad_request_file,
                                &not_found_file,
                                &internal_error_file,
                                &unauthorized_file,
                                &wrong_version_file,
                                &not_implemented_file);

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
  listening_socket_ptr = &listening_sock_description;

  signal(SIGINT, handle_sigint);

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

  /*time_t begin;
  time_t end;*/
  while (1)
  {
    if (terminate)
    {
      printf("terminate");
      goto exit;
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
      perror("select error");
      printf("teste");
      success = -1;
      goto exit;
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
      if ((ptr->state == Free ||
           ptr->state == Receiving ) &&
          FD_ISSET(ptr->socket_descriptor, &read_fds))
      {
        struct timeval next;
        next.tv_sec = ptr->last_connection_time.tv_sec + 1;
        next.tv_usec = ptr->last_connection_time.tv_usec;
        struct timeval now;
        gettimeofday(&now, NULL);
        if (timercmp(&now, &next, >))
        {
          allinactive &= 0;
          if (receive_request(ptr, transmission_rate) == -1)
          {
            success = -1;
            goto exit;
          }

          if (ptr->partial_read  >= (uint32_t )transmission_rate)
          {
            gettimeofday(&(ptr->last_connection_time), NULL);
            lowest.tv_sec = ptr->last_connection_time.tv_sec;
            lowest.tv_usec = ptr->last_connection_time.tv_usec;
            ptr->partial_read = 0;
          }
        }
      }

      if (ptr->state == Handling)
      {
        handle_request(ptr, path);
      }

      if (FD_ISSET(ptr->socket_descriptor, &write_fds))
      {
        struct timeval next;
        next.tv_sec = ptr->last_connection_time.tv_sec + 1;
        next.tv_usec = ptr->last_connection_time.tv_usec;
        struct timeval now;
        gettimeofday(&now, NULL);
        if (timercmp(&now, &next, >))
        {
          allinactive &= 0;
          if (ptr->state == SendingHeader)
          {
            send_header(ptr, transmission_rate);
          }

          if (ptr->state == SendingResource)
          {
            send_response(ptr, transmission_rate);
          }

          if (ptr->partial_wrote >= (uint32_t )transmission_rate)
          {
            gettimeofday(&(ptr->last_connection_time), NULL);
            lowest.tv_sec = ptr->last_connection_time.tv_sec;
            lowest.tv_usec = ptr->last_connection_time.tv_usec;
            ptr->partial_wrote = 0;
          }
        }
      }

      if (ptr->state == ReadingFromFile)
      {
        queue_request_to_read(ptr, &req_manager, transmission_rate);
        //read_data_from_file(ptr, transmission_rate);
      }

      if (ptr->state == WaitingFromIO)
      {
        receive_from_thread(ptr, transmission_rate);
      }

      if (timercmp(&(ptr->last_connection_time), &lowest, <))
      {
        lowest.tv_sec = ptr->last_connection_time.tv_sec;
        lowest.tv_usec = ptr->last_connection_time.tv_usec;
      }

      if (ptr->state == Sent)
      {
        Connection *next = ptr->next_ptr;
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

    if (manager.size != 0)
    {
      if (((manager.size == 1) &&
          (manager.head->state == Free)) ||
          !allinactive)
      {
        continue;
      }
      struct timeval now;
      gettimeofday(&now, 0);

      struct timeval one_second_later;
      one_second_later.tv_sec =  lowest.tv_sec + 1;
      one_second_later.tv_usec = lowest.tv_usec;
      if(timercmp(&one_second_later, &now, >))
      {
        struct timeval time_to_sleep;
        timersub(&one_second_later, &now, &time_to_sleep);
        usleep(time_to_sleep.tv_usec);
      }
    }
  }

  success = 0;

exit:
  clean_default_files();

  free_request_list(&req_manager);
  free_list(&manager);

  int index = 0;
  for ( index = 0; index < NUMBER_OF_THREADS; ++index)
  {
    pthread_join(thread_pool[index].pthread, NULL);
  }

  if (listening_sock_description != -1)
  {
    close(listening_sock_description);
  }

  return success;
}
