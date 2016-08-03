#include "request_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


request_list_node* create_request(FILE *file,
                                 uint32_t id,
                                 int32_t datagram_socket,
                                 uint32_t data_size,
                                 uint32_t offset,
                                 uint8_t operation)
{
  request_list_node *node = malloc(sizeof(request_list_node));

  init_node(node,
            file,
            id,
            datagram_socket,
            data_size,
            offset,
            operation);

  return node;
}

void init_node(request_list_node *node,
               FILE *file,
               uint32_t id,
               int32_t datagram_socket,
               uint32_t data_size,
               uint32_t offset,
               uint8_t operation)
{
  node->file      = file;
  node->buffer[0] = '\0';
  node->id        = id;
  node->data_size = data_size;
  node->operation = operation;
  node->offset    = offset;
  node->datagram_socket = datagram_socket;

  /*list*/
  node->previous_ptr = NULL;
  node->next_ptr     = NULL;
}


void destroy_node(request_list_node *node)
{
  node->file      = NULL;
  node->buffer[0] = '\0';
  node->data_size = 0;
  node->id        = 0;
  node->operation = None;
  if (node->datagram_socket != -1)
  {
    close(node->datagram_socket);
    node->datagram_socket = -1;
  }
}
