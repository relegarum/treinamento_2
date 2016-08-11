#include "connection_item.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
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

#include "http_utils.h"
#include "file_utils.h"

#define MAX_REQUEST_SIZE      8000
#define MAX_ERROR_STR_SIZE    30
#define PROTOCOL_SIZE         9
#define OPERATION_SIZE        6
#define MAX_RESOURCE_SIZE     PATH_MAX
#define MAX_LONG_STR_SIZE     10 /* 4294967295 */
#define MAX_REQUEST_MASK_SIZE 23 /* GET %s HTTP/1.0\r\n\r\n */
#define MAX_MIME_SIZE         128
#define UNKNOWN_MIME_SIZE     24
#define MAX_TRIES             3

const char * const HeaderBadRequest     = "HTTP/1.0 400 Bad Request\r\n\r\n";
const char * const HeaderOk             = "HTTP/1.0 200 OK\r\n";
const char * const HeaderCreated        = "HTTP/1.0 201 Created\r\n";
const char * const HeaderConflict       = "HTTP/1.1 409 Conflict\r\n\r\n";
const char * const HeaderNotFound       = "HTTP/1.0 404 Not Found\r\n\r\n";
const char * const HeaderInternalError  = "HTTP/1.0 500 Internal Server Error\r\n\r\n";
const char * const HeaderUnauthorized   = "HTTP/1.0 401 Unauthorized\r\n\r\n";
const char * const HeaderWrongVersion   = "HTTP/1.0 505 HTTP Version Not Supported\r\n\r\n";
const char * const HeaderNotImplemented = "HTTP/1.0 501 HTTP Not Implemented\r\n\r\n";
const char * const EndOfHeader    = "\r\n\r\n";
const char * const RequestMsgMask = "GET %s HTTP/1.0\r\n\r\n";

const char *ContentLenghtMask    = "Content-Length: %lld\r\n";
const char *ContentTypeStr       = "Content-Type: %s\r\n";
const char *UnknownTypeStr       = "application/octet-stream";
const char *ServerStr            = "Server: Aker\r\n";
const char *ContentLengthNeedle  ="Content-Length:";


void init_connection_item(Connection *item, int socket_descriptor, uint32_t id)
{
  item->socket_descriptor    = socket_descriptor;
  item->state                = Free;
  item->header_sent          = 0;
  item->error                = 0;
  item->wrote_data           = 0;
  item->read_data            = 0;
  item->resource_size        = 0;
  item->partial_read         = 0;
  item->partial_wrote        = 0;
  item->read_file_data       = 0;
  item->data_to_write_size   = 0;
  item->tries                = 0;
  item->id                   = id;
  item->method               = Unknown;
  item->buffer[0]            = '\0';
  item->request              = NULL;
  item->header               = NULL;
  item->end_of_header        = NULL;
  item->next_ptr             = NULL;
  item->previous_ptr         = NULL;
  item->datagram_socket      = -1;

  item->last_connection_time.tv_sec = 0;
  item->last_connection_time.tv_usec = 0;
}

void free_connection_item(Connection *item)
{
  if (item->method == Put)
  {
    treat_file_after_put(&(item->file_components), item->error);
  }

  if (item->socket_descriptor != -1)
  {
    close(item->socket_descriptor);
    item->socket_descriptor = -1;
  }

  if (item->error != 1)
  {
    destroy_file_components(&item->file_components);
  }

  if (item->header != NULL)
  {
    free(item->header);
    item->header = NULL;
  }

  if (item->request != NULL)
  {
    free(item->request);
    item->request = NULL;
  }

  if (item->datagram_socket != -1)
  {
    close(item->datagram_socket);
    item->datagram_socket = -1;
  }


  item->state                = Closed;
  item->id                   = 0;
  item->header_sent          = 0;
  item->read_data            = 0;
  item->wrote_data           = 0;
  item->partial_read         = 0;
  item->partial_wrote        = 0;
  item->read_file_data       = 0;
  item->resource_size        = 0;
  item->data_to_write_size   = 0;
  item->tries                = 0;
  item->method               = Unknown;
  item->buffer[0]            = '\0';
  item->next_ptr             = NULL;
  item->previous_ptr         = NULL;
  item->end_of_header        = NULL;

  item->last_connection_time.tv_sec = 0;
  item->last_connection_time.tv_usec = 0;

  free(item);
}

