#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H
#include <pthread.h>
#include "request_list.h"


typedef struct request_manager_struct
{
  request_list_node *head;
  request_list_node *tail;
  uint32_t           size;
  pthread_mutex_t    mutex;
  pthread_cond_t     conditional_variable;
}request_manager;

request_manager create_request_manager();
void init_request_list(request_manager *manager);
void add_request_in_list(request_manager *manager,
                         request_list_node *new_item);

void remove_request_in_list(request_manager *manager,
                            request_list_node *item);

void free_request_list(request_manager *manager);

#endif // REQUEST_MANAGER_H
