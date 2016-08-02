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
                 (void *)(this_thread));
  pthread_attr_destroy(&attr);
}

void *do_thread(void *arg)
{
  pthread_detach(pthread_self());
  thread *this_thread = (thread *)(arg);
  request_manager *manager = this_thread->manager;
  int32_t         id       = this_thread->id;

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

    handle_request_item(item, id);
  }
}


void handle_request_item(request_list_node *item, int32_t id)
{
  fseek(item->file, item->offset, SEEK_SET);

  int32_t read_data =  fread(item->buffer,
                             sizeof(char),
                             item->data_size,
                             item->file);

  /*printf("thread %d\n", id);*/
  if (write(item->datagram_socket, item->buffer, read_data) < 0)
  {
    perror(" sendto error:");
  }

  destroy_node(item);
}
