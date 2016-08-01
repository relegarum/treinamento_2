#include "thread.h"



void init_thread(thread *this, request_manager *manager, int32_t id)
{
  this->manager = manager;
  this->id = id;
  this->ret = 0;
}

void start_thread(thread *this)
{
  pthread_create(&(this->pthread), NULL, &do_thread, (void *)this->manager);
}

void clean_thread(thread *this)
{
  pthread_exit(&(this->ret));
}

void *do_thread(void *this)
{
  request_manager *manager = (request_manager *)this;
  while (1)
  {
    pthread_mutex_lock(&(manager->mutex));
    while (manager->size != 0)
    {
      pthread_cond_wait(&(manager->conditional_variable), &(manager->mutex));
    }
    request_list_node *item = manager->head;
    remove_request_in_list(manager, item);
    pthread_mutex_unlock(&(manager->mutex));
  }
}


void handle_request_item(request_list_node *item)
{
  printf("\nBuffer: %s", item->buffer);
  printf(" Data Size: %d", item->data_size);
  printf(" Operation: %d", item->operation);
}
