#include "http_utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

#define PROTOCOL_SIZE     9
#define OPERATION_SIZE    6
#define MAX_RESOURCE_SIZE 50

static const char* HeaderBadRequest    = "HTTP/1.0 400 Bad Request\r\n";
static const char* HeaderOk            = "HTTP/1.0 200 OK\r\n";
static const char* HeaderNotFound      = "HTTP/1.0 404 Not Found\r\n";
static const char* HeaderInternalError = "HTTP/1.0 500 Internal Server Error\r\n";
static const char* HeaderUnauthorized  = "HTTP/1.0 401 Unauthorized\r\n";
static const char* HeaderWrongVerison  = "HTTP/1.0 505 HTTP Version Not Supported\r\n";
static const char* ContentLenghtMask   = "Content-Length=%d\r\n";

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

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

  if (sscanf(http_response,"HTTP/%*s %d %s\r\n", &status, error_string) != 2)
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
        printf("Resource not found in server. Status: %s\n", error_string);
        return -1;
      }
      break;
  case MovedPermanently:
      {
        printf("Resource was permanently move from the server. Status: %s\n", error_string);
        return -1;
      }
      break;
  case MovedTemporarily:
      {
        printf("Resource was temporarily move from the server. Status: %s\n", error_string);
        return -1;
      }
      break;
  case ServerError:
      {
        printf("Server error. Status: %s\n", error_string);
        return -1;
      }
      break;
  default:
      {
        printf("Unknown error status. Status: %s\n", error_string);
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
  const char *protocol_suffix_end = "://";
  char *pointer = strstr(uri, protocol_suffix_end);
  if( pointer != NULL )
  {
    pointer += strlen( protocol_suffix_end );
    sscanf(pointer, "%[^/]%s", hostname, resource);
  }
  else
  {
    sscanf(uri, "%[^/]%s", hostname, resource);
  }
}

