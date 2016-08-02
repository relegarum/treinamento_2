#include "request_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_NAME_SIZE 4


request_list_node* create_request(FILE *file,
                                 char *buffer,
                                 uint32_t id,
                                 uint32_t data_size,
                                 uint32_t offset,
                                 uint8_t operation)
{
  request_list_node *node = malloc(sizeof(request_list_node));
  init_node(node, file, buffer, id, data_size, offset, operation);
  return node;
}

void init_node(request_list_node *node,
               FILE *file,
               char *buffer,
               uint32_t id,
               uint32_t data_size,
               uint32_t offset,
               uint8_t operation)
{
  node->file      = file;
  node->buffer    = buffer;
  node->id        = id;
  node->data_size = data_size;
  node->operation = operation;
  node->offset    = offset;

  node->datagram_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
  /* HANDLE THIS ERROR */
  node->name.sun_family = AF_UNIX;
  snprintf(node->name.sun_path, MAX_NAME_SIZE, "%d", node->id);
  //strncpy(this->name.sun_path,itoa(this->id), MAX_NAME_SIZE);
  /*this->name.sun_len = strlen(this->name.sun_path);*/

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
