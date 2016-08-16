/* \file connection_manager.c
 *
 * \brief Contem a declaracao funcoes de manipulacao da lista de conexoes
 * ativas.
 *
 * "$Id: $"
*/
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

int get_greatest_socket_descriptor(ConnectionManager *manager);
useconds_t calculate_time_to_sleep(const ConnectionManager *manager,
                                   const struct timeval *lowest,
                                   const int8_t allinactive);

#endif