Connection *create_connection_item(int socket_descriptor, uint32_t id)
{
  Connection *item = malloc(sizeof(Connection));
  init_connection_item(item, socket_descriptor, id);
  return item;
}

int32_t receive_request(Connection *item, const uint32_t transmission_rate)
{
  uint32_t rate = (BUFSIZ < transmission_rate) ? BUFSIZ: transmission_rate;
  item->request = realloc(item->request,
                          sizeof(char)*(item->read_data +
                                        transmission_rate + 1));

  char *carriage = item->request + item->read_data;

  int32_t  socket_descriptor    = item->socket_descriptor;
  uint32_t total_bytes_received = 0;
  int8_t   end_of_header_flag   = 0;
  do
  {
    uint32_t bytes_to_read  = rate - total_bytes_received;
    int32_t bytes_received = recv(socket_descriptor, carriage, bytes_to_read, 0);
    if (bytes_received < 0)
    {
      if( errno == EWOULDBLOCK || errno == EAGAIN )
      {
        end_of_header_flag = 1;
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
      end_of_header_flag = 1;
      break;
    }

    total_bytes_received += bytes_received;
    item->read_data      += bytes_received;
    item->partial_read   += bytes_received;
    carriage             += bytes_received; /*&buffer[total_bytes_received]*/
  } while((rate != total_bytes_received));

  if (item->read_data > END_OF_HEADER_SIZE) /*\r\n\r\n*/
  {
    char *needle = strstr(item->request, EndOfHeader);
    if (needle != NULL)
    {
      needle += END_OF_HEADER_SIZE;
      item->data_to_write_size = (item->read_data - (needle - item->request));
      memcpy(item->buffer, needle, item->data_to_write_size);
      end_of_header_flag = 1;
    }
  }

  if (end_of_header_flag)
  {
    item->request[item->read_data + 1] = '\0'; /* *carriage = '\0*/
    item->state = Handling;
  }

  return 0;
}

int32_t send_response(Connection *item, uint32_t transmission_rate)
{
  if (send_resource(item, transmission_rate) == -1)
  {
    item->state = Sent;
    return -1;
  }

  if (item->wrote_data != item->resource_size)
  {
    item->state = ReadingFromFile;
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
  char file_final_path[PATH_MAX];
  char mime[MAX_MIME_SIZE];

  memset(operation, '\0', OPERATION_SIZE);
  memset(resource,  '\0', MAX_RESOURCE_SIZE);
  memset(protocol,  '\0', PROTOCOL_SIZE);
  memset(file_final_path, '\0', PROTOCOL_SIZE);

  char *request = item->request;
  /*item->resource_file = NULL;*/
  item->resource_size = 0;
  /* OPERATION_SIZE, MAX_RESOURCE_SIZE, PROTOCOL_SIZE */
  if (sscanf(request, "%5s %4095s %8s\r\n", operation, resource, protocol) != 3)
  {
    item->header = strdup(HeaderBadRequest);
    item->file_components.file_ptr = bad_request_file;
    item->error = 1;
    goto exit_handle;
  }


  if (verify_protocol(protocol) != 0)
  {
    item->header = strdup(HeaderWrongVersion);
    item->file_components.file_ptr = wrong_version_file;
    item->error = 1;
    goto exit_handle;
  }

  if (resource[0] != '/')
  {
    item->header = strdup(HeaderBadRequest);
    item->file_components.file_ptr = bad_request_file;
    item->error = 1;
    goto exit_handle;
  }

  if (verify_file_path(path, resource, file_final_path) != 0)
  {
    item->header = strdup(HeaderNotFound);
    item->file_components.file_ptr = not_found_file;
    item->error = 1;
    goto exit_handle;
  }

  if (strncmp(operation, "GET", OPERATION_SIZE) == 0)
  {
    if (handle_get_method(item, file_final_path) != 0 )
    {
      goto exit_handle;
    }
  }
  else if (strncmp(operation, "PUT", OPERATION_SIZE) == 0)
  {
    if (handle_put_method(item, file_final_path) != 0)
    {
      goto exit_handle;
    }
    return;
  }
  else
  {
    item->header = strdup(HeaderNotImplemented);
    item->file_components.file_ptr = not_implemented_file;
    item->error = 1;
    goto exit_handle;
  }


exit_handle:
  if (item->file_components.file_ptr != NULL)
  {
    get_resource_data(item, file_final_path, mime);
  }

  if (item->error == 0)
  {
    setup_header(item, mime);
  }
  item->state = SendingHeader;
  return;
}

int32_t get_resource_data(Connection *item, char *file_name, char *mime)
{
  if (item->file_components.file_ptr == NULL)
  {
    return -1;
  }
  int32_t fd = fileno(item->file_components.file_ptr);
  struct stat buffer;
  if (fstat(fd, &buffer) == -1)
  {
    item->header = strdup(HeaderInternalError);
    item->file_components.file_ptr = NULL;
    item->error = 1;
    return -1;
  }

  if (!S_ISREG(buffer.st_mode))
  {
    item->header = strdup(HeaderBadRequest);
    /*fclose(item->file_components.file_ptr);*/
    item->file_components.file_ptr = NULL;
    item->error = 1;
    return -1;
  }

  uint64_t size = buffer.st_size;
  item->resource_size = size;
  if (file_name != NULL)
  {
    uint32_t file_name_size = strlen(file_name);
    if (file_name_size != 0)
    {
      if (get_file_mime(file_name_size, file_name, mime) != 0)
      {
        strncpy(mime, UnknownTypeStr, UNKNOWN_MIME_SIZE);
      }
    }
  }
  return 0;
}

int32_t send_header(Connection *item, const uint32_t transmission_rate)
{
  int32_t socket_descriptor = item->socket_descriptor;

  uint32_t total_bytes_sent  = 0;
  uint32_t header_size       = strlen(item->header);
  uint32_t rate = (BUFSIZ < transmission_rate)? BUFSIZ: transmission_rate;

  if ((header_size - item->wrote_data) < rate)
  {
    rate = (header_size - item->wrote_data);
  }
  char *carriage = item->header + item->wrote_data;
  while (total_bytes_sent < rate &&
         total_bytes_sent < header_size)
  {
    int32_t bytes_to_sent = rate - total_bytes_sent;
    int32_t bytes_sent = send(socket_descriptor,
                              carriage,
                              bytes_to_sent,
                              MSG_NOSIGNAL);
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

    carriage         += bytes_sent;
    total_bytes_sent += bytes_sent;
    item->wrote_data += bytes_sent;
  }

  if (item->wrote_data >= header_size)
  {
    item->state = (item->method == Put) ? Sent : ReadingFromFile;
    item->wrote_data  = 0;
  }

  return 0;
}

int32_t send_resource(Connection *item, const int32_t transmission_rate)
{
  int ret = 0;
  uint32_t rate = (BUFSIZ < transmission_rate)? BUFSIZ: transmission_rate;
  int32_t bytes_read = item->read_file_data;

  int32_t bytes_sent = 0;
  int32_t socket_descriptor = item->socket_descriptor;
  if (bytes_read > 0)
  {
    if ((uint32_t)bytes_read < rate)
    {
      rate = bytes_read;
    }
    uint32_t total_byte_sent = 0;
    char *carriage = item->buffer;

    while (total_byte_sent != rate)
    {
      int32_t bytes_to_sent = rate - total_byte_sent;
      bytes_sent = send(socket_descriptor,
                        carriage,
                        bytes_to_sent,
                        MSG_NOSIGNAL);
      if (bytes_sent == -1)
      {
        if (errno == EAGAIN ||
            errno == EWOULDBLOCK )
        {
          break;
        }
        perror( "Error in send" );
        ret = -1;
        goto exit_send_resource;
      }
      item->partial_wrote += bytes_sent;
      item->wrote_data    += bytes_sent;
      total_byte_sent     += bytes_sent;

      carriage += bytes_sent;
    }
  }

exit_send_resource:
  return ret;
}

void setup_header(Connection *item, char *mime)
{
  int32_t end_size                 = strlen("\r\n\0");
  int32_t content_length_mask_size = strlen(ContentLenghtMask);
  int32_t header_ok_size           = strlen(HeaderOk);
  int32_t server_str_size          = strlen(ServerStr);
  int32_t content_type_str_size    = strlen(ContentTypeStr) + MAX_MIME_SIZE;
  int32_t header_mask_size         = header_ok_size +
                                     server_str_size +
                                     content_length_mask_size +
                                     content_type_str_size +
                                     end_size;

  const int32_t max_Length_str_size = 50;
  const int32_t header_size      = header_mask_size +
                                   max_Length_str_size +
                                   end_size;

  item->header     = malloc(sizeof(char) * (header_size));
  char *headerMask = malloc(sizeof(char) * (header_mask_size)); // \r\n


  snprintf(headerMask,
           header_mask_size,
           "%s%s%s%s\r\n",
           HeaderOk,
           ContentLenghtMask,
           ServerStr,
           ContentTypeStr);

  snprintf(item->header,
           header_size,
           headerMask,
           item->resource_size,
           mime);

  free(headerMask);
}

void queue_request_to_read(Connection *item,
                           request_manager *manager,
                           const uint32_t transmission_rate)
{
  if (!is_valid_file(&(item->file_components)))
  {
    item->state = Sent;
    return;
  }

  int socket_pair[2];
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, socket_pair))
  {
    perror("socketPair");
  }

  item->datagram_socket = socket_pair[0];
  int io_thread_socket  = socket_pair[1];

  //set_socket_as_nonblocking(item->datagram_socket)
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 1000; // 100ms
  set_socket_timeout(item->datagram_socket, &timeout);

  uint32_t rate = (BUFSIZ < transmission_rate)? BUFSIZ - 1: transmission_rate;
  request_list_node *node = create_request_to_read(item->file_components.file_ptr,
                                                   io_thread_socket,
                                                   rate,
                                                   item->wrote_data);
  add_request_in_list(manager, node);
  item->state = WaitingFromIORead;
}


