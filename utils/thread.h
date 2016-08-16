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


void init_thread(thread *this_thread, request_manager *manager, int32_t id);
void start_thread(thread *this_thread);
void start_thread_pool(thread *thread_pool, const uint32_t pool_size);
void join_thread_pool(thread *thread_pool, const uint32_t pool_size);
/*void clean_thread(thread *this);*/
void *do_thread(void *arg);
void handle_request_item(request_list_node *item);
void read_from_file(request_list_node *item);
void write_into_file(request_list_node *item);


#endif /* THREAD_H*/
