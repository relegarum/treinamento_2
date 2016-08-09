#ifndef REQUEST_LIST_H
#define REQUEST_LIST_H
#include <stdio.h>
#include <stdint.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>


enum Operation
{
  None  = -1,
  Read  = 0,
  Write = 1
};

typedef struct request_list_node_struct
{
  FILE     *file;
  char     buffer[BUFSIZ];
  uint8_t  operation;
  uint32_t data_size;
  uint32_t offset;

  /*SOCK_DGRAM*/
  int    datagram_socket;
  struct sockaddr_un name;

  /*list*/
  struct request_list_node_struct *previous_ptr;
  struct request_list_node_struct *next_ptr;

}request_list_node;

request_list_node* create_request_to_read(FILE* file,
                                          int32_t datagram_socket,
                                          uint32_t data_size,
                                          uint32_t offset);


request_list_node *create_request_to_write(FILE *file,
                                           char *buffer,
                                           int32_t datagram_socket,
                                           uint32_t data_size,
                                           uint32_t offset);

void init_node(request_list_node *node,
               FILE *file,
               char *buffer,
               int32_t datagram_socket,
               uint32_t data_size,
               uint32_t offset,
               uint8_t operation);

void destroy_node(request_list_node *node);

#endif /* REQUEST_LIST_H */
