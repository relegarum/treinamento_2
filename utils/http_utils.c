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

#define MAX_ERROR_STR_SIZE 30
#define PROTOCOL_SIZE     9
#define OPERATION_SIZE    6
#define MAX_RESOURCE_SIZE 50
#define MAX_LONG_STR_SIZE 10 /* 4294967295 */
#define MAX_REQUEST_MASK_SIZE 23 /* GET %s HTTP/1.0\r\n\r\n */

const char *RequestMsgMask      = "GET %s HTTP/1.0\r\n\r\n";
const char *HeaderBadRequest    = "HTTP/1.0 400 Bad Request\r\n";
const char *HeaderOk            = "HTTP/1.0 200 OK\r\n";
const char *HeaderNotFound      = "HTTP/1.0 404 Not Found\r\n";
const char *HeaderInternalError = "HTTP/1.0 500 Internal Server Error\r\n";
const char *HeaderUnauthorized  = "HTTP/1.0 401 Unauthorized\r\n";
const char *HeaderWrongVersion  = "HTTP/1.0 505 HTTP Version Not Supported\r\n";
const char *ContentLenghtMask   = "Content-Length=%10d\r\n";
const char *ContentTypeStr      = "Content-Type=application/octet-stream\r\n";
const char *ServerStr           = "Server=Aker\r\n";
const char *IndexStr            = "/index.html";
const char *HTTP10Str           = "HTTP/1.0";
const char *HTTP11Str           = "HTTP/1.1";

const char *HtmlBadRequestFileName   = "BadRequest.html";
const char *HtmlNotFoundFileName     = "NotFound.html";
const char *HtmlInternalErrorName    = "InternalErrorName.html";
const char *HtmlUnauthorizedFileName = "Unauthorized.html";
const char *HtmlWrongVersionFileName = "WrongVersion.html";

