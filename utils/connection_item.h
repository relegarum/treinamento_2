#ifndef CONNECTION_ITEM_H
#define CONNECTION_ITEM_H

#include <stdio.h>
#include <stdint.h>

#define END_OF_HEADER_SIZE    4

extern const char *const EndOfHeader;

extern const char *const IndexStr;

extern const char *const RequestMsgMask;

enum ConnectionStates
{
  Closed            = -1,
  Free              =  0,
  Receiving         =  1,
  SendingHeader     =  2,
  SendingResource   =  3,
  Sent              =  4,
  Handling          =  5
};

typedef struct ConnectionStruct
{
  int32_t  socket_descriptor;
  uint8_t  state;
  uint8_t  header_sent;
  uint8_t  error;
  uint64_t read_data;
  uint64_t wrote_data;
  uint64_t response_size;
  char     *request;
  FILE     *resource_file;
  char     *header;

  /*List*/
  struct ConnectionStruct *previous_ptr;
  struct ConnectionStruct *next_ptr;
} Connection;

void init_connection_item(Connection *item, int socket_descriptor);
Connection *create_connection_item(int socket_descriptor);


int32_t receive_request_blocking(Connection *item);
int32_t receive_request(Connection *item, const uint32_t transmission_rate);
void handle_request(Connection *item, char *path);
int32_t send_response(Connection *item, uint32_t transmission_rate);
int32_t send_header_blocking(Connection *item);
int32_t send_header(Connection *item, uint32_t transmission_rate);
int32_t send_resource(Connection *item, uint32_t transmission_rate);
int32_t get_resource_data(Connection *item, char *file_name, char *mime);
void setup_header(Connection *item, char *mime);


int8_t verify_file_path(char *path, char *resource, char *full_path);
int32_t get_file_mime(uint32_t full_path_size, char *full_path, char *mime);

void free_connection_item(Connection *item);

FILE *bad_request_file;
FILE *not_found_file;
FILE *internal_error_file;
FILE *unauthorized_file;
FILE *wrong_version_file;

#endif /* CONNECTION_ITEM_H*/
