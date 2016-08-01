/* \file test_suit.c
 *
 * \brief Suite de testes para garantia de funcionamento de algumas features
 *
 * "$Id: $"
*/

#include <limits.h>
#include <string.h>

#include "file_utils.h"
#include "http_utils.h"
#include "connection_item.h"
#include "connection_manager.h"
#include "thread.h"
#include "request_manager.h"
#define NUMBER_OF_THREADS 8


void test_connection_manager()
{
  ConnectionManager manager;
  init_list(&manager);
  Connection *item1 = create_connection_item(1, 1);
  Connection *item2 = create_connection_item(2, 2);
  Connection *item3 = create_connection_item(3, 3);
  Connection *item10 = create_connection_item(10, 10);

  if (manager.size != 0)
  {
    printf("Error in creation\n");
  }

  add_connection_in_list(&manager, item1);
  add_connection_in_list(&manager, item2);
  add_connection_in_list(&manager, item3);
  add_connection_in_list(&manager, item10);

  if (manager.size != 4)
  {
    printf("Error in add\n");
  }

  remove_connection_in_list(&manager, item1);

  if (manager.size != 3)
  {
    printf("Error in delete");
  }

  free_list(&manager);

  if ( manager.head != NULL ||
       manager.tail != NULL )
  {
    printf("Error in free");
  }
}


int32_t test_verify_path()
{
  char *test_return = "/home/abaiao/repo/treinamento/build_server/index.html";
  char *path = "/home/abaiao/repo/treinamento/build_server";
  char *resourceOk = "/index.html";
  char *resourceWrongWithReturn = "/../index.html";
  char *resourceOkWithReturn = "/release/../index.html";
  char *resourceOkWithReturn2 = "/release/teste/../../index.html";
  char *resourceWrong = "/index3.html";

  char full_path[PATH_MAX];
  if (verify_file_path( path, resourceOk, full_path) != 0)
  {
    printf("Error test Ok");
    return 1;
  }

  if (strncmp(test_return,full_path, strlen(test_return)) != 0 )
  {
    printf("Error test Ok");
    return 1;
  }

  if (verify_file_path( path, resourceWrongWithReturn, full_path) == 0)
  {
    printf("Error test Ok");
    return 1;
  }

  if (verify_file_path( path, resourceOkWithReturn, full_path) != 0)
  {
    printf("Error test Ok");
    return 1;
  }

  if (strncmp(test_return,full_path, strlen(test_return)) != 0 )
  {
    printf("Error test Ok");
    return 1;
  }

  if (verify_file_path( path, resourceOkWithReturn2, full_path) != 0)
  {
    printf("Error test Ok");
    return 1;
  }

  if (strncmp(test_return,full_path, strlen(test_return)) != 0 )
  {
    printf("Error test Ok");
    return 1;
  }

  if (verify_file_path( path, resourceWrong, full_path) == 0)
  {
    printf("Error test Ok");
    return 1;
  }

  return 0;
}



void setup_threads_T(thread *thread_pool, const uint32_t pool_size, request_manager *manager)
{
  uint32_t index = 0;
  for (;index < pool_size; ++index)
  {
    init_thread(&(thread_pool[index]), manager, index);
    start_thread(&(thread_pool[index]));
  }
}

/*void clean_threads_T(thread *thread_pool, const uint32_t pool_size)
{
  uint32_t index = 0;
  for (;index < pool_size; ++index)
  {
    clean_thread(&(thread_pool[index]));
  }
}*/

void test_threads()
{
  request_manager request_manager = create_request_manager();
  thread thread_pool[NUMBER_OF_THREADS];
  setup_threads_T(thread_pool, NUMBER_OF_THREADS, &request_manager);

  sleep(5);

  request_list_node *request = create_request(NULL, NULL, 255, 10, Read);
  add_request_in_list(&request_manager, request);

  request_list_node *request2 = create_request(NULL, NULL, 254, 11, Read);
  add_request_in_list(&request_manager, request2);

  request_list_node *request3 = create_request(NULL, NULL, 253, 12, Read);
  add_request_in_list(&request_manager, request3);

  sleep(5);

  free_request_list(&request_manager);
}
