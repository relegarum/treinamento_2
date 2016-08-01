#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
#include <stdint.h>

#include "request_manager.h"

typedef struct thread_struct
{
  request_manager *manager;
  int32_t     id;
  int32_t     ret;
  pthread_t   pthread;
} thread;

typedef struct pair_manager_id_struct
{
  request_manager *manager;
  int32_t         id;
}pair_manager_id;

void init_thread(thread *this, request_manager *manager, int32_t id);
void start_thread(thread *this);
/*void clean_thread(thread *this);*/
void *do_thread(void *this);
void handle_request_item(request_list_node *item);


#endif /* THREAD_H*/