void receive_from_thread_read(Connection *item, const uint32_t transmission_rate)
{
  uint32_t rate = (BUFSIZ - 1 < transmission_rate)? BUFSIZ - 1 :
                                                    transmission_rate;
  int32_t read_data = read(item->datagram_socket, item->buffer, rate);
  if (read_data < 0)
  {
    if (errno == EAGAIN ||
        errno == EWOULDBLOCK)
    {
      item->tries++;
      return;
    }

    perror("read error");
    if (errno == EBADF)
    {
      item->state = ReadingFromFile;
    }
    else
    {
      item->state = Sent;
    }
    return;
  }

  if (read_data == 0)
  {
    item->state = Sent;
  }

  /*printf("Tries until read: %d\n", item->tries);*/
  close(item->datagram_socket);
  item->read_file_data = read_data;
  item->state = SendingResource;
}

int32_t handle_get_method(Connection *item, char *file_name)
{
  item->method = Get;

  init_file_components(&(item->file_components), file_name, ReadFile);
  get_file_state(item);

  item->state = SendingHeader;
  return item->error;
}


int32_t handle_put_method(Connection *item, char *file_name)
{
  item->method = Put;
  int32_t ret = 0;

  ret = init_file_components(&(item->file_components), file_name, WriteFile);
  if (ret == ExistentFile )
  {
    item->header = strdup(HeaderConflict);
    item->error  = 1;
    goto exit_handle;
  }
  ret = get_file_state(item);
  ret = extract_content_length_from_header(item);

 exit_handle:
  if (item->data_to_write_size > 0)
  {
    item->state = WritingIntoFile;
  }
  else
  {
    item->state = ReceivingFromPut;
  }

  return ret;
}

