/* \file connection_item.c
 *
 * \brief Contem a declaracao de funcoes de criacao e manipulacao de conexoes
 * ativas.
 *
 * "$Id: $"
*/
#ifndef CONNECTION_ITEM_H
#define CONNECTION_ITEM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include "file_utils.h"

#include "request_manager.h"

#define END_OF_HEADER_SIZE    4

extern const char *const HeaderBadRequest;
extern const char *const HeaderOk;
extern const char *const HeaderCreated;
extern const char *const HeaderConflict;
extern const char *const HeaderNotFound;
extern const char *const HeaderInternalError;
extern const char *const HeaderUnauthorized;
extern const char *const HeaderWrongVersion;
extern const char *const HeaderNotImplemented;
extern const char *const HeaderForbidden;

extern const char *const EndOfHeader;
extern const char *const RequestMsgMask;

enum ConnectionStates
{
  Closed             = -1,
  Free               =  0,
  Receiving          =  1,
  SendingHeader      =  2,
  SendingResource    =  3,
  Sent               =  4,
  Handling           =  5,
  ReadingFromFile    =  6,
  WritingIntoFile    =  7,
  WaitingFromIORead  =  8,
  WaitingFromIOWrite =  9,
  ReceivingFromPut   =  10,
  TreatingGetMethod  =  11,
  TreatingPutMethod  =  12,
  FirsState          = Closed,
  LastState          = TreatingPutMethod
};

enum ConnectionMethods
{
  Unknown = 0,
  Get     = 1,
  Put     = 2
};

typedef struct ConnectionStruct
{
  int32_t         socket_descriptor;
  uint32_t        id;
  int8_t          state;
  uint8_t         header_sent;
  uint8_t         error;
  uint64_t        read_data;
  uint64_t        wrote_data;
  uint64_t        resource_size;
  uint64_t        partial_read;
  uint64_t        partial_wrote;
  uint64_t        read_file_data;
  uint64_t        data_to_write_size;
  uint8_t         method;
  uint8_t         tries;
  int32_t         datagram_socket;
  char            buffer[BUFSIZ];
  char            *request;
  FileComponents  file_components;
  char            *header;
  char            *end_of_header;
  struct timeval  last_connection_time;

  /*List*/
  struct ConnectionStruct *previous_ptr;
  struct ConnectionStruct *next_ptr;
} Connection;

/* Construtor and Destrutor */
void init_connection_item(Connection *item, int socket_descriptor, uint32_t id);
Connection *create_connection_item(int socket_descriptor, uint32_t id);
void free_connection_item(Connection *item);

/* Receive function set */
int32_t receive_request(Connection *item, const uint32_t transmission_rate);

/* Send function set */
int32_t send_response(Connection *item, uint32_t transmission_rate);
int32_t send_header(Connection *item, const uint32_t transmission_rate);
int32_t send_resource(Connection *item, const int32_t transmission_rate);

/* Utils */
int32_t verify_if_has_to_exchange_data(Connection* item);
int32_t get_resource_data(Connection *item, char *file_name, char *mime);
void setup_header(Connection *item, char *mime);
void handle_request(Connection *item, char *path);
void handle_new_socket(int new_socket_description,
                       fd_set *master,
                       int *greatest_socket_description);

void close_socket(int *socket,
                  fd_set *master);

int32_t handle_get_method(Connection *item, char *resource, char *full_path);
int32_t handle_put_method(Connection *item, char *resouce, char *full_path);
int32_t get_file_state(Connection *item);
int32_t extract_content_length_from_header(Connection *item);

/*PUT related functions*/
/*void write_data_into_file(Connection *item,
                          FILE *resource_file);*/

int32_t receive_data_from_put(Connection *item,
                              const uint32_t transmission_rate);

void queue_request_to_write(Connection *item,
                            request_manager *manager,
                            fd_set *master,
                            int *greatest_socket_description);

void receive_from_thread_write(Connection *item);

/* Thread related functions */
void queue_request_to_read(Connection *item,
                           request_manager *manager,
                           const uint32_t transmission_rate,
                           fd_set *master,
                           int *greatest_socket_description);

void queue_request_to_write(Connection *item,
                            request_manager *manager,
                            fd_set *master,
                            int *greatest_socket_description);

void receive_from_thread_read(Connection *item,
                              const uint32_t transmission_rate);

void receive_from_thread_write(Connection *item);

void verify_connection_state(Connection *item);

#endif /* CONNECTION_ITEM_H*/
