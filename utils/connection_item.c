#include "connection_item.h"
#include <stdlib.h>
#include <unistd.h>

void init_connection_item(Connection *item, int socket_descriptor)
{
  item->socket_descriptor = socket_descriptor;
  item->state              = Free;
  item->header_sent        = 0;
  item->error              = 0;
  item->wrote_data          = 0;
  item->read_data          = 0;
  item->response_size      = 0;
  item->resource_file      = NULL;
  item->request            = NULL;
  item->header             = NULL;
  item->next_ptr           = NULL;
  item->previous_ptr       = NULL;
}

void free_connection_item(Connection *item)
{
  item->socket_descriptor = -1;
  item->state              = Closed;
  item->header_sent        = 0;
  item->read_data          = 0;
  item->wrote_data         = 0;
  item->response_size      = 0;
  item->next_ptr           = NULL;
  item->previous_ptr       = NULL;

  if (item->socket_descriptor != -1)
  {
    close(item->socket_descriptor);
    item->socket_descriptor = -1;
  }

  if (item->error != 1)
  {
    if (item->resource_file != NULL)
    {
      fclose(item->resource_file);
      item->resource_file = NULL;
    }
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
