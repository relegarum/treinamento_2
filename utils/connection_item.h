#ifndef CONNECTION_ITEM_H
#define CONNECTION_ITEM_H
#include <stdio.h>
#include <stdint.h>

enum ConnectionStates
{
  Closed    = -1,
  Free      =  0,
  Receiving =  1,
  Sending   =  2
};

typedef struct ConnectionStruct
{
  int32_t socket_description;
  int8_t  state;
  int32_t wroteData;
  int32_t response_size;
  char    *request;
  FILE    *resource_file;
  char    *header;

  /*List*/
  struct ConnectionStruct *previous_ptr;
  struct ConnectionStruct *next_ptr;
} Connection;

void init_connection_item(Connection *item, int socket_descriptor);
Connection *create_connection_item(int socket_descriptor);
void free_connection_item(Connection *item);

#endif /* CONNECTION_ITEM_H*/
