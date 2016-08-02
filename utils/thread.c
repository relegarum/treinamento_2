#include "thread.h"
#include <unistd.h>

void init_thread(thread *this_thread, request_manager *manager, int32_t id)
{
  this_thread->manager = manager;
  this_thread->id = id;
  this_thread->ret = 0;
  printf("Thread init: %d size:%d\n", id, this_thread->manager->size);
}

void start_thread(thread *this_thread)
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&(this_thread->pthread),
                 &attr,
                 &do_thread,
                 (void *)(this_thread->manager));
  pthread_attr_destroy(&attr);
}

/*void clean_thread(thread *this)
{
  printf("cleaning %d", this->id);
  pthread_exit(&(this->ret));
}*/

void *do_thread(void *arg)
{
  pthread_detach(pthread_self());
  request_manager *manager = (request_manager *)(arg);
  /*int32_t id = pair->id;*/

  //printf("Start thread %d\n", pair->id);
  //printf("Manager %d\n", manager->size);

  while (1)
  {
    while(manager == NULL)
    {
      printf("manager is null");
    }
    pthread_mutex_lock(&(manager->mutex));
    while (manager->size == 0)
    {
      pthread_cond_wait(&(manager->conditional_variable), &(manager->mutex));
      if (manager->exit)
      {
        pthread_exit(NULL);
      }
    }

    request_list_node *item = manager->head;
    remove_request_in_list(manager, item);
    pthread_mutex_unlock(&(manager->mutex));

    handle_request_item(item);
  }
}


void handle_request_item(request_list_node *item)
{
  printf(" Data Size: %d\n", item->data_size);
  printf(" Operation: %d\n", item->operation);
  printf(" Id: %d\n",      item->id);
}
