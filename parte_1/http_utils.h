#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H
#include <stdint.h>

enum HTTP_STATUS
{
  Ok               = 200,
  Not_Found        = 404,
  MovedPermanently = 301,
  MovedTemporarily = 202,
  ServerError      = 500,
};

uint32_t get_response_size(char *first_chunk);

uint32_t handle_response_status(char *content);

int32_t get_header(int socket_descriptor,
                   char *resource_required,
                   int32_t *header_length,
                   int32_t *content_length);

void get_resource(char *uri, char *hostname, char *resource);

int32_t download_file(int socket_descriptor, char *resource_required, int32_t content_length, char *content_buffer);

#endif // HTTP_UTILS_H
