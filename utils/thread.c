/* \file threads.c
 *
 * \brief Contem a implementacao das funcoes de manipulacao das threads assim
 * como a acao executada pela thread em si: funcao do_thread
 *
 *
 * "$Id: $"
*/
#include "thread.h"
#include <unistd.h>
#include <stdlib.h>

void init_thread(thread *this_thread, request_manager *manager, int32_t id)
{
  this_thread->manager = manager;
  this_thread->id = id;
  this_thread->ret = 0;
  ++(this_thread->manager->number_of_threads);
  printf("Thread init: %d size:%d\n", id, this_thread->manager->size);
}

void start_thread(thread *this_thread)
{
  pthread_create(&(this_thread->pthread),
                 NULL,
                 &do_thread,
                 (void *)(this_thread));
}

void start_thread_pool(thread *thread_pool, const uint32_t pool_size)
{
  uint32_t index = 0;
  for (;index < pool_size; ++index)
  {
    start_thread(&(thread_pool[index]));
  }
}

void *do_thread(void *arg)
{
  thread *this_thread = (thread *)(arg);
  request_manager *manager = this_thread->manager;
  int32_t         id       = this_thread->id;
  request_list_node *item  = NULL;

  while (1)
  {
    pthread_mutex_lock(&(manager->mutex));
    {
      while (manager->size == 0)
      {
        pthread_cond_wait(&(manager->conditional_variable), &(manager->mutex));
        if (manager->exit)
        {
          --(manager->number_of_threads);
          pthread_cond_broadcast(&(manager->conditional_variable));
          pthread_mutex_unlock(&(manager->mutex));

          printf("Thread dying: %d\n", id);
          return NULL;
        }
      }

      item = manager->head;
      remove_request_in_list(manager, item);
    }
    pthread_mutex_unlock(&(manager->mutex));

    handle_request_item(item /*, id*/);
    destroy_node(item);
  }
}


void handle_request_item(request_list_node *item /*,int32_t id */)
{
  if (item->operation == Read)
  {
    read_from_file(item);
  }
  else
  {
    write_into_file(item);
  }
}

void read_from_file(request_list_node *item)
{
  fseek(item->file, item->offset, SEEK_SET);

  int32_t read_data =  fread(item->buffer,
                             sizeof(char),
                             item->data_size,
                             item->file);

  if (read_data < 0)
  {
    printf("fread error\n");
    perror(__FUNCTION__);
  }
  else
  {
    /*printf("thread %d\n", id);*/
    if (write(item->datagram_socket, item->buffer, read_data) < 0)
    {
      printf("write error\n");
      perror(__FUNCTION__);
    }
  }
}

void write_into_file(request_list_node *item)
{
  fseek(item->file, item->offset, SEEK_SET);
  uint32_t data_wrote = fwrite(item->buffer,
                               sizeof(char),
                               item->data_size,
                               item->file);

  if (data_wrote == 0 )
  {
    perror(__FUNCTION__);
    data_wrote = 0;
  }
  fflush(item->file);
  /*printf("%s", item->buffer);*/

  if (write(item->datagram_socket, &data_wrote, sizeof(data_wrote)) < 0)
  {
    printf("write error\n");
    perror(__FUNCTION__);
  }
}


void join_thread_pool(thread *thread_pool, const uint32_t pool_size)
{
  uint32_t index = 0;
  for (index = 0; index < pool_size; ++index)
  {
    pthread_join(thread_pool[index].pthread, NULL);
  }
}
