/*$Id: $*/

/*Standard includes*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/*Network includes */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

int handle_arguments( int32_t argc,
                      char **argv,
                      char *uri,
                      char *output_file,
                      int8_t* overwrite )
{
  const int32_t number_of_arguments = 3;
  if ( argc < number_of_arguments )
  {
    printf( "Wrong number of arguments\n Format: URI outputFile\n" );
    exit( 1 );
  }

  strncpy( uri,         argv[ 1 ], strlen( argv[ 1 ] ) + 1 );
  strncpy( output_file, argv[ 2 ], strlen( argv[ 2 ] ) + 1 );

  if ( argc == 4 )
  {
    if ( strncmp( argv[ 3 ], "over", 4 ) == 0 )
    {
      *overwrite = 1;
    }
  }

  return 0;
}

int connect_server( int32_t socket_file_descriptor,
                    struct sockaddr_in dest_address )
{
  int32_t success = connect( socket_file_descriptor, ( struct sockaddr *) &dest_address, sizeof( struct sockaddr ) );
  if( success == -1 )
  {
    perror("connect");
    close( socket_file_descriptor );
    exit(1);
  }

  return 0;
}


int main( int32_t argc, char **argv )
{
  /*Arguments handle*/
  const int32_t max_uri_size         = 30;
  const int32_t max_output_file_size = 30;
  const int32_t backlog              = 10;

  char    uri[ max_uri_size ];
  char    output_file[ max_output_file_size ];
  int8_t overwrite = 0;

  handle_arguments( argc, argv, uri, output_file, &overwrite );
  /*End of arguments handle*/

  /* Socket Setup */
  struct hostent *dest = gethostbyname( uri );
  if ( dest == NULL )
  {
    herror("Error in get host by name\n");
    exit(1);
  }

  int32_t socket_file_descriptor = socket( PF_INET, SOCK_STREAM, 0 );
  if ( socket_file_descriptor == -1 )
  {
    perror( "socket" );
    exit( 1 );
  }

  struct sockaddr_in host_address;
  host_address.sin_family       = AF_INET;
  host_address.sin_port         = 0;
  host_address .sin_addr.s_addr = INADDR_ANY;
  memset( &( host_address.sin_zero ), '\0',  8 );

  if( bind( socket_file_descriptor, ( struct sockaddr * )&host_address, sizeof( struct sockaddr ) ) != 0 )
  {
     herror( "Error in bind," );
     close( socket_file_descriptor );
     return 1;
  }

  printf( "Ip Address  : %s\n", inet_ntoa( *( ( struct in_addr * ) dest->h_addr_list[ 0 ] ) ) );

  struct sockaddr_in dest_address;
  dest_address.sin_family       = AF_INET;
  dest_address.sin_port         = htons( 80 );
  dest_address .sin_addr        = *( (struct in_addr *)dest->h_addr_list[ 0 ] );
  memset( &( host_address.sin_zero ), '\0',  8 );

  if( connect_server( socket_file_descriptor, dest_address ) != 0 )
  {
    return 1;
  }

  char* final_request[ 100 ];
  char* path_to_file = "index.html";
  sprintf( final_request, "GET %s HTTP/1.0", path_to_file );


  fd_set readfds;
  FD_ZERO( &readfds );



  /* End of socket setup*/

  close( socket_file_descriptor );
  return 0;
}
