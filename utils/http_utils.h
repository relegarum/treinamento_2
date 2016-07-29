#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "connection_manager.h"
#include "connection_item.h"

/* Possible response status from HTTP */
enum HTTP_STATUS
{
  Ok               = 200,
  Created          = 201,
  Accepted         = 202,
  NoContent        = 204,
  MovedPermanently = 301,
  MovedTemporarily = 302,
  NotModified      = 304,
  BadRequest       = 400,
  Unauthorized     = 401,
  Forbidden        = 403,
  Not_Found        = 404,
  ServerError      = 500,
  NotImplemented   = 501,
  BadGateway       = 502,
  ServiceUnavaible = 503
};

void *get_in_addr(struct sockaddr *sa);

uint32_t get_response_size(char *first_chunk);

uint32_t handle_response_status(char *http_response);

int32_t get_header(int socket_descriptor,
                   char *resource_required,
                   int32_t *header_length,
                   int32_t *content_length);

int32_t handle_header(int socket_descriptor, int32_t *header_length , int32_t *content_size);

void get_resource(char *uri, char *hostname, char *resource);

int32_t download_file(int socket_descriptor, char *hostname, char *resource_required, int32_t transmission_rate, FILE* output_file);

int32_t extract_content(char *http_response, char* content,int32_t content_length);

int8_t verify_file_path(char *path, char *resourcec, char *fullpath);

int verify_connection(ConnectionManager *manager, int32_t listening_socket, fd_set *read_fds, fd_set *master, int *greatest_fds );

void create_default_response_files(char *path,
                                   FILE **bad_request_file,
                                   FILE **not_found_file,
                                   FILE **internal_error_file,
                                   FILE **unauthorized_file,
                                   FILE **wrong_version_file );

int setup_listening_connection(char* port, int32_t* listening_socket);

void clean_default_files();

#endif // HTTP_UTILS_H
