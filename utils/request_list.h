#ifndef REQUEST_LIST_H
#define REQUEST_LIST_H
#include <stdio.h>
#include <stdint.h>

enum Operation
{
  None  = -1,
  Read  = 0,
  Write = 1
};

typedef struct request_list_node_struct
{
  FILE     *file;
  char     *buffer;
  uint32_t id;
  uint8_t  operation;
  uint32_t data_size;

  /*list*/
  struct request_list_node_struct *previous_ptr;
  struct request_list_node_struct *next_ptr;

}request_list_node;

void init_node(request_list_node *this,
               FILE *file,
               char* buffer,
               uint32_t id,
               uint32_t data_size,
               uint8_t operation);

request_list_node* create_request(FILE* file,
                                 char* buffer,
                                 uint32_t id,
                                 uint32_t data_size,
                                 uint8_t operation);

void destroy_node(request_list_node *this);

#endif /* REQUEST_LIST_H */