int32_t download_file(int socket_descriptor, char *hostname, char *resource_required, int32_t transmission_rate, FILE *output_file)
{
  int32_t resource_required_length = strlen(resource_required);
  int32_t hostname_length          = strlen(hostname);

  char *request_mask = "GET %s HTTP/1.0\r\n"
                       //"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
                       //"Accept-Encoding: gzip, deflate, sdch\r\n"
                       //"Accept-Language: en-US,en;q=0.8\r\n"
                       //"Cache-Control: max-age=0\r\n"
                       //"Connection: keep-alive\r\n"
                       "Host: %s\r\n"
                       //"Upgrade-Insecure-Requests: 1\r\n"
                       //"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.108 Safari/537.36\r\n"
                       "\r\n"
          ;

  char *request_msg = malloc(sizeof(char)*(strlen(request_mask) + resource_required_length + hostname_length + 1));
  sprintf(request_msg,
          request_mask,
          (resource_required_length != 0) ? resource_required : "/index.html",
          hostname );

  printf( "%s\n", request_msg );

  int32_t request_len = strlen(request_msg);
  int32_t total_bytes_sent = 0;
  int32_t bytes_sent = 0;
  do
  {
    int32_t attempt_size = request_len - total_bytes_sent;
    bytes_sent = send(socket_descriptor, &request_msg[total_bytes_sent], attempt_size, 0);
    if (bytes_sent == -1)
    {
        perror( "Error in send" );
        return -1;
    }
    total_bytes_sent += bytes_sent;
  }
  while (total_bytes_sent != request_len);
  free(request_msg);

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


int32_t receive_request(int32_t socket_descriptor, Connection *item, const int32_t transmission_rate)
{
  // Read data from client (( use http_utils.c ))%
  char buffer[8000];
  char *carriage = buffer;
  int32_t total_bytes_received = 0;
  int32_t bytes_received = 0;
  while(1)
  {
    bytes_received = recv(socket_descriptor, carriage, sizeof(buffer), 0);
    if (bytes_received < 0)
    {
      if( errno == EWOULDBLOCK || errno == EAGAIN )
      {
        break;
      }
      else
      {
        perror("recv");
        return -1;
      }
    }

    if (bytes_received == 0)
    {
      break;
    }

    total_bytes_received += bytes_received;
    carriage = buffer + total_bytes_received; /*&buffer[total_bytes_received]*/
  }
  buffer[total_bytes_received] = '\0'; /* *carriage = '\*/

  item->request = malloc(sizeof(char)*total_bytes_received);
  memset( item->request, '\0', total_bytes_received);
  strncpy(item->request, buffer, total_bytes_received + 1);
  item->response_size = total_bytes_received;
  item->active = 1;
  printf("Incomming request: \n%s\n", item->request);

  return 0;
}

int32_t send_response(int32_t socket_descriptor, fd_set *master_ptr, Connection *item, int32_t transmission_rate)
{
  int32_t total_bytes_sent = 0;
  int32_t bytes_sent = 0;
  int32_t header_size = strlen(item->header);
  char *header = item->header;
  do
  {
    int32_t attempt_size = header_size - total_bytes_sent;
    bytes_sent = send(socket_descriptor, &header[total_bytes_sent], attempt_size, 0);
    if (bytes_sent == -1)
    {
        perror( "Error in send" );
        return -1;
    }
    total_bytes_sent += bytes_sent;
  }
  while (total_bytes_sent != header_size);
  free(item->header); // check this ####

  total_bytes_sent = 0;
  bytes_sent = 0;
  char resource[8000];
  int32_t total_bytes_read = 0;
  int32_t bytes_read = 0;
  do
  {
    bytes_read = fread(resource, sizeof(char), transmission_rate, item->resource_file);
    if ((bytes_read != -1) &&
        (bytes_read != 0 ))
    {
      do
      {
        if (bytes_read < transmission_rate)
        {
          transmission_rate = bytes_read;
        }
        //int32_t attempt_size = transmission_rate - total_bytes_sent;
        bytes_sent = send(socket_descriptor, &resource[total_bytes_sent], transmission_rate, 0);
        if (bytes_sent == -1)
        {
          perror( "Error in send" );
          return -1;
        }
        total_bytes_sent += bytes_sent;
      }
      while (total_bytes_sent != bytes_read);
    }
  }while (bytes_read != 0 || total_bytes_read == item->response_size);
  //free(item->header); // check this ####

  /*char *request_mask = "%s%s";
  char *request_msg = malloc(sizeof(char)*(request_size + 1200));
  sprintf(request_msg,
          request_mask,
          item->header,
          resource_required);

  printf("\nSend header:\n%s\n", request_msg);

  int32_t request_len = strlen(request_msg);
  int32_t total_bytes_sent = 0;
  int32_t bytes_sent = 0;
  do
  {
    int32_t attempt_size = request_len - total_bytes_sent;
    bytes_sent = send(socket_descriptor, &request_msg[total_bytes_sent], attempt_size, 0);
    if (bytes_sent == -1)
    {
        perror( "Error in send" );
        return -1;
    }
    total_bytes_sent += bytes_sent;
  }
  while (total_bytes_sent != request_len);
  free(request_msg);*/

  printf("Socket = %d closed\n\n", socket_descriptor);
  close(socket_descriptor);
  FD_CLR(socket_descriptor, master_ptr);

  return 0;
}


void handle_request(Connection *item, char *path)
{
  char *index_str = "/index.html";
  char operation[OPERATION_SIZE];
  char resource[MAX_RESOURCE_SIZE];
  char protocol[PROTOCOL_SIZE];

  memset(operation, '\0', OPERATION_SIZE);
  memset(resource,  '\0', MAX_RESOURCE_SIZE);
  memset(protocol,  '\0', PROTOCOL_SIZE);

  char *return_string = "..";

  char *request = item->request;
  item->resource_file = NULL;
  item->response_size = 0;
  if (sscanf(request, "%s %s %s\r\n", operation, resource, protocol) != 3)
  {
    item->header = strdup(HeaderBadRequest);
    return;
  }

  printf("%s", operation);

  if (strncmp(protocol, "HTTP/1.0", PROTOCOL_SIZE) != 0 &&
     (strncmp(protocol, "HTTP/1.1", PROTOCOL_SIZE) != 0 ) )
  {
    item->header = strdup(HeaderWrongVerison);
    return;
  }

  if (strstr(resource, return_string) != NULL)
  {
    item->header = strdup(HeaderBadRequest);
    return;
  }

  int32_t resource_size = strlen(resource);
  if( strncmp(resource, "/", resource_size) == 0 ||
      strncmp(resource, ".", resource_size) == 0 )
  {
    strncpy(resource, index_str, strlen(index_str));
  }

  const int32_t path_size     = strlen(path);
  resource_size = strlen(resource);
  char  *file_name = malloc(sizeof(char) * (path_size+resource_size+1));
  sprintf( file_name, "%s%s", path, resource);
  FILE *teste = fopen(file_name, "rb");
  item->resource_file = teste;
  if (item->resource_file == NULL)
  {
    if (errno == EACCES)
    {
      item->header = strdup(HeaderUnauthorized);
      return;
    }
    else if ( errno == E2BIG)
    {
      item->header = strdup(HeaderBadRequest);
      return;
    }
    else
    {
      item->header = strdup(HeaderNotFound);
      return;
    }
  }

  struct stat buffer;
  if (stat(file_name, &buffer) == -1 )
  {
    item->header = strdup(HeaderInternalError);
    return;
  }

  int32_t contentLenghtMaskSize = strlen(ContentLenghtMask);
  const int32_t maxLenghtStrSize = 12;
  item->header     = malloc(sizeof(char)*(strlen(HeaderOk) + contentLenghtMaskSize + maxLenghtStrSize));
  char *headerMask = malloc(sizeof(char)*(strlen(HeaderOk) + contentLenghtMaskSize + 1));
  sprintf(headerMask, "%s%s", HeaderOk, ContentLenghtMask );
  sprintf(item->header, headerMask, buffer.st_size);
  item->response_size = buffer.st_size;

  return;
}


int verify_connection( int32_t listening_socket, fd_set *read_fds, fd_set *master, int *greatest_fds)
{
  struct sockaddr_storage client_address;
  char                    remote_ip[INET6_ADDRSTRLEN];

  if (FD_ISSET(listening_socket, read_fds))
  {
    socklen_t addrlen = sizeof(client_address);
    int32_t new_socket_description = accept(listening_socket,
                                    (struct sockaddr *)&client_address,
                                    &addrlen);
    if (new_socket_description == -1)
    {
      perror("Accept");
      return -1;
    }
    else
    {
      FD_SET(new_socket_description, master);
      if (fcntl(new_socket_description, F_SETFL, fcntl(new_socket_description, F_GETFL) | O_NONBLOCK) < 0)
      {
        perror("fcntl");
        return -1;
      }

      inet_ntop(client_address.ss_family,
                get_in_addr((struct sockaddr *)&client_address),
                remote_ip,
                sizeof(remote_ip));

      printf("Connection from %s -> socket_num = %d\n", remote_ip, new_socket_description);
      if (new_socket_description > *greatest_fds)
      {
        *greatest_fds = new_socket_description;
      }
    }
  }

  return 0;
}
