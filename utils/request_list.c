#include "request_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_NAME_SIZE 4


request_list_node* create_request(FILE *file,
                                 char *buffer,
                                 uint32_t id,
                                 uint32_t data_size,
                                 uint8_t operation)
{
  request_list_node *node = malloc(sizeof(request_list_node));
  init_node(node, file, buffer, id, data_size, operation);
  return node;
}

void init_node(request_list_node *this,
               FILE *file,
               char *buffer,
               uint32_t id,
               uint32_t data_size,
               uint8_t operation)
{
  this->file      = file;
  this->buffer    = buffer;
  this->id        = id;
  this->data_size = data_size;
  this->operation = operation;

  this->datagram_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
  /* HANDLE THIS ERROR */
  this->name.sun_family = AF_UNIX;
  snprintf(this->name.sun_path, MAX_NAME_SIZE, "%d", this->id);
  //strncpy(this->name.sun_path,itoa(this->id), MAX_NAME_SIZE);
  /*this->name.sun_len = strlen(this->name.sun_path);*/

  /*list*/
  this->previous_ptr = NULL;
  this->next_ptr     = NULL;
}


void destroy_node(request_list_node *this)
{
  this->file      = NULL;
  this->buffer[0] = '\0';
  this->data_size = 0;
  this->id        = 0;
  this->operation = None;
  if (this->datagram_socket != -1)
  {
    close(this->datagram_socket);
    this->datagram_socket = -1;
  }
}