int32_t get_file_state(Connection *item)
{
  if (!is_valid_file(&(item->file_components)))
  {
    if (errno == EACCES)
    {
      item->header = strdup(HeaderUnauthorized);
      item->file_components.file_ptr = unauthorized_file;
      item->error         = 1;
      perror("access error");
      return -1;

    }
    else if (errno == E2BIG)
    {
      item->header = strdup(HeaderBadRequest);
      item->file_components.file_ptr = bad_request_file;
      item->error         = 1;
      perror("too big");
      return -1;
    }
    else
    {
      item->header = strdup(HeaderNotFound);
      item->file_components.file_ptr = not_found_file;
      item->error         = 1;
      perror("other'");
      return -1;
    }
  }

  return 0;
}

int32_t extract_content_length_from_header(Connection *item)
{
  char *carriage = strstr(item->request, ContentLengthNeedle);
  if (carriage == NULL)
  {
    item->header = strdup(HeaderBadRequest);
    item->file_components.file_ptr = bad_request_file;
    item->error         = 1;
    return -1;
  }
  sscanf(carriage, ContentLenghtMask, &(item->resource_size));

  return 0;
}

void verify_connection_state(Connection *item)
{
  if (item->state < FirsState ||
      item->state > LastState)
  {
    printf("Connection in state Unknown:\n"
           "State: %d\n"
           "Request: %s\n", item->state, item->request);
    item->state = Closed;
  }
}


