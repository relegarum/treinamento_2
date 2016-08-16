/* \file connection_manager.c
 *
 * \brief Contem a implementacao funcoes de manipulacao da lista de conexoes
 * ativas.
 *
 * "$Id: $"
*/
#include "connection_manager.h"
#include <stdlib.h>

void add_connection_in_list(ConnectionManager *manager, Connection *new_item)
{
  ++(manager->size);
  if (manager->head == NULL)
  {
    manager->head = new_item;
    manager->tail = new_item;
  }
  else
  {
    new_item->previous_ptr = manager->tail;
    manager->tail->next_ptr = new_item;
    manager->tail = new_item;
  }
}

void remove_connection_in_list(ConnectionManager *manager, Connection *item)
{
  if (item == NULL)
  {
    printf("\nNull connection cannot be removed\n");
    return;
  }

  --(manager->size);
  if ((item->previous_ptr != NULL) &&
      (item->next_ptr != NULL))
  {
    item->previous_ptr->next_ptr = item->next_ptr;
    item->next_ptr->previous_ptr = item->previous_ptr;
  }
  else if ((item->previous_ptr == NULL) && /*Only Header case*/
           (item->next_ptr == NULL))
  {
    free_connection_item(item);
    /*free(item);*/
    /*item = NULL; Check this*/
    manager->head = NULL;
    manager->tail = NULL;
    return;
  }
  else if (item->previous_ptr == NULL) /*Header case*/
  {
    item->next_ptr->previous_ptr = NULL;
    manager->head = item->next_ptr;
  }
  else if (item->next_ptr == NULL) /*Tail case*/
  {
    item->previous_ptr->next_ptr = NULL;
    manager->tail = item->previous_ptr;
  }

  free_connection_item(item);
  /*free(item);*/
  /*item = NULL; Check this*/
}

void free_list(ConnectionManager *manager)
{
  while(manager->head != NULL)
  {
    remove_connection_in_list(manager, (manager->head));
  }
}

void init_list(ConnectionManager *manager)
{
  manager->head = NULL;
  manager->tail = NULL;
  manager->size = 0;
}

ConnectionManager create_manager()
{
  ConnectionManager manager;
  init_list(&manager);
  return manager;
}

int get_greatest_socket_descriptor(ConnectionManager *manager)
{
  int greatest = 0;
  Connection *ptr = manager->head;
  while (ptr != NULL)
  {
    if (ptr->socket_descriptor > greatest)
    {
      greatest = ptr->socket_descriptor;
    }

    if (ptr->datagram_socket > greatest)
    {
      greatest = ptr->datagram_socket;
    }

    ptr = ptr->next_ptr;
  }

  return greatest;
}

useconds_t calculate_time_to_sleep(const ConnectionManager *manager,
                                   const struct timeval *lowest,
                                   const int8_t allinactive)
{
  if (manager->size != 0)
  {
    if (((manager->size == 1) &&
         (manager->head->state == Free)) ||
        !allinactive)
    {
      return 0;
    }
    struct timeval now;
    gettimeofday(&now, 0);

    struct timeval one_second_later;
    one_second_later.tv_sec =  lowest->tv_sec + 1;
    one_second_later.tv_usec = lowest->tv_usec;
    if(timercmp(&one_second_later, &now, >))
    {
      struct timeval time_to_sleep;
      timersub(&one_second_later, &now, &time_to_sleep);
      return time_to_sleep.tv_usec;
    }
  }

  return 0;
}
