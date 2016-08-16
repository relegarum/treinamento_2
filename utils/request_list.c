/* \file request_list.c
 *
 * \brief Contem a implementacao dos metodos manipulacao das requisicoes de IO
 * feitas pela thread principal para as threads secundarias.
 *
 * "$Id: $"
*/
#include "request_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


request_list_node *create_request_to_read(FILE *file,
                                          int32_t datagram_socket,
                                          uint32_t data_size,
                                          uint64_t offset)
{
  request_list_node *node = malloc(sizeof(request_list_node));

  init_node(node,
            file,
            NULL,
            datagram_socket,
            data_size,
            offset,
            Read);

  return node;
}

request_list_node *create_request_to_write(FILE *file,
                                           char *buffer,
                                           int32_t datagram_socket,
                                           uint32_t data_size,
                                           uint64_t offset)
{
  request_list_node *node = malloc(sizeof(request_list_node));

  init_node(node,
            file,
            buffer,
            datagram_socket,
            data_size,
            offset,
            Write);

  return node;
}

void init_node(request_list_node *node,
               FILE *file,
               char *buffer,
               int32_t datagram_socket,
               uint32_t data_size,
               uint64_t offset,
               uint8_t operation)
{
  node->file      = file;
  if (buffer != NULL)
  {
    memcpy(node->buffer, buffer, data_size);
  }
  else
  {
    node->buffer[0] = '\0';
  }

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
  node->operation = None;
  if (node->datagram_socket != -1)
  {
    close(node->datagram_socket);
    node->datagram_socket = -1;
  }
  free(node);
}
