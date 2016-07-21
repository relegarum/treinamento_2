#include "http_utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

uint32_t get_response_size(char *first_chunk)
{
  char *slice = strstr(first_chunk, "Content-Length:");
  uint32_t size = 0;
  if (slice != NULL)
  {
    sscanf(slice, "Content-Length: %d\r\n", &size);
  }

  return size;
}


uint32_t handle_response_status(char *content)
{
  char error_string[ 30 ];
  int32_t status = 0;

  if( sscanf( content,"HTTP/1.0 %d %s", &status, error_string) != 2 )
  {
    printf("Coudn't parse HTTP Status\n");
    return -1;
  }

  switch (status)
  {
  case Ok:
      {
        return 0;
      }
      break;
  case Not_Found:
      {
        printf("Resource not found in server. Status: %s", error_string);
        return -1;
      }
      break;
  case MovedPermanently:
      {
        printf("Resource was permanently move from the server. Status: %s", error_string);
        return -1;
      }
      break;
  case MovedTemporarily:
      {
        printf("Resource was temporarily move from the server. Status: %s", error_string);
        return -1;
      }
      break;
  case ServerError:
      {
        printf("Server error. Status: %s", error_string);
        return -1;
      }
      break;
  default:
      {
        printf("Unknown error status. Status: %s", error_string);
        return -1;
      }
      break;
  }
}



int32_t get_header(int socket_descriptor, char *resource_required, int32_t *header_length, int32_t *content_length)
{
  int resource_required_length = strlen(resource_required);

  char request_msg[ 80 ];
  sprintf(request_msg, "HEAD %s HTTP/1.0\r\n\r\n", (resource_required_length != 0) ? resource_required : "/index.html");
  int32_t request_len = strlen(request_msg);
  int32_t bytes_sent = send(socket_descriptor, request_msg, request_len, 0);
  if (bytes_sent != request_len)
  {
    printf("Coudn't send entire request\n");
    return 1;
  }

  char header_buffer[300];
  int bytes_received = recv(socket_descriptor, header_buffer, sizeof(header_buffer), 0);
  if (bytes_received <= 0)
  {
    printf("Coudn't receive response\n");
    return 1;
  }
  header_buffer[ bytes_received ] = '\0';

  *header_length  = bytes_received;
  *content_length = get_response_size(header_buffer);

  printf( "Header Received:\n %s \n", header_buffer );

  return 0;
}

void get_resource(char *uri, char *hostname, char *resource)
{
  sscanf(uri, "%[^/]%s", hostname, resource);
}

int32_t download_file(int socket_descriptor, char *resource_required, int32_t content_length, char *content_buffer)
{
  int resource_required_length = strlen(resource_required);

  char request_msg[ 80 ];
  sprintf(request_msg, "GET %s HTTP/1.0\r\n\r\n", (resource_required_length != 0) ? resource_required : "/index.html");
  int32_t request_len = strlen(request_msg);
  int32_t bytes_sent = send(socket_descriptor, request_msg, request_len, 0);
  if (bytes_sent != request_len)
  {
    printf("Coudn't send entire request\n");
    return -1;
  }

  char *carriage = content_buffer;
  int32_t  chunk_size = content_length;
  uint32_t total_bytes_received = 0;
  uint32_t bytes_received = 0;
  while ((bytes_received = recv(socket_descriptor, carriage, chunk_size, 0)) != 0)
  {
    total_bytes_received += bytes_received;
    carriage = &( content_buffer[ total_bytes_received ] );
  }
  content_buffer[ total_bytes_received ] = '\0';
  printf( "Content Received:\n %s \n", content_buffer );

  if( handle_response_status( content_buffer ) != 0 )
  {
    return -1;
  }

  return 0;
}