#define HTML_HEADER(number, string) "HTTP/1.0 "#number" "#string"\r\n";
#define HTML_ERROR(number, string) "<HTML><TITLE>"#number" "#string"</TITLE><BODY><H2>"#number" "#string"</H2></BODY></HTML>"


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
  char error_string[MAX_ERROR_STR_SIZE];
  int32_t status = 0;

  if (sscanf(http_response,"HTTP/%*s %3d %29s\r\n", &status, error_string) != 2)
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
  uint32_t resource_required_length = strlen(resource_required);
  uint32_t request_msg_size         = MAX_REQUEST_MASK_SIZE + ((resource_required_length != 0)? resource_required_length : strlen(IndexStr));

  char *request_msg = malloc(sizeof(char)*request_msg_size);

  snprintf(request_msg, request_msg_size, RequestMsgMask, (resource_required_length != 0) ? resource_required : IndexStr);
  int32_t request_len = strlen(request_msg);
  int32_t bytes_sent = send(socket_descriptor, request_msg, request_len, 0);

  free(request_msg);
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
                       "Host: %s\r\n"
                       "User-Agent: AkerClient\r\n"
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
  do
  {
    int32_t bytes_sent = 0;
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
  char header_slice[header_slice_size];
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


int32_t receive_request(Connection *item, const int32_t transmission_rate)
{
  item->state = Receiving;
  // Read data from client (( use http_utils.c ))%
  char buffer[8000];
  char *carriage               = buffer;
  int32_t total_bytes_received = 0;
  int32_t socket_descriptor    = item->socket_descriptor;
  while(1)
  {
    int32_t bytes_received       = 0;
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
  buffer[total_bytes_received] = '\0'; /* *carriage = '\0*/

  item->request = malloc(sizeof(char)*(total_bytes_received + 1));
  memset( item->request, '\0', total_bytes_received + 1);
  strncpy(item->request, buffer, total_bytes_received + 1);
  item->response_size = total_bytes_received;
  item->state = Sending;
  //printf("Incomming request: \n%s\n", item->request);

  return 0;
}

int32_t send_response(Connection *item, int32_t transmission_rate)
{
  item->state = Sending;
  if(item->header_sent == 0)
  {
    if (send_header(item, transmission_rate) == -1)
    {
      return -1;
    }
  }

  if (item->resource_file == NULL)
  {
    item->state = Sent;
    return -1;
  }

  if (send_resource(item, transmission_rate) == -1)
  {
    item->state = Sent;
    return -1;
  }

  if (item->wroteData > item->response_size)
  {
    printf("Error in read data: Preventing loop\n");
    item->state = Sent;
  }
  else if (item->wroteData != item->response_size)
  {
    return 0;
  }

  item->state = Sent;
  return 0;
}


void handle_request(Connection *item, char *path)
{
  item->state = Handling;
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
  if (sscanf(request, "%5s %49s %8s\r\n", operation, resource, protocol) != 3) // OPERATION_SIZE, MAX_RESOURCE_SIZE, PROTOCOL_SIZE
  {
    item->header = strdup(HeaderBadRequest);
    item->resource_file = bad_request_file;
    goto exit_handle;
  }

  if (strncmp(protocol, HTTP10Str, PROTOCOL_SIZE) != 0 &&
     (strncmp(protocol, HTTP11Str, PROTOCOL_SIZE) != 0 ) )
  {
    item->header = strdup(HeaderWrongVersion);
    item->resource_file = wrong_version_file;
    goto exit_handle;
  }

  if (strstr(resource, return_string) != NULL)
  {
    item->header = strdup(HeaderBadRequest);
    item->resource_file = bad_request_file;
    goto exit_handle;
  }

  int32_t resource_size = strlen(resource);
  if( strncmp(resource, "/", resource_size) == 0 ||
      strncmp(resource, ".", resource_size) == 0 )
  {
    strncpy(resource, IndexStr, strlen(IndexStr));
  }

  resource_size = strlen(resource);
  const int32_t path_size      = strlen(path);
  const int32_t file_name_size = path_size + resource_size + 1;
  char  *file_name = malloc(sizeof(char) * (file_name_size));
  snprintf(file_name, file_name_size, "%s%s", path, resource);
  item->resource_file = fopen(file_name, "rb");;
  free(file_name);

  if (item->resource_file == NULL)
  {
    if (errno == EACCES)
    {
      item->header = strdup(HeaderUnauthorized);
      item->resource_file = unauthorized_file;
      item->error         = 1;
      goto exit_handle;
    }
    else if ( errno == E2BIG)
    {
      item->header = strdup(HeaderBadRequest);
      item->resource_file = bad_request_file;
      item->error         = 1;
      goto exit_handle;
    }
    else
    {
      item->header = strdup(HeaderNotFound);
      item->resource_file = not_found_file;
      item->error         = 1;
      goto exit_handle;
    }
  }

exit_handle:
  get_resource_data(item);
  item->state = Sending;
  return;
}


int32_t get_resource_data(Connection *item)
{
  int32_t fd = fileno( item->resource_file);
  struct stat buffer;
  if (fstat(fd, &buffer) == -1)
  {
    item->header = strdup(HeaderInternalError);
    item->resource_file = NULL;
    return -1;
  }

  int32_t content_length_mask_size = strlen(ContentLenghtMask);
  int32_t header_ok_size          = strlen(HeaderOk);
  int32_t server_str_size         = strlen(ServerStr);
  int32_t content_type_str_size   = strlen(ContentTypeStr);
  int32_t header_mask_size        = header_ok_size +
                                  server_str_size +
                                  content_length_mask_size +
                                  content_type_str_size +
                                  3; /* \r\n\0 */

  const int32_t maxLenghtStrSize = 50;
  const int32_t header_size      = header_mask_size + maxLenghtStrSize + 3;
  item->header     = malloc(sizeof(char) * (header_size));
  char *headerMask = malloc(sizeof(char) * (header_mask_size)); // \r\n

  uint64_t size = buffer.st_size;
  snprintf(headerMask, header_mask_size, "%s%s%s%s\r\n", HeaderOk, ContentLenghtMask, ServerStr, ContentTypeStr);
  snprintf(item->header, header_size, headerMask, size);
  item->response_size = size;
  free(headerMask);

  return 0;
}

int verify_connection(ConnectionManager *manager, int32_t listening_socket, fd_set *read_fds, fd_set *master, int *greatest_fds)
{
  struct sockaddr_storage client_address;

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
      Connection* item = create_connection_item(new_socket_description);
      add_connection_in_list(manager, item);
      FD_SET(new_socket_description, master);
      if (fcntl(new_socket_description, F_SETFL, fcntl(new_socket_description, F_GETFL) | O_NONBLOCK) < 0)
      {
        perror("fcntl");
        return -1;
      }

      char remote_ip[INET6_ADDRSTRLEN];
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

int32_t send_header(Connection *item, int32_t transmission_rate)
{
  int32_t socket_descriptor = item->socket_descriptor;

  int32_t total_bytes_sent  = 0;
  int32_t header_size       = strlen(item->header);
  char *header              = item->header;
  do
  {
    int32_t bytes_sent        = 0;
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
  item->header_sent = 1;

  return 0;
}

int32_t send_resource(Connection *item, int32_t transmission_rate)
{
  int32_t socket_descriptor = item->socket_descriptor;
  char resource[transmission_rate + 1];
  int32_t bytes_read = 0;
  int32_t bytes_sent = 0;
  fseek(item->resource_file, item->wroteData, SEEK_SET);

  bytes_read = fread(resource, sizeof(char), transmission_rate, item->resource_file);

  if ((bytes_read != -1) &&
      (bytes_read != 0 ))
  {
    if (bytes_read < transmission_rate)
    {
      transmission_rate = bytes_read;
    }
    int32_t total_byte_sent = 0;

    int32_t bytes_to_sent = 0;
    while (total_byte_sent != transmission_rate)
    {
      bytes_to_sent = transmission_rate - total_byte_sent;
      bytes_sent = send(socket_descriptor, resource, bytes_to_sent, 0);
      if (bytes_sent == -1)
      {
        if (errno == EAGAIN ||
            errno == EWOULDBLOCK )
        {
          break;
        }
        perror( "Error in send" );
        return -1;
      }
      item->wroteData += bytes_sent;
      total_byte_sent += bytes_sent;

      if (bytes_sent != transmission_rate)
      {
        printf( "flag to see if sent bytes isn't transmission rate" );
      }
    }
  }
  return 0;
}

void create_default_response_files(char *path,
                                   FILE **bad_request_file,
                                   FILE **not_found_file,
                                   FILE **internal_error_file,
                                   FILE **unauthorized_file,
                                   FILE **version_wrond_file)
{

  *not_found_file     = NULL;
  *internal_error_file= NULL;
  *unauthorized_file  = NULL;
  *version_wrond_file = NULL;

  int32_t path_size = strlen(path);
  char *path_bad_request_file_name    = malloc(sizeof(char)*(strlen(HtmlBadRequestFileName)   + path_size + 2));
  char *path_not_found_file_name      = malloc(sizeof(char)*(strlen(HtmlNotFoundFileName)     + path_size + 2));
  char *path_internal_error_file_name = malloc(sizeof(char)*(strlen(HtmlInternalErrorName)    + path_size + 2));
  char *path_unauthorized_file_name   = malloc(sizeof(char)*(strlen(HtmlUnauthorizedFileName) + path_size + 2));
  char *path_wrong_file_name          = malloc(sizeof(char)*(strlen(HtmlWrongVersionFileName) + path_size + 2));
  {
    snprintf(path_bad_request_file_name, path_size + strlen(HtmlBadRequestFileName) + 2, "%s/%s", path, HtmlBadRequestFileName);
    *bad_request_file = fopen(path_bad_request_file_name, "w+b");
    if (*bad_request_file != NULL)
    {
      char *html = HTML_ERROR(400, Bad Request);
      fwrite(html, sizeof(char), strlen(html), *bad_request_file);
      fflush(*not_found_file);
    }

    snprintf(path_not_found_file_name, path_size + strlen(HtmlNotFoundFileName) + 2, "%s/%s", path, HtmlNotFoundFileName);
    *not_found_file  = fopen(path_not_found_file_name, "w+b");
    if (*not_found_file != NULL)
    {
      char *html = HTML_ERROR(404, Not Found);
      fwrite(html, sizeof(char), strlen(html), *not_found_file);
      fflush(*not_found_file);
    }

    snprintf(path_internal_error_file_name, path_size + strlen(HtmlInternalErrorName) + 2, "%s/%s", path, HtmlInternalErrorName);
    *internal_error_file = fopen(path_internal_error_file_name, "w+b");
    if (*internal_error_file != NULL)
    {
      char *html = HTML_ERROR(500, Internal Server Error);
      fwrite(html, sizeof(char), strlen(html), *internal_error_file);
      fflush(*internal_error_file);
    }

    snprintf(path_unauthorized_file_name, path_size + strlen(HtmlUnauthorizedFileName) + 2, "%s/%s", path, HtmlUnauthorizedFileName);
    *unauthorized_file = fopen(path_unauthorized_file_name, "w+b");
    if (*unauthorized_file != NULL)
    {
      char *html = HTML_ERROR(401, Unauthorized);
      fwrite(html, sizeof(char), strlen(html), *unauthorized_file);
      fflush(*unauthorized_file);
    }

    snprintf(path_wrong_file_name, path_size + strlen(HtmlWrongVersionFileName) + 2, "%s/%s", path, HtmlWrongVersionFileName);
    *version_wrond_file  = fopen(path_wrong_file_name, "w+b");
    if (*version_wrond_file != NULL)
    {
      char *html = HTML_ERROR(505, HTTP Version Not Supported);
      fwrite(html, sizeof(char), strlen(html), *version_wrond_file);
      fflush(*version_wrond_file);
    }
  }
  free(path_bad_request_file_name);
  free(path_not_found_file_name);
  free(path_internal_error_file_name);
  free(path_unauthorized_file_name);
  free(path_wrong_file_name);
}
