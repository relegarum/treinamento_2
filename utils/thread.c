#include "thread.h"
#include <unistd.h>



void init_thread(thread *this, request_manager *manager, int32_t id)
{
  this->manager = manager;
  this->id = id;
  this->ret = 0;
}

void start_thread(thread *this)
{
  pair_manager_id pair;
  pair.manager =  this->manager;
  pair.id =  this->id;
  pthread_create(&(this->pthread), NULL, &do_thread, (void *)(&pair));
}

/*void clean_thread(thread *this)
{
  printf("cleaning %d", this->id);
  pthread_exit(&(this->ret));
}*/

void *do_thread(void *this)
{
  pair_manager_id *pair = (pair_manager_id *)this;
  request_manager *manager = pair->manager;
  /*int32_t id = pair->id;*/

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
