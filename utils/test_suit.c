/* \file test_suit.c
 *
 * \brief Suite de testes para garantia de funcionamento de algumas features
 *
 * "$Id: $"
*/

#include <limits.h>
#include <string.h>

#include "http_utils.h"
#include "connection_item.h"
#include "connection_manager.h"


void test_connection_manager()
{
  ConnectionManager manager;
  init_list(&manager);
  Connection *item1 = create_connection_item(1);
  Connection *item2 = create_connection_item(2);
  Connection *item3 = create_connection_item(3);
  Connection *item10 = create_connection_item(10);

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