void write_data_into_file(Connection *item,
                          FILE *resource_file)
{
  fseek(resource_file, item->wrote_data, SEEK_SET);
  int32_t wrote_bytes = fwrite(item->buffer,
                               sizeof(char),
                               item->data_to_write_size,
                               resource_file);
  if (wrote_bytes <= 0)
  {
    perror("write_data_into_file");
    item->state = WritingIntoFile;
    return;
  }

  fflush(resource_file);

  item->wrote_data += wrote_bytes;
  if (item->wrote_data >= item->resource_size)
  {
    item->state = SendingHeader;
    item->header = strdup(HeaderOk);
    item->wrote_data = 0; /* maybe create a new variable to do that */
    return;
  }
  item->state = ReceivingFromPut;
}

int32_t receive_data_from_put(Connection *item, const uint32_t transmission_rate)
{
  uint32_t rate = (BUFSIZ < transmission_rate)? BUFSIZ: transmission_rate;

  int32_t  socket_descriptor    = item->socket_descriptor;
  uint32_t total_bytes_received = 0;
  int8_t   end_of_resource   = 0;
  do
  {
    uint32_t bytes_to_read  = rate - total_bytes_received;
    int32_t bytes_received = recv(socket_descriptor, item->buffer, bytes_to_read, 0);
    if (bytes_received < 0)
    {
      if( errno == EWOULDBLOCK || errno == EAGAIN )
      {
        end_of_resource = 1;
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
    item->read_data      += bytes_received;
    item->partial_read   += bytes_received;
  } while((rate != total_bytes_received));

  /*if (item->read_data >= item->header_size )
  {
    return fail;
  }*/

  if (end_of_resource &&
      item->read_data >= item->resource_size)
  {
    item->buffer[total_bytes_received + 1] = '\0'; /* *carriage = '\0*/
  }

  item->data_to_write_size = total_bytes_received;
  item->state = WritingIntoFile;
  return 0;
}


void queue_request_to_write(Connection *item,
                            request_manager *manager)

{
  int socket_pair[2];
  if (socketpair(AF_UNIX, SOCK_DGRAM, 0, socket_pair))
  {
    perror("socketPair");
  }

  item->datagram_socket = socket_pair[0];
  int io_thread_socket  = socket_pair[1];

  set_socket_as_nonblocking(item->datagram_socket);

  request_list_node *node = create_request_to_write(item->file_components.file_ptr,
                                                    item->buffer,
                                                    io_thread_socket,
                                                    item->data_to_write_size,
                                                    item->wrote_data);
  add_request_in_list(manager, node);
  item->state = WaitingFromIOWrite;
}

void receive_from_thread_write(Connection *item)
{
  uint32_t data_wrote = 0;
  /*uint32_t rate = (BUFSIZ - 1 < transmission_rate)? BUFSIZ - 1: transmission_rate;*/
  int32_t read_data = read(item->datagram_socket, &data_wrote, sizeof(data_wrote));
  if (read_data < 0)
  {
    if (errno == EAGAIN ||
        errno == EWOULDBLOCK)
    {
      return;
    }

    perror("read error");
    if (errno == EBADF)
    {
      item->state = WritingIntoFile;
    }
    else
    {
      item->state = Sent;
    }
    return;
  }

  if (data_wrote > 0)
  {
    item->state = ReceivingFromPut;
    item->wrote_data += data_wrote;
  }
  else
  {
    if ((item->tries)++ < MAX_TRIES )
    {
      item->state = WaitingFromIORead;
      return;
    }
    else
    {
      item->state  = SendingHeader;
      item->header = strdup(HeaderInternalError);
      item->error  = 1;
    }
  }

  if (item->wrote_data >= item->resource_size)
  {
    item->state = SendingHeader;
    if (item->file_components.is_new_file)
    {
      item->header = strdup(HeaderCreated);
    }
    else
    {
      item->header = strdup(HeaderOk);
    }
    item->wrote_data = 0;
    return;
  }

  close(item->datagram_socket);
}
