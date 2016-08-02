#ifndef CONNECTION_ITEM_H
#define CONNECTION_ITEM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include "request_manager.h"

#define END_OF_HEADER_SIZE    4

extern const char *const EndOfHeader;
extern const char *const RequestMsgMask;

enum ConnectionStates
{
  Closed            = -1,
  Free              =  0,
  Receiving         =  1,
  SendingHeader     =  2,
  SendingResource   =  3,
  Sent              =  4,
  Handling          =  5,
  ReadingFromFile   =  6,
  WritingIntoFile   =  7
};

typedef struct ConnectionStruct
{
  int32_t         socket_descriptor;
  uint32_t        id;
  uint8_t         state;
  uint8_t         header_sent;
  uint8_t         error;
  uint64_t        read_data;
  uint64_t        wrote_data;
  uint64_t        response_size;
  uint64_t        partial_read;
  uint32_t        partial_wrote;
  uint32_t        read_file_data;
  char            buffer[BUFSIZ];
  char            *request;
  FILE            *resource_file;
  char            *header;
  struct timeval  last_connection_time;

  /*List*/
  struct ConnectionStruct *previous_ptr;
  struct ConnectionStruct *next_ptr;
} Connection;

void init_connection_item(Connection *item, int socket_descriptor, uint32_t id);
Connection *create_connection_item(int socket_descriptor, uint32_t id);


int32_t receive_request_blocking(Connection *item);
int32_t receive_request(Connection *item, const uint32_t transmission_rate);
void handle_request(Connection *item, char *path);
int32_t send_response(Connection *item, uint32_t transmission_rate);
int32_t send_header_blocking(Connection *item);
int32_t send_header(Connection *item, const uint32_t transmission_rate);
int32_t send_resource(Connection *item, const int32_t transmission_rate);
int32_t get_resource_data(Connection *item, char *file_name, char *mime);
void setup_header(Connection *item, char *mime);
int8_t is_active(Connection *item);

void queue_request_to_read(Connection *item,
                           request_manager *manager,
                           const uint32_t rate);

int32_t read_data_from_file(Connection *item, const uint32_t transmission_rate);
void wrote_data_into_file(char *buffer,
                          const uint32_t rate,
                          FILE *resource_file);


void free_connection_item(Connection *item);

FILE *bad_request_file;
FILE *not_found_file;
FILE *internal_error_file;
FILE *unauthorized_file;
FILE *wrong_version_file;
FILE *not_implemented_file;

#endif /* CONNECTION_ITEM_H*/
