#include "connection_item.h"
#include <stdlib.h>

void init_connection_item(Connection *item, int socket_descriptor)
{
  item->socket_description = socket_descriptor;
  item->state              = Free;
  item->wroteData          = 0;
  item->response_size      = 0;
  item->resource_file      = NULL;
  item->request            = NULL;
  item->header             = NULL;
  item->next_ptr           = NULL;
  item->previous_ptr       = NULL;
}

void free_connection_item(Connection *item)
{
  item->socket_description = -1;
  item->state              = Closed;
  item->wroteData          = 0;
  item->response_size      = 0;
  item->next_ptr           = NULL;
  item->previous_ptr       = NULL;

  if (item->resource_file != NULL)
  {
    fclose(item->resource_file);
    item->resource_file = NULL;
  }

  if (item->header != NULL)
  {
    free(item->header);
    item->header = NULL;
  }

  if (item->request != NULL)
  {
    free(item->request);
    item->request = NULL;
  }
}

Connection *create_connection_item(int socket_descriptor)
{
  Connection *item = malloc(sizeof(Connection));
  init_connection_item(item, socket_descriptor);
  return item;
}
