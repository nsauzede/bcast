#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

SOCKET create_sock( int port, char *addr)
{
	SOCKET s;

#ifdef _WIN32
	WORD ver;
	WSADATA wsadata;
	ver = MAKEWORD( 2, 2);
	WSAStartup( ver, &wsadata);
#endif
	s = socket( AF_INET, SOCK_DGRAM, 0);
	if (s == -1)
	{
		perror( "socket");
		exit( 1);
	}
	int on = 1;
	if (setsockopt( s, SOL_SOCKET, SO_BROADCAST, (void *)&on, sizeof( on)) == -1)
	{
		perror( "setsockopt");
		exit( 2);
	}
	struct sockaddr_in sa;
	memset( &sa, 0, sizeof( sa));
	sa.sin_family = PF_INET;
	sa.sin_port = htons( port);
	sa.sin_addr.s_addr = inet_addr( addr);
//	sa.sin_addr.s_addr = INADDR_ANY;
#if 1
	if (connect( s, (struct sockaddr *)&sa, sizeof( sa)) == -1)
	{
		perror( "connect");
		exit( 3);
	}
#else
	if (bind( s, (struct sockaddr *)&sa, sizeof( sa)) == -1)
	{
		perror( "bind");
		exit( 3);
	}
#endif

	return s;
}

#define PORT 12345
#define ADDR "127.0.0.1"
int main( int argc, char *argv[])
{
	SOCKET sock;
	int port = PORT;
	char *addr = ADDR;

	int arg = 1;
	if (argc > arg)
	{
		addr = argv[arg++];
	}
	sock = create_sock( port, addr);
	char *str = "hello";
	int size = strlen( str);
	printf( "sending %d bytes [%s] to %s:%d\n", size, str, addr, port);
	send( sock, str, size, 0);

	return 0;
}
