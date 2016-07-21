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

uint32_t handle_response_status(char *http_response)
{
  char error_string[ 30 ];
  int32_t status = 0;

  if( sscanf( http_response,"HTTP/1.0 %d %s", &status, error_string) != 2 )
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

/**/
int32_t get_header(int socket_descriptor, char *resource_required, int32_t *header_length, int32_t *content_length)
{
  int resource_required_length = strlen(resource_required);

  char request_msg[ 80 ];
  sprintf(request_msg, "GET %s HTTP/1.0\r\n\r\n", (resource_required_length != 0) ? resource_required : "/index.html");
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
  const char *protocol_suffix = "http://";
  char *pointer = strstr(uri, protocol_suffix);
  if( pointer != NULL )
  {
    pointer += strlen( protocol_suffix );
    sscanf(pointer, "%[^/]%s", hostname, resource);
  }
  else
  {
    sscanf(uri, "%[^/]%s", hostname, resource);
  }
}

int32_t download_file(int socket_descriptor, char *resource_required, int32_t transmission_rate, FILE *output_file)
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

  int32_t header_length  = 0;
  int32_t content_length = 0;
  if( handle_header( socket_descriptor, &header_length, &content_length ) == -1 )
  {
    return -1;
  }


  char *header = malloc(sizeof(char)*header_length);
  char *carriage = header;
  int32_t bytes_received = recv(socket_descriptor, carriage, header_length, 0);
  if (bytes_received < header_length)
  {
    carriage = &header[ bytes_received ];
    bytes_received = recv( socket_descriptor, carriage, ( header_length - bytes_received), 0 );
  }
  free( header );

  bytes_received = 0;
  char *chunk = malloc(sizeof(char)*transmission_rate);
  while ((bytes_received = recv(socket_descriptor, chunk, transmission_rate, 0)) != 0)
  {
    int write_bytes = transmission_rate;
    if (bytes_received < transmission_rate)
    {
      chunk[ bytes_received ] = '\0';
      write_bytes = bytes_received;
    }

    fwrite( chunk, sizeof(char), write_bytes, output_file );
  }

  free(chunk);

  return 0;
}

int32_t extract_content(char *http_response, char *content, int32_t content_length)
{
  char *pointer    = strstr( http_response, "\r\n\r\n");
  char *contentPtr = pointer + 4; /*strlen( \r\n\r\n )*/
  strncpy( content, contentPtr, content_length );

  return 0;
}

int32_t handle_header(int socket_descriptor, int32_t *header_length, int32_t *content_size )
{
  int32_t bytes_received       = 0;
  const int32_t header_slice_size = 2048;
  char header_slice[ header_slice_size ];
  bytes_received = recv(socket_descriptor, header_slice, header_slice_size, MSG_PEEK );
  if( bytes_received == 0 )
  {
    printf( "Nothing was received as a Header!" );
    return -1;
  }

  if( handle_response_status( header_slice ) != 0 )
  {
    return -1;
  }

  char *pointer    = strstr( header_slice, "\r\n\r\n");
  char *contentPtr = pointer + 4; /*strlen( \r\n\r\n )*/
  *header_length    = (contentPtr - header_slice);
  *content_size     = get_response_size( header_slice );

  return 0;
}
