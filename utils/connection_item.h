#ifndef CONNECTION_ITEM_H
#define CONNECTION_ITEM_H
#include <stdio.h>
#include <stdint.h>

enum ConnectionStates
{
  Closed            = -1,
  Free              =  0,
  Receiving         =  1,
  SendingHeader     =  2,
  SendingResource   =  3,
  Sent              =  4,
  Handling          =  5
};

typedef struct ConnectionStruct
{
  int32_t  socket_descriptor;
  uint8_t  state;
  uint8_t  header_sent;
  uint8_t  error;
  uint64_t read_data;
  uint64_t wrote_data;
  uint64_t response_size;
  char     *request;
  FILE     *resource_file;
  char     *header;

  /*List*/
  struct ConnectionStruct *previous_ptr;
  struct ConnectionStruct *next_ptr;
} Connection;

void init_connection_item(Connection *item, int socket_descriptor);

Connection *create_connection_item(int socket_descriptor);
void free_connection_item(Connection *item);

#endif /* CONNECTION_ITEM_H*/
