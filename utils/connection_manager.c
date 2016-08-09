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
  if( item == NULL)
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
