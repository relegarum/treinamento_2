#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H
#include "connection_item.h"

typedef struct ConnectionManagerStruct
{
  Connection *head;
  Connection *tail;
  int32_t     size;
}ConnectionManager;

ConnectionManager create_manager();
void init_list(ConnectionManager *manager);
void add_connection_in_list(ConnectionManager *manager, Connection *new_item);
void remove_connection_in_list(ConnectionManager *manager, Connection *item);
void free_list(ConnectionManager *manager);

#endif
